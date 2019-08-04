/*****************************************************************************
* \file      dtl_json_writer.c
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     DTL-powered JSON writer
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
#include "dtl_json.h"
#include "adt_bytearray.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


//////////////////////////////////////////////////////////////////////////////
// PRIVATE CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define OUTPUT_TYPE_STR  0
#define OUTPUT_TYPE_FILE 1
typedef uint8_t outputType_t;
static const char m_indentChar = ' ';

typedef struct dtl_json_writer_tag
{
   outputType_t outputType;
   int32_t indentWidth;
   int32_t currentIndent;
   adt_bytearray_t *indentArray;
   FILE *destFile;
   adt_str_t *destStr;
   const char *newLineStr;
   bool sortKeys;
} dtl_json_writer_t;

#define TMP_BUF_SIZE 64
#define INDENT_ARRAY_GROWTH 128

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_writer_createWithFile(dtl_json_writer_t *self, FILE *fh);
static adt_error_t dtl_json_writer_createWithString(dtl_json_writer_t *self);
static void dtl_json_writer_destroy(dtl_json_writer_t *self, bool keepStr);
static void dtl_json_writer_setIndentWidth(dtl_json_writer_t *self, int32_t indent);
static void dtl_json_writer_setSortKeys(dtl_json_writer_t *self, bool sortKeys);
static void dtl_json_writer_increaseIndent(dtl_json_writer_t *self);
static void dtl_json_writer_decreaseIndent(dtl_json_writer_t *self);
static void dtl_json_writer_growIndentArray(dtl_json_writer_t *self);
static dtl_error_t dtl_json_writer_write_dv(dtl_json_writer_t *self, const dtl_dv_t *dv, bool indentEnable);
static dtl_error_t dtl_json_writer_write_sv(dtl_json_writer_t *self, const dtl_sv_t *sv, bool indentEnable);
static dtl_error_t dtl_json_writer_write_av(dtl_json_writer_t *self, const dtl_av_t *av, bool indentEnable);
static dtl_error_t dtl_json_writer_write_hv(dtl_json_writer_t *self, const dtl_hv_t *hv, bool indentEnable);
static void dtl_json_writer_print(dtl_json_writer_t *self, const char *str);
static void dtl_json_writer_putc(dtl_json_writer_t *self, const char c);
static void dtl_json_writer_indented_cstr(dtl_json_writer_t *self, const char *str);
static void dtl_json_writer_write_indent_str(dtl_json_writer_t *self);


//////////////////////////////////////////////////////////////////////////////
// PRIVATE VARIABLES
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int32_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sortKeys)
{
   dtl_json_writer_t writer;
   dtl_json_writer_createWithFile(&writer, fh);
   if (indent > 0)
   {
      dtl_json_writer_setIndentWidth(&writer, indent);
   }
   if (sortKeys)
   {
      dtl_json_writer_setSortKeys(&writer, true);
   }
   dtl_json_writer_write_dv(&writer, dv, true);
   dtl_json_writer_destroy(&writer, false);
   return 0;
}

adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sortKeys)
{
   adt_str_t *retval = (adt_str_t*) 0;
   dtl_json_writer_t writer;
   dtl_json_writer_createWithString(&writer);
   if (indent > 0)
   {
      dtl_json_writer_setIndentWidth(&writer, indent);
   }
   if (sortKeys)
   {
      dtl_json_writer_setSortKeys(&writer, true);
   }
   dtl_json_writer_write_dv(&writer, dv, true);
   retval = writer.destStr;
   dtl_json_writer_destroy(&writer, true);
   return retval;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void dtl_json_writer_createWithFile(dtl_json_writer_t *self, FILE *fh)
{
   if (self != 0)
   {
      self->outputType = OUTPUT_TYPE_FILE;
      self->indentWidth = 0;
      self->currentIndent = 0;
      self->indentArray = (adt_bytearray_t*) 0;
      self->destStr = (adt_str_t*) 0;
      self->destFile = fh;
      self->newLineStr = "\n";
      self->sortKeys = false;
   }
}

static adt_error_t dtl_json_writer_createWithString(dtl_json_writer_t *self)
{
   adt_error_t retval = ADT_NO_ERROR;
   if (self != 0)
   {
      self->outputType = OUTPUT_TYPE_STR;
      self->indentWidth = 0;
      self->currentIndent = 0;
      self->indentArray = (adt_bytearray_t*) 0;
      self->destStr = adt_str_new();
      self->destFile = (FILE*) 0;
      self->sortKeys = false;
      if (self->destStr == 0)
      {
         retval = ADT_MEM_ERROR;
      }
      self->newLineStr = "\n";
   }
   else
   {
      retval = ADT_INVALID_ARGUMENT_ERROR;
   }
   return retval;
}

static void dtl_json_writer_destroy(dtl_json_writer_t *self, bool keepStr)
{
   if (self != 0)
   {
      if ( (!keepStr) && (self->destStr != 0) )
      {
         adt_str_delete(self->destStr);
      }
      if (self->indentArray != 0)
      {
         adt_bytearray_delete(self->indentArray);
      }
   }
}

static void dtl_json_writer_setIndentWidth(dtl_json_writer_t *self, int32_t indent)
{
   self->indentWidth = indent;
   if ( (self->indentWidth > 0) && (self->indentArray == 0) )
   {
      self->indentArray = adt_bytearray_new(INDENT_ARRAY_GROWTH);
   }
}

static void dtl_json_writer_setSortKeys(dtl_json_writer_t *self, bool sortKeys)
{
   self->sortKeys = sortKeys;
}

static void dtl_json_writer_increaseIndent(dtl_json_writer_t *self)
{
   if (self->indentWidth > 0)
   {
      int32_t oldIndent = self->currentIndent;
      int32_t currentLen = adt_bytearray_length(self->indentArray);
      self->currentIndent+=self->indentWidth;
      if (currentLen < self->currentIndent)
      {
         dtl_json_writer_growIndentArray(self);
      }
      else
      {
         uint8_t *data = adt_bytearray_data(self->indentArray);
         memset(&data[oldIndent], m_indentChar, self->indentWidth);
         data[self->currentIndent] = 0u;
      }
   }
}

static void dtl_json_writer_decreaseIndent(dtl_json_writer_t *self)
{
   if (self->indentWidth > 0)
   {
      if (self->currentIndent > self->indentWidth)
      {
         self->currentIndent -= self->indentWidth;
      }
      else
      {
         self->currentIndent = 0;
      }
      assert(self->indentArray != 0);
      assert(adt_bytearray_length(self->indentArray) > self->currentIndent);
      adt_bytearray_data(self->indentArray)[self->currentIndent] = 0u;
   }
}

static void dtl_json_writer_growIndentArray(dtl_json_writer_t *self)
{
   adt_error_t result = adt_bytearray_resize(self->indentArray, self->currentIndent+1);
   if (result == ADT_NO_ERROR)
   {
      uint8_t *data = adt_bytearray_data(self->indentArray);
      assert(data != 0);
      memset(data, m_indentChar, self->currentIndent);
      data[self->currentIndent] = 0u;
   }
}

static dtl_error_t dtl_json_writer_write_dv(dtl_json_writer_t *self, const dtl_dv_t *dv, bool indentEnable)
{
   if ( (self != 0) && (dv != 0) )
   {
      switch (dtl_dv_type(dv))
      {
      case DTL_DV_INVALID:
         break;
      case DTL_DV_NULL:
         break;
      case DTL_DV_SCALAR:
         dtl_json_writer_write_sv(self, (const dtl_sv_t*) dv, indentEnable);
         break;
      case DTL_DV_ARRAY:
         dtl_json_writer_write_av(self, (const dtl_av_t*) dv, indentEnable);
         break;
      case DTL_DV_HASH:
         dtl_json_writer_write_hv(self, (const dtl_hv_t*) dv, indentEnable);
         break;
      default:
         break;
      }
      return DTL_NO_ERROR;
   }
   return DTL_INVALID_ARGUMENT_ERROR;
}

static dtl_error_t dtl_json_writer_write_sv(dtl_json_writer_t *self, const dtl_sv_t *sv, bool indentEnable)
{
   char buf[TMP_BUF_SIZE];
   union {
      const char *str;
      int32_t i32;
      uint32_t u32;
   } val;
   if (indentEnable)
   {
      dtl_json_writer_write_indent_str(self);
   }
   switch(dtl_sv_type(sv))
   {
   case DTL_SV_I32:
      val.i32 = dtl_sv_to_i32(sv, NULL);
      if (self->destFile != 0)
      {
         fprintf(self->destFile, "%d", (int) val.i32);
      }
      else
      {
         sprintf(buf, "%d", (int) val.i32);
         adt_str_append_cstr(self->destStr, buf);
      }
      break;
   case DTL_SV_U32:
      val.u32 = dtl_sv_to_i32(sv, NULL);
      if (self->destFile != 0)
      {
         fprintf(self->destFile, "%u", (unsigned int) val.u32);
      }
      else
      {
         sprintf(buf, "%u", (unsigned int) val.u32);
         adt_str_append_cstr(self->destStr, buf);
      }
      break;
   case DTL_SV_BOOL:
      val.str = dtl_sv_to_bool(sv)? "true" : "false";
      if (self->destFile != 0)
      {
         fprintf(self->destFile, "%s", val.str);
      }
      else
      {
         adt_str_append_cstr(self->destStr, val.str);
      }
      break;
   case DTL_SV_STR:
      val.str = dtl_sv_to_cstr((dtl_sv_t*) sv);
      assert(val.str != 0);
      if (self->destFile != 0)
      {
         fprintf(self->destFile, "\"%s\"", val.str);
      }
      else
      {
         adt_str_push(self->destStr, '"');
         adt_str_append_cstr(self->destStr, val.str);
         adt_str_push(self->destStr, '"');
      }
      break;
   default:
      break;
   }
   return DTL_NO_ERROR;
}

static dtl_error_t dtl_json_writer_write_av(dtl_json_writer_t *self, const dtl_av_t *av, bool indentEnable)
{
   int32_t i;
   int32_t arrayLen = dtl_av_length(av);
   if (indentEnable)
   {
      dtl_json_writer_indented_cstr(self,  "[");
   }
   else
   {
      dtl_json_writer_putc(self, '[');
   }
   if (arrayLen > 0)
   {
      dtl_json_writer_increaseIndent(self);
      if (self->currentIndent == 0)
      {
         const char *separatorString = ", ";
         for (i = 0; i<arrayLen; i++)
         {
            dtl_dv_t *childElem = dtl_av_value(av, i);
            if ( i > 0)
            {
               dtl_json_writer_print(self, separatorString);
            }
            if (childElem != 0)
            {
               dtl_json_writer_write_dv(self, childElem, true);
            }
            else
            {
               dtl_json_writer_print(self, "NULL");
            }
         }
      }
      else
      {
         const char *separatorString = ",";
         dtl_json_writer_print(self, self->newLineStr);
         for (i = 0; i<arrayLen; i++)
         {
            dtl_dv_t *childElem = dtl_av_value(av, i);
            if (childElem != 0)
            {
               dtl_json_writer_write_dv(self, childElem, true);
            }
            if ( i < (arrayLen-1))
            {
               dtl_json_writer_print(self, separatorString);
            }
            dtl_json_writer_print(self, self->newLineStr);
         }
      }
      dtl_json_writer_decreaseIndent(self);
   }
   dtl_json_writer_indented_cstr(self,  "]");
   return DTL_NO_ERROR;
}

static dtl_error_t dtl_json_writer_write_hv(dtl_json_writer_t *self, const dtl_hv_t *hv, bool indentEnable)
{
   int32_t i;
   int32_t numKeys;
   dtl_av_t* keys;
   keys = dtl_hv_keys(hv);
   if (keys == 0)
   {
      return DTL_MEM_ERROR;
   }
   if (self->sortKeys)
   {
      dtl_error_t errorCode = dtl_av_sort(keys, (dtl_key_func_t*) 0, false);
      if (errorCode)
      {
         return errorCode;
      }
   }
   numKeys = dtl_av_length(keys);
   if (indentEnable)
   {
      dtl_json_writer_indented_cstr(self,  "{");
   }
   else
   {
      dtl_json_writer_putc(self, '{');
   }
   if (numKeys > 0)
   {
      dtl_json_writer_increaseIndent(self);
      if (self->currentIndent == 0)
      {
         const char *separatorString = ", ";

         for (i = 0; i<numKeys; i++)
         {
            dtl_dv_t *value;
            const char *key = (const char*) dtl_sv_to_cstr((dtl_sv_t*) dtl_av_value(keys, i));
            if ( i > 0)
            {
               dtl_json_writer_print(self, separatorString);
            }
            dtl_json_writer_putc(self, '"');
            dtl_json_writer_print(self, key);
            dtl_json_writer_print(self, "\": ");
            value = (dtl_dv_t*) dtl_hv_get_cstr(hv, key);
            dtl_json_writer_write_dv(self, value, true);
         }
      }
      else
      {
         const char *separatorString = ",";
         dtl_json_writer_print(self, self->newLineStr);
         for (i = 0; i<numKeys; i++)
         {

            dtl_dv_t *value;
            const char *key = (const char*) dtl_sv_to_cstr((dtl_sv_t*) dtl_av_value(keys, i));
            dtl_json_writer_write_indent_str(self);
            dtl_json_writer_putc(self, '"');
            dtl_json_writer_print(self, key);
            dtl_json_writer_print(self, "\": ");
            value = (dtl_dv_t*) dtl_hv_get_cstr(hv, key);
            dtl_json_writer_write_dv(self, value, false);
            if ( i < (numKeys-1))
            {
               dtl_json_writer_print(self, separatorString);
            }
            dtl_json_writer_print(self, self->newLineStr);
         }
      }
      dtl_json_writer_decreaseIndent(self);
   }
   dtl_json_writer_indented_cstr(self,  "}");
   dtl_dec_ref(keys);
   return DTL_NO_ERROR;
}


static void dtl_json_writer_print(dtl_json_writer_t *self, const char *str)
{
   if (self->destFile != 0)
   {
      fprintf(self->destFile, "%s", str);
   }
   else
   {
      adt_str_append_cstr(self->destStr, str);
   }
}

static void dtl_json_writer_putc(dtl_json_writer_t *self, const char c)
{
   if (self->destFile != 0)
   {
      fputc(c, self->destFile);
   }
   else
   {
      adt_str_push(self->destStr, c);
   }
}

static void dtl_json_writer_indented_cstr(dtl_json_writer_t *self, const char *str)
{
   dtl_json_writer_write_indent_str(self);
   dtl_json_writer_print(self, str);
}

static void dtl_json_writer_write_indent_str(dtl_json_writer_t *self)
{
   if (self->currentIndent > 0)
   {
      if (self->destFile != 0)
      {
         fprintf(self->destFile, "%s", (const char *) adt_bytearray_data(self->indentArray));
      }
      else
      {
         adt_str_append_cstr(self->destStr, (const char *) adt_bytearray_data(self->indentArray));
      }
   }
}
