/*****************************************************************************
* \file      dtl_json_reader.c
* \author    Conny Gustafsson
* \date      2019-07-18
* \brief     DTL-powered JSON reader
*
* Copyright (c) 2019-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bstr.h"
#include "dtl_json.h"
#include "adt_bytearray.h"
#include "adt_stack.h"
#include "filestream.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef uint8_t parse_state_t;

#define PARSE_STATE_NONE          ((parse_state_t) 0u)
#define PARSE_STATE_ERROR         ((parse_state_t) 1u)
#define PARSE_STATE_PRE_VALUE     ((parse_state_t) 2u)
#define PARSE_STATE_VALUE         ((parse_state_t) 3u)
#define PARSE_STATE_POST_VALUE    ((parse_state_t) 4u)
#define PARSE_STATE_ARRAY_BEGIN   ((parse_state_t) 5u)
#define PARSE_STATE_ARRAY_NEXT    ((parse_state_t) 6u)
#define PARSE_STATE_OBJECT_BEGIN  ((parse_state_t) 7u)
#define PARSE_STATE_OBJECT_KEY    ((parse_state_t) 8u)
#define PARSE_STATE_OBJECT_SEP    ((parse_state_t) 9u)
#define PARSE_STATE_OBJECT_NEXT   ((parse_state_t) 10u)

typedef struct dtl_json_reader_data_tag
{
   dtl_dv_t *current_elem; // strong reference
   dtl_dv_t *parent_elem;  // weak reference
   bool is_array;
   bool is_object;
   adt_str_t object_key;
} dtl_json_reader_data_t;

typedef struct dtl_json_reader_tag
{
   adt_stack_t stack;
   adt_bytearray_t parse_buf;
   const uint8_t *begin;
   const uint8_t *end;
   bool eof;
   bool parse_complete;
   bstr_context_t ctx;
   parse_state_t parse_state;
   dtl_json_reader_data_t *data;
   dtl_json_error_t last_error;
   uint32_t line_number;
} dtl_json_reader_t;

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_reader_create(dtl_json_reader_t *self);
static void dtl_json_reader_destroy(dtl_json_reader_t *self);
static void dtl_json_reader_data_create(dtl_json_reader_data_t *self);
static void dtl_json_reader_data_destroy(dtl_json_reader_data_t *self);
static dtl_json_reader_data_t* dtl_json_reader_data_new(void);
static void dtl_json_reader_data_delete(dtl_json_reader_data_t *self);
static void dtl_json_reader_data_vdelete(void *arg);

static void dtl_json_reader_read_chunk(void *arg, const uint8_t *chunk, uint32_t chunk_len);
static void dtl_json_reader_close(void *arg);
static const uint8_t *dtl_json_reader_parse_block(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end);
static const uint8_t *dtl_json_reader_parse_value(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end);
static const uint8_t *dtl_json_reader_parse_number(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end);
static const uint8_t *dtl_json_reader_lstrip(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
dtl_dv_t* dtl_json_load(FILE *fh)
{
   dtl_dv_t *retval = NULL;
   if (fh == NULL)
   {
      return NULL;
   }

   cutil_ifstream_handler_t handler;
   cutil_ifstream_t ifstream;
   dtl_json_reader_t reader;
   dtl_json_reader_create(&reader);
   memset(&handler, 0, sizeof(handler));
   handler.arg = (void*) &reader;
   handler.write = dtl_json_reader_read_chunk;
   handler.close = dtl_json_reader_close;
   cutil_ifstream_create(&ifstream, &handler);
   if (cutil_ifstream_read_text_file_from_handle(&ifstream, fh) == 0)
   {
      if (reader.parse_complete && (reader.parse_state == PARSE_STATE_NONE) && (adt_stack_size(&reader.stack) == 0) && (reader.data->current_elem != NULL))
      {
         retval = reader.data->current_elem;
         dtl_dv_inc_ref(reader.data->current_elem);
      }
      dtl_json_reader_destroy(&reader);
   }
   else
   {
      dtl_json_reader_destroy(&reader);
   }
   return retval;
}

dtl_dv_t* dtl_json_loads(const adt_str_t *str)
{
   if (str != NULL)
   {
      const uint8_t *begin = (const uint8_t*) adt_str_data(str);
      const uint8_t *end = begin + adt_str_length(str);
      return dtl_json_load_bstr(begin, end);
   }
   return NULL;
}

dtl_dv_t* dtl_json_load_cstr(const char *str)
{
   if (str != NULL)
   {
      const uint8_t *begin = (const uint8_t*) str;
      const uint8_t *end = begin + strlen(str);
      return dtl_json_load_bstr(begin, end);
   }
   return NULL;
}

dtl_dv_t* dtl_json_load_bstr(const uint8_t *begin, const uint8_t *end)
{
   dtl_dv_t *retval = NULL;
   if ((begin != NULL) && (end != NULL) && (begin <= end))
   {
      dtl_json_reader_t reader;
      dtl_json_reader_create(&reader);
      reader.eof = true;
      const uint8_t *result = dtl_json_reader_parse_block(&reader, begin, end);
      if ((result == end) && (reader.parse_state == PARSE_STATE_NONE) && (adt_stack_size(&reader.stack) == 0))
      {
         if (reader.data->current_elem != NULL)
         {
            retval = reader.data->current_elem;
            dtl_dv_inc_ref(reader.data->current_elem);
         }
      }
      dtl_json_reader_destroy(&reader);
   }
   return retval;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_reader_create(dtl_json_reader_t *self)
{
   if (self != NULL)
   {
      self->eof = false;
      self->parse_complete = false;
      self->begin = NULL;
      self->end = NULL;
      self->last_error = DTL_JSON_NO_ERROR;
      self->line_number = 1u;
      self->parse_state = PARSE_STATE_NONE;
      self->data = dtl_json_reader_data_new();
      adt_bytearray_create(&self->parse_buf);
      bstr_context_create(&self->ctx);
      adt_stack_create(&self->stack, dtl_json_reader_data_vdelete);
   }
}

static void dtl_json_reader_destroy(dtl_json_reader_t *self)
{
   if (self != NULL)
   {
      adt_bytearray_destroy(&self->parse_buf);
      dtl_json_reader_data_delete(self->data);
      adt_stack_destroy(&self->stack);
   }
}

static void dtl_json_reader_data_create(dtl_json_reader_data_t *self)
{
   if (self != NULL)
   {
      self->current_elem = NULL;
      self->parent_elem = NULL;
      self->is_array = false;
      self->is_object = false;
      adt_str_create(&self->object_key);
   }
}

static void dtl_json_reader_data_destroy(dtl_json_reader_data_t *self)
{
   if (self != NULL)
   {
      adt_str_destroy(&self->object_key);
      if (self->current_elem != NULL)
      {
         dtl_dv_dec_ref(self->current_elem);
      }
   }
}

static dtl_json_reader_data_t* dtl_json_reader_data_new(void)
{
   dtl_json_reader_data_t *self = (dtl_json_reader_data_t*) malloc(sizeof(dtl_json_reader_data_t));
   if (self != NULL)
   {
      dtl_json_reader_data_create(self);
   }
   return self;
}

static void dtl_json_reader_data_delete(dtl_json_reader_data_t *self)
{
   if (self != NULL)
   {
      dtl_json_reader_data_destroy(self);
      free(self);
   }
}

static void dtl_json_reader_data_vdelete(void *arg)
{
   dtl_json_reader_data_delete((dtl_json_reader_data_t*) arg);
}

/**
 * For now we wait until entire file has been read into memory.
 * Streaming JSON parsing may be implemented in the future.
 */
static void dtl_json_reader_read_chunk(void *arg, const uint8_t *chunk, uint32_t chunk_len)
{
   dtl_json_reader_t *self = (dtl_json_reader_t*) arg;
   if ((self != NULL) && (chunk != NULL) && (chunk_len > 0) && (chunk_len < INT32_MAX))
   {
      adt_bytearray_append(&self->parse_buf, chunk, chunk_len);
   }
}

static void dtl_json_reader_close(void *arg)
{
   dtl_json_reader_t *self = (dtl_json_reader_t*) arg;
   if (self != NULL)
   {
      const uint8_t *begin = adt_bytearray_data(&self->parse_buf);
      const uint8_t *end = begin + adt_bytearray_length(&self->parse_buf);
      if ((begin != NULL) && (end != NULL))
      {
         const uint8_t *result = dtl_json_reader_parse_block(self, begin, end);
         if ((result == end) && (self->parse_state == PARSE_STATE_NONE) && (adt_stack_size(&self->stack) == 0))
         {
            self->parse_complete = true;
         }
      }
   }
}

static const uint8_t *dtl_json_reader_parse_block(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end)
{
   const uint8_t *next = begin;

   if (self->parse_state == PARSE_STATE_NONE)
   {
      self->parse_state = PARSE_STATE_PRE_VALUE;
   }
   while ((self->parse_state != PARSE_STATE_NONE) && (self->parse_state != PARSE_STATE_ERROR))
   {
      const uint8_t *result;
      uint8_t next_char = 0;

      if (next >= end)
      {
         if (self->parse_state == PARSE_STATE_POST_VALUE)
         {
            // Allow post_value to complete at end of buffer
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
            break;
         }
      }
      else
      {
         next_char = *next;
      }

      switch (self->parse_state)
      {
      case PARSE_STATE_PRE_VALUE:
         next = dtl_json_reader_lstrip(self, next, end);
         if (next < end)
         {
            self->parse_state = PARSE_STATE_VALUE;
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
         }
         break;
      case PARSE_STATE_VALUE:
         result = dtl_json_reader_parse_value(self, next, end);
         if ((result != NULL) && (result > next))
         {
            next = result;
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
         }
         break;
      case PARSE_STATE_POST_VALUE:
         next = dtl_json_reader_lstrip(self, next, end);
         if (self->data->is_array)
         {
            assert(self->data->parent_elem != NULL);
            dtl_av_push((dtl_av_t*) self->data->parent_elem, self->data->current_elem, false);
            self->data->current_elem = NULL;
            self->parse_state = PARSE_STATE_ARRAY_NEXT;
         }
         else if (self->data->is_object)
         {
            assert(self->data->parent_elem != NULL);
            dtl_hv_set_cstr((dtl_hv_t*) self->data->parent_elem, adt_str_cstr(&self->data->object_key), self->data->current_elem, false);
            self->data->current_elem = NULL;
            adt_str_clear(&self->data->object_key);
            self->parse_state = PARSE_STATE_OBJECT_NEXT;
         }
         else
         {
            self->parse_state = PARSE_STATE_NONE;
         }
         break;
      case PARSE_STATE_ARRAY_BEGIN:
         next = dtl_json_reader_lstrip(self, next, end);
         if (next < end)
         {
            next_char = *next;
            if (next_char == ']')
            {
               // empty array, no need to create child state
               self->parse_state = PARSE_STATE_POST_VALUE;
               next++;
            }
            else
            {
               // non-empty array, push current data and initiate child state
               dtl_json_reader_data_t *child_data = dtl_json_reader_data_new();
               if (child_data != NULL)
               {
                  child_data->is_array = true;
                  child_data->parent_elem = self->data->current_elem;
                  adt_stack_push(&self->stack, self->data);
                  self->data = child_data;
                  self->parse_state = PARSE_STATE_PRE_VALUE;
               }
               else
               {
                  self->parse_state = PARSE_STATE_ERROR;
                  self->last_error = DTL_JSON_MEM_ERROR;
               }
            }
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
         }
         break;
      case PARSE_STATE_ARRAY_NEXT:
         if (next_char == ',')
         {
            next++;
            self->parse_state = PARSE_STATE_PRE_VALUE;
         }
         else if (next_char == ']')
         {
            next++;
            dtl_dv_inc_ref(self->data->current_elem);
            dtl_json_reader_data_delete(self->data);
            assert(adt_stack_size(&self->stack) > 0);
            self->data = (dtl_json_reader_data_t*) adt_stack_top(&self->stack);
            adt_stack_pop(&self->stack);
            self->parse_state = PARSE_STATE_POST_VALUE;
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
         }
         break;
      case PARSE_STATE_OBJECT_BEGIN:
         next = dtl_json_reader_lstrip(self, next, end);
         if (next < end)
         {
            next_char = *next;
            if (next_char == '}')
            {
               // empty object, no need to create child state
               self->parse_state = PARSE_STATE_POST_VALUE;
               next++;
            }
            else
            {
               // non-empty object, push current data and initiate child state
               dtl_json_reader_data_t *child_data = dtl_json_reader_data_new();
               if (child_data != NULL)
               {
                  child_data->is_object = true;
                  child_data->parent_elem = self->data->current_elem;
                  adt_stack_push(&self->stack, self->data);
                  self->data = child_data;
                  self->parse_state = PARSE_STATE_OBJECT_KEY;
               }
               else
               {
                  self->parse_state = PARSE_STATE_ERROR;
                  self->last_error = DTL_JSON_MEM_ERROR;
               }
            }
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
         }
         break;
      case PARSE_STATE_OBJECT_KEY:
         next = dtl_json_reader_lstrip(self, next, end);
         if (next < end)
         {
            next_char = *next;
            if (next_char == '"')
            {
               const uint8_t *inner_result;
               inner_result = bstr_parse_json_string_literal(&self->ctx, next, end, &self->data->object_key);
               if ((inner_result != NULL) && (inner_result > next))
               {
                  next = inner_result;
                  if (adt_str_length(&self->data->object_key) == 0)
                  {
                     self->parse_state = PARSE_STATE_ERROR;
                     self->last_error = DTL_JSON_EMPTY_KEY_ERROR;
                  }
                  else
                  {
                     self->parse_state = PARSE_STATE_OBJECT_SEP;
                  }
               }
               else
               {
                  self->parse_state = PARSE_STATE_ERROR;
                  self->last_error = DTL_JSON_UNMATCHED_STRING_LITERAL;
               }
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
         }
         break;
      case PARSE_STATE_OBJECT_SEP:
         next = dtl_json_reader_lstrip(self, next, end);
         if (next < end)
         {
            next_char = *next;
            if (next_char == ':')
            {
               next++;
               self->parse_state = PARSE_STATE_PRE_VALUE;
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
         }
         break;
      case PARSE_STATE_OBJECT_NEXT:
         if (next_char == ',')
         {
            next++;
            self->parse_state = PARSE_STATE_OBJECT_KEY;
         }
         else if (next_char == '}')
         {
            next++;
            dtl_dv_inc_ref(self->data->current_elem);
            dtl_json_reader_data_delete(self->data);
            assert(adt_stack_size(&self->stack) > 0);
            self->data = (dtl_json_reader_data_t*) adt_stack_top(&self->stack);
            adt_stack_pop(&self->stack);
            self->parse_state = PARSE_STATE_POST_VALUE;
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
         }
         break;
      default:
         assert(false);
      }
   }
   if (self->parse_state == PARSE_STATE_ERROR)
   {
      if (self->data->current_elem != NULL)
      {
         dtl_dv_dec_ref(self->data->current_elem);
         self->data->current_elem = NULL;
      }
   }
   return next;
}

static const uint8_t *dtl_json_reader_parse_value(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end)
{
   const uint8_t *next = begin;
   if (next < end)
   {
      const uint8_t *result = NULL;
      int first_char = (int) *begin;
      if (bstr_pred_is_digit(first_char) || (first_char == '-'))
      {
         result = dtl_json_reader_parse_number(self, next, end);
         if ((result != NULL) && (result > begin))
         {
            next = result;
            self->parse_state = PARSE_STATE_POST_VALUE;
         }
         else
         {
            self->parse_state = PARSE_STATE_ERROR;
            if (self->last_error == DTL_JSON_NO_ERROR)
            {
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
         }
      }
      else
      {
         adt_str_t *str;

         switch (first_char)
         {
         case '"':
            str = adt_str_new();
            if (str != NULL)
            {
               result = bstr_parse_json_string_literal(&self->ctx, next, end, str);
               if ((result != NULL) && (result > begin))
               {
                  self->data->current_elem = (dtl_dv_t*) dtl_sv_make_str(str);
                  next = result;
                  self->parse_state = PARSE_STATE_POST_VALUE;
               }
               else
               {
                  self->parse_state = PARSE_STATE_ERROR;
                  self->last_error = DTL_JSON_UNMATCHED_STRING_LITERAL;
               }
               adt_str_delete(str);
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_MEM_ERROR;
            }
            break;
         case '[':
            self->data->current_elem = (dtl_dv_t*) dtl_av_new();
            if (self->data->current_elem == NULL)
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_MEM_ERROR;
            }
            else
            {
               self->parse_state = PARSE_STATE_ARRAY_BEGIN;
               next++;
            }
            break;
         case '{':
            self->data->current_elem = (dtl_dv_t*) dtl_hv_new();
            if (self->data->current_elem == NULL)
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_MEM_ERROR;
            }
            else
            {
               self->parse_state = PARSE_STATE_OBJECT_BEGIN;
               next++;
            }
            break;
         case 'f':
            result = bstr_match_cstr(next, end, "false");
            if ((result != NULL) && (result > begin))
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_bool(false);
               next = result;
               self->parse_state = PARSE_STATE_POST_VALUE;
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
            break;
         case 't':
            result = bstr_match_cstr(next, end, "true");
            if ((result != NULL) && (result > begin))
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_bool(true);
               next = result;
               self->parse_state = PARSE_STATE_POST_VALUE;
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
            break;
         case 'n':
            result = bstr_match_cstr(next, end, "null");
            if ((result != NULL) && (result > begin))
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_none();
               next = result;
               self->parse_state = PARSE_STATE_POST_VALUE;
            }
            else
            {
               self->parse_state = PARSE_STATE_ERROR;
               self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
            break;
         default:
            self->parse_state = PARSE_STATE_ERROR;
            self->last_error = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            break;
         }
      }
   }
   else
   {
      self->parse_state = PARSE_STATE_ERROR;
      self->last_error = DTL_JSON_UNEXPECTED_EOB_ERROR;
   }
   return next;
}

static const uint8_t *dtl_json_reader_parse_number(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end)
{
   bstr_number_t number;
   const uint8_t *next = begin;
   const uint8_t *result = bstr_parse_json_number(&self->ctx, begin, end, &number);
   if ((result != NULL) && (result > begin))
   {
      if (number.has_integer && (!number.has_fraction) && (!number.has_exponent))
      {
         if (number.is_negative)
         {
            if (number.integer > (uint32_t) INT32_MAX)
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_i64(-((int64_t) number.integer));
            }
            else
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_i32(-((int32_t) number.integer));
            }
         }
         else
         {
            if (number.integer > (uint32_t) INT32_MAX)
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_u32(number.integer);
            }
            else
            {
               self->data->current_elem = (dtl_dv_t*) dtl_sv_make_i32((int32_t) number.integer);
            }
         }
         next = result;
      }
      else
      {
         next = NULL;
         self->parse_state = PARSE_STATE_ERROR;
      }
   }
   return next;
}

static const uint8_t *dtl_json_reader_lstrip(dtl_json_reader_t *self, const uint8_t *begin, const uint8_t *end)
{
   const uint8_t *next = begin;
   while (next < end)
   {
      int c = (int) *next;
      if (!bstr_pred_is_whitespace(c))
      {
         break;
      }
      if (c == '\n')
      {
         self->line_number++;
      }
      next++;
   }
   return next;
}
