/*****************************************************************************
* \file      dtl_json_reader.c
* \author    Conny Gustafsson
* \date      2019-07-18
* \brief     DTL-powered JSON reader
*
* Copyright (c) 2019 Conny Gustafsson
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
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
typedef uint8_t parseState_t;

#define PARSE_STATE_NONE          ((parseState_t) 0u)
#define PARSE_STATE_ERROR         ((parseState_t) 1u)
#define PARSE_STATE_PRE_VALUE     ((parseState_t) 2u)
#define PARSE_STATE_VALUE         ((parseState_t) 3u)
#define PARSE_STATE_POST_VALUE    ((parseState_t) 4u)
#define PARSE_STATE_ARRAY_BEGIN   ((parseState_t) 5u)
#define PARSE_STATE_ARRAY_NEXT    ((parseState_t) 6u)
#define PARSE_STATE_OBJECT_BEGIN  ((parseState_t) 7u)
#define PARSE_STATE_OBJECT_KEY    ((parseState_t) 8u)
#define PARSE_STATE_OBJECT_SEP    ((parseState_t) 9u)
#define PARSE_STATE_OBJECT_NEXT   ((parseState_t) 10u)


typedef struct dtl_json_readerData_tag
{
   dtl_dv_t *currentElem; //strong reference
   dtl_dv_t *parentElem; //weak reference
   bool isArray;
   bool isObject;
   adt_str_t objectKey;
} dtl_json_readerData_t;

typedef struct dtl_json_reader_tag
{
   adt_stack_t stack;
   adt_bytearray_t parseBuf;
   const uint8_t *pBegin;
   const uint8_t *pEnd;
   bool eof;
   bool parseComplete;
   bstr_context_t ctx;
   parseState_t parseState;
   dtl_json_readerData_t *data;
   dtl_json_error_t lastError;
   uint32_t lineNumber;
} dtl_json_reader_t;



//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_reader_create(dtl_json_reader_t *self);
static void dtl_json_reader_destroy(dtl_json_reader_t *self);
static void dtl_json_readerData_create(dtl_json_readerData_t *self);
static void dtl_json_readerData_destroy(dtl_json_readerData_t *self);
static dtl_json_readerData_t* dtl_json_readerData_new(void);
static void dtl_json_readerData_delete(dtl_json_readerData_t *self);
static void dtl_json_readerData_vdelete(void *arg);

static void dtl_json_reader_readChunk(void *arg,const uint8_t *pChunk, uint32_t chunkLen);
static void dtl_json_reader_close(void *arg);
static const uint8_t *dtl_json_reader_parse_block(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd);
static const uint8_t *dtl_json_reader_parse_value(dtl_json_reader_t *self, const uint8_t *pLineBegin, const uint8_t *pLineEnd);
static const uint8_t *dtl_json_reader_parse_number(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd);
static const uint8_t *dtl_json_reader_lstrip(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
dtl_dv_t* dtl_json_load(FILE *fh)
{
   dtl_dv_t *retval = (dtl_dv_t*) 0;
   ifstream_handler_t handler;
   ifstream_t ifstream;
   dtl_json_reader_t reader;
   dtl_json_reader_create(&reader);
   memset(&handler, 0, sizeof(handler));
   handler.arg = (void*) &reader;
   handler.write = dtl_json_reader_readChunk;
   handler.close = dtl_json_reader_close;
   ifstream_create(&ifstream, &handler);
   if (ifstream_readTextFileFromHandle(&ifstream, fh) == 0)
   {
      if ( (reader.parseComplete) && (reader.data->currentElem != 0) )
      {
         retval = reader.data->currentElem;
         dtl_dv_inc_ref(reader.data->currentElem);
      }
      dtl_json_reader_destroy(&reader);
   }
   else
   {
      dtl_json_reader_destroy(&reader);
   }
   return retval;
}

dtl_dv_t* dtl_json_loads(adt_str_t *str);

dtl_dv_t* dtl_json_load_cstr(const char *str)
{
   const char *pBegin = str;
   const char *pEnd;
   size_t len = strlen(str);
   pEnd = str + len;
   return dtl_json_load_bstr( (const uint8_t*) pBegin, (const uint8_t*) pEnd);
}

dtl_dv_t* dtl_json_load_bstr(const uint8_t *pBegin, const uint8_t *pEnd)
{
   dtl_dv_t *retval = (dtl_dv_t*) 0;
   const uint8_t *pResult;
   dtl_json_reader_t reader;
   dtl_json_reader_create(&reader);
   reader.eof = true;
   pResult = dtl_json_reader_parse_block(&reader, pBegin, pEnd);
   if (pResult == (const uint8_t*) pEnd)
   {
      if (reader.data->currentElem != 0)
      {
         retval = reader.data->currentElem;
         dtl_dv_inc_ref(reader.data->currentElem);
      }
      dtl_json_reader_destroy(&reader);
   }
   else
   {
      dtl_json_reader_destroy(&reader);
   }
   return retval;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_reader_create(dtl_json_reader_t *self)
{
   if (self != 0)
   {
      self->eof = false;
      self->parseComplete = false;
      self->pBegin = 0;
      self->pEnd = 0;
      self->lastError = DTL_JSON_NO_ERROR;
      self->lineNumber = 1u;
      self->parseState = PARSE_STATE_NONE;
      self->data = dtl_json_readerData_new();
      adt_bytearray_create(&self->parseBuf, ADT_BYTE_ARRAY_DEFAULT_GROW_SIZE);
      bstr_context_create(&self->ctx);
      adt_stack_create(&self->stack, dtl_json_readerData_vdelete);
   }
}
static void dtl_json_reader_destroy(dtl_json_reader_t *self)
{
   if (self != 0)
   {
      adt_bytearray_destroy(&self->parseBuf);
      dtl_json_readerData_delete(self->data);
      adt_stack_destroy(&self->stack);
   }
}

static void dtl_json_readerData_create(dtl_json_readerData_t *self)
{
   if (self != 0)
   {
      self->currentElem = (dtl_dv_t*) 0;
      self->parentElem = (dtl_dv_t*) 0;
      self->isArray = false;
      self->isObject = false;
      adt_str_create(&self->objectKey);
   }
}

static void dtl_json_readerData_destroy(dtl_json_readerData_t *self)
{
   if (self != 0)
   {
      adt_str_destroy(&self->objectKey);
      if ( self->currentElem != 0)
      {
         dtl_dv_dec_ref(self->currentElem);
      }
   }
}

static dtl_json_readerData_t* dtl_json_readerData_new(void)
{
   dtl_json_readerData_t *self = (dtl_json_readerData_t*) malloc(sizeof(dtl_json_readerData_t));
   if (self != 0)
   {
      dtl_json_readerData_create(self);
   }
   return self;
}

static void dtl_json_readerData_delete(dtl_json_readerData_t *self)
{
   if (self != 0)
   {
      dtl_json_readerData_destroy(self);
      free(self);
   }
}

static void dtl_json_readerData_vdelete(void *arg)
{
   dtl_json_readerData_delete((dtl_json_readerData_t*) arg);
}

/**
 * For now we wait until entire file has been read into memory.
 * Sometime in the future I will implement support for streamed parsing of JSON
 */
static void dtl_json_reader_readChunk(void *arg,const uint8_t *pChunk, uint32_t chunkLen)
{
   dtl_json_reader_t *self = (dtl_json_reader_t*) arg;
   if ( (self != 0) && (pChunk != 0) && (chunkLen > 0) && (chunkLen < INT32_MAX) )
   {
      adt_bytearray_append(&self->parseBuf, pChunk, chunkLen);
   }
}

static void dtl_json_reader_close(void *arg)
{
   dtl_json_reader_t *self = (dtl_json_reader_t*) arg;
   if (self != 0)
   {
      const uint8_t *pBegin;
      const uint8_t *pEnd;
      pBegin = adt_bytearray_data(&self->parseBuf);
      pEnd = pBegin + adt_bytearray_length(&self->parseBuf);
      if ( (pBegin != 0) && (pEnd != 0) )
      {
         const uint8_t *pResult = dtl_json_reader_parse_block(self, pBegin, pEnd);
         if (pResult == pEnd)
         {
            self->parseComplete = true;
         }
      }
   }
}

static const uint8_t *dtl_json_reader_parse_block(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd)
{
   const uint8_t *pNext = pBegin;

   if (self->parseState == PARSE_STATE_NONE)
   {
      self->parseState = PARSE_STATE_PRE_VALUE;
   }
   while( (self->parseState != PARSE_STATE_NONE) && (self->parseState != PARSE_STATE_ERROR) && (pNext < pEnd) )
   {
      const uint8_t *pResult;
      uint8_t nextChar = *pNext;

      switch(self->parseState)
      {
      case PARSE_STATE_PRE_VALUE:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         self->parseState = PARSE_STATE_VALUE;
         break;
      case PARSE_STATE_VALUE:
         pResult = dtl_json_reader_parse_value(self, pNext, pEnd);
         if (pResult > pNext)
         {
            pNext = pResult;
         }
         break;
      case PARSE_STATE_POST_VALUE:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         if (self->data->isArray)
         {
            assert(self->data->parentElem != 0);
            dtl_av_push((dtl_av_t*) self->data->parentElem, self->data->currentElem, false);
            self->data->currentElem = (dtl_dv_t*) 0;
            self->parseState = PARSE_STATE_ARRAY_NEXT;
         }
         else if (self->data->isObject)
         {
            assert(self->data->parentElem != 0);
            dtl_hv_set_cstr((dtl_hv_t*) self->data->parentElem, adt_str_cstr(&self->data->objectKey), self->data->currentElem, false);
            self->data->currentElem = (dtl_dv_t*) 0;
            adt_str_clear(&self->data->objectKey);
            self->parseState = PARSE_STATE_OBJECT_NEXT;
         }
         else
         {
            self->parseState = PARSE_STATE_NONE;
         }
         break;
      case PARSE_STATE_ARRAY_BEGIN:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         if ( pNext < pEnd)
         {
            nextChar = *pNext;
            if (nextChar==']')
            {
               //empty array, no need to create child state
               self->parseState = PARSE_STATE_POST_VALUE;
               pNext++;
            }
            else
            {
               //non-empty array, push current data and initiate child state
               dtl_json_readerData_t *childData = dtl_json_readerData_new();
               if (childData != 0)
               {
                  childData->isArray = true;
                  childData->parentElem = self->data->currentElem;
                  adt_stack_push(&self->stack, self->data);
                  self->data = childData;
                  self->parseState = PARSE_STATE_PRE_VALUE;
               }
               else
               {
                  self->parseState = PARSE_STATE_ERROR;
                  self->lastError = DTL_JSON_MEM_ERROR;
               }
            }
         }
         break;
      case PARSE_STATE_ARRAY_NEXT:
         if (nextChar == ',')
         {
            pNext++;
            self->parseState = PARSE_STATE_PRE_VALUE;
         }
         else if(nextChar == ']')
         {
            pNext++;
            dtl_dv_inc_ref(self->data->currentElem);
            dtl_json_readerData_delete(self->data);
            assert(adt_stack_size(&self->stack) > 0);
            self->data = adt_stack_top(&self->stack);
            adt_stack_pop(&self->stack);
            self->parseState = PARSE_STATE_POST_VALUE;
         }
         else
         {
            self->parseState = PARSE_STATE_ERROR;
            self->lastError = DTL_JSON_UNEXPECTED_CHAR_ERROR;
         }
         break;
      case PARSE_STATE_OBJECT_BEGIN:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         if ( pNext < pEnd)
         {
            nextChar = *pNext;
            if (nextChar=='}')
            {
               //empty object, no need to create child state
               self->parseState = PARSE_STATE_POST_VALUE;
               pNext++;
            }
            else
            {
               //non-empty array, push current data and initiate child state
               dtl_json_readerData_t *childData = dtl_json_readerData_new();
               if (childData != 0)
               {
                  childData->isObject = true;
                  childData->parentElem = self->data->currentElem;
                  adt_stack_push(&self->stack, self->data);
                  self->data = childData;
                  self->parseState = PARSE_STATE_OBJECT_KEY;
               }
               else
               {
                  self->parseState = PARSE_STATE_ERROR;
                  self->lastError = DTL_JSON_MEM_ERROR;
               }
            }
         }
         break;
      case PARSE_STATE_OBJECT_KEY:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         if ( pNext < pEnd)
         {
            nextChar = *pNext;
            if (nextChar=='"')
            {
               const uint8_t *pResult;
               pResult = bstr_parse_json_string_literal(&self->ctx, pNext, pEnd, &self->data->objectKey);
               if (pResult > pNext)
               {
                  pNext = pResult;
                  if (adt_str_length(&self->data->objectKey) == 0)
                  {
                     self->parseState = PARSE_STATE_ERROR;
                     self->lastError = DTL_JSON_EMPTY_KEY_ERROR;
                  }
                  else
                  {
                     self->parseState = PARSE_STATE_OBJECT_SEP;
                  }
               }
               else
               {
                  self->parseState = PARSE_STATE_ERROR;
                  self->lastError = DTL_JSON_UNMATCHED_STRING_LITERAL;
               }
            }
            //TODO: We should probably allow stray comma here to make it easier for the user
            else
            {
               self->parseState = PARSE_STATE_ERROR;
               self->lastError = DTL_JSON_UNEXPECTED_CHAR_ERROR;
            }
         }
         break;
      case PARSE_STATE_OBJECT_SEP:
         pNext = dtl_json_reader_lstrip(self, pNext, pEnd);
         if ( pNext < pEnd)
         {
            nextChar = *pNext;
            if (nextChar==':')
            {
               pNext++;
               self->parseState = PARSE_STATE_PRE_VALUE;
            }
         }
         break;
      case PARSE_STATE_OBJECT_NEXT:
         if (nextChar == ',')
         {
            pNext++;
            self->parseState = PARSE_STATE_OBJECT_KEY;
         }
         else if(nextChar == '}')
         {
            pNext++;
            dtl_dv_inc_ref(self->data->currentElem);
            dtl_json_readerData_delete(self->data);
            assert(adt_stack_size(&self->stack) > 0);
            self->data = adt_stack_top(&self->stack);
            adt_stack_pop(&self->stack);
            self->parseState = PARSE_STATE_POST_VALUE;
         }
         else
         {
            self->parseState = PARSE_STATE_ERROR;
            self->lastError = DTL_JSON_UNEXPECTED_CHAR_ERROR;
         }
         break;
      default:
         assert(0);
      }
   }
   if (self->parseState == PARSE_STATE_ERROR)
   {
      dtl_av_delete((dtl_av_t*) self->data->currentElem);
   }
   return pNext;
}

static const uint8_t *dtl_json_reader_parse_value(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd)
{
   const uint8_t *pNext = pBegin;
   if (pNext < pEnd)
   {
      const uint8_t *pResult = (const uint8_t*) 0;
      int firstChar = (int) *pBegin;
      if (bstr_pred_is_digit(firstChar) || firstChar == '-')
      {
         pResult = dtl_json_reader_parse_number(self, pNext, pEnd);
         if (pResult > pBegin)
         {
            pNext = pResult;
            self->parseState = PARSE_STATE_POST_VALUE;
         }
      }
      else
      {
         adt_str_t *str;

         switch(firstChar)
         {
         case '"':
            str = adt_str_new();
            if (str != 0)
            {
               pResult = bstr_parse_json_string_literal(&self->ctx, pNext, pEnd, str);
               if (pResult > pBegin)
               {
                  self->data->currentElem = (dtl_dv_t*) dtl_sv_make_str(str);
                  pNext = pResult;
                  self->parseState = PARSE_STATE_POST_VALUE;
               }
               adt_str_delete(str);
            }
            break;
         case '[':
            self->data->currentElem = (dtl_dv_t*) dtl_av_new();
            if (self->data->currentElem == 0)
            {
               self->parseState = PARSE_STATE_ERROR;
               self->lastError = DTL_JSON_MEM_ERROR;
            }
            else
            {
               self->parseState = PARSE_STATE_ARRAY_BEGIN;
               pNext++;
            }
            break;
         case '{':
            self->data->currentElem = (dtl_dv_t*) dtl_hv_new();
            if (self->data->currentElem == 0)
            {
               self->parseState = PARSE_STATE_ERROR;
               self->lastError = DTL_JSON_MEM_ERROR;
            }
            else
            {
               self->parseState = PARSE_STATE_OBJECT_BEGIN;
               pNext++;
            }
            break;
         case 'f':
            pResult = bstr_match_cstr(pNext, pEnd, "false");
            if (pResult > pBegin)
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_bool(false);
               pNext = pResult;
               self->parseState = PARSE_STATE_POST_VALUE;
            }
            break;
         case 't':
            pResult = bstr_match_cstr(pNext, pEnd, "true");
            if (pResult > pBegin)
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_bool(true);
               pNext = pResult;
               self->parseState = PARSE_STATE_POST_VALUE;
            }
            break;
         case 'n':
            pResult = bstr_match_cstr(pNext, pEnd, "null");
            if (pResult > pBegin)
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_none();
               pNext = pResult;
               self->parseState = PARSE_STATE_POST_VALUE;
            }
            break;
         default:
            self->parseState = PARSE_STATE_ERROR;
            self->lastError = DTL_JSON_UNEXPECTED_CHAR_ERROR;
         }
      }
   }
   return pNext;
}

static const uint8_t *dtl_json_reader_parse_number(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd)
{
   bstr_number_t number;
   const uint8_t *pNext = pBegin;
   const uint8_t *pResult = bstr_parse_json_number(&self->ctx, pBegin, pEnd, &number);
   if (pResult > pBegin)
   {
      if ( (number.hasInteger) && (!number.hasFraction) && (!number.hasExponent) )
      {
         if (number.isNegative)
         {
            if (number.integer > INT32_MAX)
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_i64( -((int64_t) number.integer) );
            }
            else
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_i32( -((int32_t) number.integer) );
            }
         }
         else
         {
            if ( number.integer > INT32_MAX)
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_u32(number.integer);
            }
            else
            {
               self->data->currentElem = (dtl_dv_t*) dtl_sv_make_i32( ((int32_t) number.integer) );
            }
         }
         pNext = pResult;
      }
      else
      {
         pNext = (const uint8_t*) 0;
         self->parseState = PARSE_STATE_ERROR;
      }
   }
   return pNext;
}


static const uint8_t *dtl_json_reader_lstrip(dtl_json_reader_t *self, const uint8_t *pBegin, const uint8_t *pEnd)
{
   const uint8_t *pNext = pBegin;
   while (pNext < pEnd)
   {
      int c = (int) *pNext;
      if (!bstr_pred_is_whitespace(c)){
         break;
      }
      if (c == '\n')
      {
         self->lineNumber++;
      }
      pNext++;
   }
   return pNext;
}
