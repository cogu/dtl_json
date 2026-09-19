/*****************************************************************************
* \file      dtl_json_writer.c
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     DTL-powered JSON writer
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
#include <stdint.h>
#include <inttypes.h>
#include "dtl_json.h"
#include "adt_bytearray.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define OUTPUT_TYPE_STR  0u
#define OUTPUT_TYPE_FILE 1u
typedef uint8_t output_type_t;
static const char m_indent_char = ' ';

typedef struct dtl_json_writer_tag
{
   output_type_t output_type;
   int32_t indent_width;
   int32_t current_indent;
   adt_bytearray_t *indent_array;
   FILE *dest_file;
   adt_str_t *dest_str;
   const char *newline_str;
   bool sort_keys;
} dtl_json_writer_t;

#define TMP_BUF_SIZE 64

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_writer_create_with_file(dtl_json_writer_t *self, FILE *fh);
static adt_error_t dtl_json_writer_create_with_string(dtl_json_writer_t *self);
static void dtl_json_writer_destroy(dtl_json_writer_t *self, bool keep_str);
static void dtl_json_writer_set_indent_width(dtl_json_writer_t *self, int32_t indent);
static void dtl_json_writer_set_sort_keys(dtl_json_writer_t *self, bool sort_keys);
static void dtl_json_writer_increase_indent(dtl_json_writer_t *self);
static void dtl_json_writer_decrease_indent(dtl_json_writer_t *self);
static void dtl_json_writer_grow_indent_array(dtl_json_writer_t *self);
static dtl_error_t dtl_json_writer_write_dv(dtl_json_writer_t *self, const dtl_dv_t *dv, bool indent_enable);
static dtl_error_t dtl_json_writer_write_sv(dtl_json_writer_t *self, const dtl_sv_t *sv, bool indent_enable);
static dtl_error_t dtl_json_writer_write_av(dtl_json_writer_t *self, const dtl_av_t *av, bool indent_enable);
static dtl_error_t dtl_json_writer_write_hv(dtl_json_writer_t *self, const dtl_hv_t *hv, bool indent_enable);
static void dtl_json_writer_print(dtl_json_writer_t *self, const char *str);
static void dtl_json_writer_putc(dtl_json_writer_t *self, char c);
static void dtl_json_writer_indented_cstr(dtl_json_writer_t *self, const char *str);
static void dtl_json_writer_write_indent_str(dtl_json_writer_t *self);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
dtl_error_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sort_keys)
{
   if ((dv == NULL) || (fh == NULL))
   {
      return DTL_INVALID_ARGUMENT_ERROR;
   }
   dtl_json_writer_t writer;
   dtl_json_writer_create_with_file(&writer, fh);
   if (indent > 0)
   {
      dtl_json_writer_set_indent_width(&writer, indent);
   }
   if (sort_keys)
   {
      dtl_json_writer_set_sort_keys(&writer, true);
   }
   dtl_error_t result = dtl_json_writer_write_dv(&writer, dv, true);
   dtl_json_writer_destroy(&writer, false);
   return result;
}

adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sort_keys)
{
   if (dv == NULL)
   {
      return NULL;
   }
   dtl_json_writer_t writer;
   adt_error_t err = dtl_json_writer_create_with_string(&writer);
   if (err != ADT_NO_ERROR)
   {
      return NULL;
   }
   if (indent > 0)
   {
      dtl_json_writer_set_indent_width(&writer, indent);
   }
   if (sort_keys)
   {
      dtl_json_writer_set_sort_keys(&writer, true);
   }
   dtl_json_writer_write_dv(&writer, dv, true);
   adt_str_t *retval = writer.dest_str;
   dtl_json_writer_destroy(&writer, true);
   return retval;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_writer_create_with_file(dtl_json_writer_t *self, FILE *fh)
{
   if (self != NULL)
   {
      self->output_type = OUTPUT_TYPE_FILE;
      self->indent_width = 0;
      self->current_indent = 0;
      self->indent_array = NULL;
      self->dest_str = NULL;
      self->dest_file = fh;
      self->newline_str = "\n";
      self->sort_keys = false;
   }
}

static adt_error_t dtl_json_writer_create_with_string(dtl_json_writer_t *self)
{
   adt_error_t retval = ADT_NO_ERROR;
   if (self != NULL)
   {
      self->output_type = OUTPUT_TYPE_STR;
      self->indent_width = 0;
      self->current_indent = 0;
      self->indent_array = NULL;
      self->dest_str = adt_str_new();
      self->dest_file = NULL;
      self->sort_keys = false;
      if (self->dest_str == NULL)
      {
         retval = ADT_MEM_ERROR;
      }
      self->newline_str = "\n";
   }
   else
   {
      retval = ADT_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static void dtl_json_writer_destroy(dtl_json_writer_t *self, bool keep_str)
{
   if (self != NULL)
   {
      if (!keep_str && (self->dest_str != NULL))
      {
         adt_str_delete(self->dest_str);
         self->dest_str = NULL;
      }
      if (self->indent_array != NULL)
      {
         adt_bytearray_delete(self->indent_array);
         self->indent_array = NULL;
      }
   }
}

static void dtl_json_writer_set_indent_width(dtl_json_writer_t *self, int32_t indent)
{
   self->indent_width = indent;
   if ((self->indent_width > 0) && (self->indent_array == NULL))
   {
      self->indent_array = adt_bytearray_new();
   }
}

static void dtl_json_writer_set_sort_keys(dtl_json_writer_t *self, bool sort_keys)
{
   self->sort_keys = sort_keys;
}

static void dtl_json_writer_increase_indent(dtl_json_writer_t *self)
{
   if (self->indent_width > 0)
   {
      int32_t old_indent = self->current_indent;
      int32_t current_len = (int32_t) adt_bytearray_length(self->indent_array);
      self->current_indent += self->indent_width;
      if (current_len < self->current_indent)
      {
         dtl_json_writer_grow_indent_array(self);
      }
      else
      {
         uint8_t *data = adt_bytearray_data(self->indent_array);
         memset(&data[old_indent], m_indent_char, (size_t) self->indent_width);
         data[self->current_indent] = 0u;
      }
   }
}

static void dtl_json_writer_decrease_indent(dtl_json_writer_t *self)
{
   if (self->indent_width > 0)
   {
      if (self->current_indent > self->indent_width)
      {
         self->current_indent -= self->indent_width;
      }
      else
      {
         self->current_indent = 0;
      }
      assert(self->indent_array != NULL);
      assert((int32_t) adt_bytearray_length(self->indent_array) > self->current_indent);
      adt_bytearray_data(self->indent_array)[self->current_indent] = 0u;
   }
}

static void dtl_json_writer_grow_indent_array(dtl_json_writer_t *self)
{
   adt_error_t result = adt_bytearray_resize(self->indent_array, (uint32_t) (self->current_indent + 1));
   if (result == ADT_NO_ERROR)
   {
      uint8_t *data = adt_bytearray_data(self->indent_array);
      assert(data != NULL);
      memset(data, m_indent_char, (size_t) self->current_indent);
      data[self->current_indent] = 0u;
   }
}

static dtl_error_t dtl_json_writer_write_dv(dtl_json_writer_t *self, const dtl_dv_t *dv, bool indent_enable)
{
   if ((self != NULL) && (dv != NULL))
   {
      switch (dtl_dv_type(dv))
      {
      case DTL_DV_NULL:
         if (indent_enable)
         {
            dtl_json_writer_indented_cstr(self, "null");
         }
         else
         {
            dtl_json_writer_print(self, "null");
         }
         break;
      case DTL_DV_SCALAR:
         return dtl_json_writer_write_sv(self, (const dtl_sv_t*) dv, indent_enable);
      case DTL_DV_ARRAY:
         return dtl_json_writer_write_av(self, (const dtl_av_t*) dv, indent_enable);
      case DTL_DV_HASH:
         return dtl_json_writer_write_hv(self, (const dtl_hv_t*) dv, indent_enable);
      default:
         break;
      }
      return DTL_NO_ERROR;
   }
   return DTL_INVALID_ARGUMENT_ERROR;
}

static dtl_error_t dtl_json_writer_write_sv(dtl_json_writer_t *self, const dtl_sv_t *sv, bool indent_enable)
{
   char buf[TMP_BUF_SIZE];
   bool ok = false;
   if (indent_enable)
   {
      dtl_json_writer_write_indent_str(self);
   }
   switch (dtl_sv_type(sv))
   {
   case DTL_SV_NONE:
      dtl_json_writer_print(self, "null");
      break;
   case DTL_SV_I32:
      {
         int32_t val = dtl_sv_to_i32(sv, NULL);
         if (self->dest_file != NULL)
         {
            fprintf(self->dest_file, "%d", (int) val);
         }
         else
         {
            snprintf(buf, sizeof(buf), "%d", (int) val);
            adt_str_append_cstr(self->dest_str, buf);
         }
      }
      break;
   case DTL_SV_U32:
      {
         uint32_t val = dtl_sv_to_u32(sv, NULL);
         if (self->dest_file != NULL)
         {
            fprintf(self->dest_file, "%u", (unsigned int) val);
         }
         else
         {
            snprintf(buf, sizeof(buf), "%u", (unsigned int) val);
            adt_str_append_cstr(self->dest_str, buf);
         }
      }
      break;
   case DTL_SV_I64:
      {
         int64_t val = dtl_sv_to_i64(sv, NULL);
         if (self->dest_file != NULL)
         {
            fprintf(self->dest_file, "%" PRId64, val);
         }
         else
         {
            snprintf(buf, sizeof(buf), "%" PRId64, val);
            adt_str_append_cstr(self->dest_str, buf);
         }
      }
      break;
   case DTL_SV_U64:
      {
         uint64_t val = dtl_sv_to_u64(sv, NULL);
         if (self->dest_file != NULL)
         {
            fprintf(self->dest_file, "%" PRIu64, val);
         }
         else
         {
            snprintf(buf, sizeof(buf), "%" PRIu64, val);
            adt_str_append_cstr(self->dest_str, buf);
         }
      }
      break;
   case DTL_SV_BOOL:
      {
         const char *val_str = dtl_sv_to_bool(sv, NULL) ? "true" : "false";
         dtl_json_writer_print(self, val_str);
      }
      break;
   case DTL_SV_STR:
      {
         const char *val_str = dtl_sv_to_cstr((dtl_sv_t*) sv, &ok);
         if (ok && (val_str != NULL))
         {
            dtl_json_writer_putc(self, '"');
            dtl_json_writer_print(self, val_str);
            dtl_json_writer_putc(self, '"');
         }
         else
         {
            return DTL_CONVERSION_ERROR;
         }
      }
      break;
   default:
      break;
   }
   return DTL_NO_ERROR;
}

static dtl_error_t dtl_json_writer_write_av(dtl_json_writer_t *self, const dtl_av_t *av, bool indent_enable)
{
   int32_t i;
   int32_t array_len = dtl_av_length(av);
   if (indent_enable)
   {
      dtl_json_writer_indented_cstr(self, "[");
   }
   else
   {
      dtl_json_writer_putc(self, '[');
   }
   if (array_len > 0)
   {
      dtl_json_writer_increase_indent(self);
      if (self->current_indent == 0)
      {
         const char *separator_str = ", ";
         for (i = 0; i < array_len; i++)
         {
            dtl_dv_t *child_elem = dtl_av_value(av, i);
            if (i > 0)
            {
               dtl_json_writer_print(self, separator_str);
            }
            if (child_elem != NULL)
            {
               dtl_json_writer_write_dv(self, child_elem, true);
            }
            else
            {
               dtl_json_writer_print(self, "null");
            }
         }
      }
      else
      {
         const char *separator_str = ",";
         dtl_json_writer_print(self, self->newline_str);
         for (i = 0; i < array_len; i++)
         {
            dtl_dv_t *child_elem = dtl_av_value(av, i);
            if (child_elem != NULL)
            {
               dtl_json_writer_write_dv(self, child_elem, true);
            }
            else
            {
               dtl_json_writer_indented_cstr(self, "null");
            }
            if (i < (array_len - 1))
            {
               dtl_json_writer_print(self, separator_str);
            }
            dtl_json_writer_print(self, self->newline_str);
         }
      }
      dtl_json_writer_decrease_indent(self);
   }
   dtl_json_writer_indented_cstr(self, "]");
   return DTL_NO_ERROR;
}

static dtl_error_t dtl_json_writer_write_hv(dtl_json_writer_t *self, const dtl_hv_t *hv, bool indent_enable)
{
   int32_t i;
   int32_t num_keys;
   dtl_av_t *keys = dtl_hv_keys(hv);
   if (keys == NULL)
   {
      return DTL_MEM_ERROR;
   }
   if (self->sort_keys)
   {
      dtl_error_t error_code = dtl_av_sort(keys, NULL, false);
      if (error_code != DTL_NO_ERROR)
      {
         dtl_dec_ref(keys);
         return error_code;
      }
   }
   num_keys = dtl_av_length(keys);
   if (indent_enable)
   {
      dtl_json_writer_indented_cstr(self, "{");
   }
   else
   {
      dtl_json_writer_putc(self, '{');
   }
   if (num_keys > 0)
   {
      dtl_json_writer_increase_indent(self);
      if (self->current_indent == 0)
      {
         const char *separator_str = ", ";

         for (i = 0; i < num_keys; i++)
         {
            bool ok = false;
            const char *key = dtl_sv_to_cstr((dtl_sv_t*) dtl_av_value(keys, i), &ok);
            if (ok)
            {
               if (i > 0)
               {
                  dtl_json_writer_print(self, separator_str);
               }
               dtl_json_writer_putc(self, '"');
               dtl_json_writer_print(self, key);
               dtl_json_writer_print(self, "\": ");
               dtl_dv_t *value = (dtl_dv_t*) dtl_hv_get_cstr(hv, key);
               dtl_json_writer_write_dv(self, value, true);
            }
         }
      }
      else
      {
         const char *separator_str = ",";
         dtl_json_writer_print(self, self->newline_str);
         for (i = 0; i < num_keys; i++)
         {
            bool ok = false;
            const char *key = dtl_sv_to_cstr((dtl_sv_t*) dtl_av_value(keys, i), &ok);
            if (ok)
            {
               dtl_json_writer_write_indent_str(self);
               dtl_json_writer_putc(self, '"');
               dtl_json_writer_print(self, key);
               dtl_json_writer_print(self, "\": ");
               dtl_dv_t *value = (dtl_dv_t*) dtl_hv_get_cstr(hv, key);
               dtl_json_writer_write_dv(self, value, false);
               if (i < (num_keys - 1))
               {
                  dtl_json_writer_print(self, separator_str);
               }
               dtl_json_writer_print(self, self->newline_str);
            }
         }
      }
      dtl_json_writer_decrease_indent(self);
   }
   dtl_json_writer_indented_cstr(self, "}");
   dtl_dec_ref(keys);
   return DTL_NO_ERROR;
}

static void dtl_json_writer_print(dtl_json_writer_t *self, const char *str)
{
   if (self->dest_file != NULL)
   {
      fprintf(self->dest_file, "%s", str);
   }
   else
   {
      adt_str_append_cstr(self->dest_str, str);
   }
}

static void dtl_json_writer_putc(dtl_json_writer_t *self, char c)
{
   if (self->dest_file != NULL)
   {
      fputc(c, self->dest_file);
   }
   else
   {
      adt_str_push(self->dest_str, c);
   }
}

static void dtl_json_writer_indented_cstr(dtl_json_writer_t *self, const char *str)
{
   dtl_json_writer_write_indent_str(self);
   dtl_json_writer_print(self, str);
}

static void dtl_json_writer_write_indent_str(dtl_json_writer_t *self)
{
   if (self->current_indent > 0)
   {
      if (self->dest_file != NULL)
      {
         fprintf(self->dest_file, "%s", (const char *) adt_bytearray_data(self->indent_array));
      }
      else
      {
         adt_str_append_cstr(self->dest_str, (const char *) adt_bytearray_data(self->indent_array));
      }
   }
}
