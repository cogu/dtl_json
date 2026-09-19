/*****************************************************************************
* \file      dtl_json.h
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     DTL-powered JSON parser and writer
*
* Copyright (c) 2019-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/
#ifndef DTL_JSON_H_
#define DTL_JSON_H_

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "dtl_type.h"
#include "adt_str.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
typedef int32_t dtl_json_error_t;

#define DTL_JSON_NO_ERROR                 ((dtl_json_error_t) 0) /**< No error */
#define DTL_JSON_MEM_ERROR                ((dtl_json_error_t) 1) /**< Memory allocation error */
#define DTL_JSON_UNEXPECTED_CHAR_ERROR    ((dtl_json_error_t) 2) /**< Unexpected character in input */
#define DTL_JSON_UNEXPECTED_EOB_ERROR     ((dtl_json_error_t) 3) /**< Premature end of buffer */
#define DTL_JSON_EMPTY_KEY_ERROR          ((dtl_json_error_t) 4) /**< Empty object key error */
#define DTL_JSON_UNMATCHED_STRING_LITERAL ((dtl_json_error_t) 5) /**< Unmatched string literal */

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////

/**
 * \brief Dumps a dynamic value as JSON to an open file handle.
 *
 * \param dv The dynamic value to serialize.
 * \param fh File handle open for writing.
 * \param indent Number of spaces to use for indentation. Set to 0 or negative for compact output.
 * \param sort_keys If true, object keys are sorted alphabetically before writing.
 * \return DTL_NO_ERROR (0) on success, or non-zero error code.
 */
dtl_error_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sort_keys);

/**
 * \brief Dumps a dynamic value as JSON into a newly allocated string.
 *
 * \param dv The dynamic value to serialize.
 * \param indent Number of spaces to use for indentation. Set to 0 or negative for compact output.
 * \param sort_keys If true, object keys are sorted alphabetically before writing.
 * \return Pointer to newly allocated adt_str_t, or NULL on failure. Caller owns the returned string.
 */
adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sort_keys);

/**
 * \brief Parses a JSON document from an open file handle.
 *
 * \param fh File handle open for reading.
 * \return Pointer to parsed dtl_dv_t, or NULL on error. Caller must release with dtl_dec_ref().
 */
dtl_dv_t* dtl_json_load(FILE *fh);

/**
 * \brief Parses a JSON document from an adt_str_t string.
 *
 * \param str String containing JSON text.
 * \return Pointer to parsed dtl_dv_t, or NULL on error. Caller must release with dtl_dec_ref().
 */
dtl_dv_t* dtl_json_loads(const adt_str_t *str);

/**
 * \brief Parses a JSON document from a null-terminated C string.
 *
 * \param str Null-terminated string containing JSON text.
 * \return Pointer to parsed dtl_dv_t, or NULL on error. Caller must release with dtl_dec_ref().
 */
dtl_dv_t* dtl_json_load_cstr(const char *str);

/**
 * \brief Parses a JSON document from a bounded byte buffer.
 *
 * \param begin Pointer to the beginning of the buffer.
 * \param end Pointer to one byte past the end of the buffer.
 * \return Pointer to parsed dtl_dv_t, or NULL on error. Caller must release with dtl_dec_ref().
 */
dtl_dv_t* dtl_json_load_bstr(const uint8_t *begin, const uint8_t *end);

#ifdef __cplusplus
}
#endif

#endif // DTL_JSON_H_
