/*****************************************************************************
* \file      dtl_json.h
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     DTL-powered JSON parser/write
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
#ifndef DTL_JSON_H
#define DTL_JSON_H

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdint.h>
#include "dtl_type.h"
#include "adt_str.h"


//////////////////////////////////////////////////////////////////////////////
// PUBLIC CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef int32_t dtl_json_error_t;

#define DTL_JSON_NO_ERROR                 ((dtl_json_error_t) 0)
#define DTL_JSON_MEM_ERROR                ((dtl_json_error_t) 1)
#define DTL_JSON_UNEXPECTED_CHAR_ERROR    ((dtl_json_error_t) 2)
#define DTL_JSON_UNEXPECTED_EOB_ERROR     ((dtl_json_error_t) 3) //EOB: End Of Buffer
#define DTL_JSON_EMPTY_KEY_ERROR          ((dtl_json_error_t) 4)
#define DTL_JSON_UNMATCHED_STRING_LITERAL ((dtl_json_error_t) 5)

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
int32_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sortKeys);
adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sortKeys);

dtl_dv_t* dtl_json_load(FILE *fh);
dtl_dv_t* dtl_json_loads(adt_str_t *str);
dtl_dv_t* dtl_json_load_cstr(const char *str);
dtl_dv_t* dtl_json_load_bstr(const uint8_t *pBegin, const uint8_t *pEnd);

#endif //DTL_JSON_H
