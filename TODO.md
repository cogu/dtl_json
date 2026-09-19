# DTL JSON Submodule TODO List

This document tracks identified architectural enhancements, error handling redesign, and API improvements across the `dtl_json` library for future branches/releases.

---

## 1. Redesign `dtl_json_error_t` to Extend `dtl_error_t`

### Background
Currently, [include/dtl_json.h](file:///home/cogu/repo/dtl_json/include/dtl_json.h) defines its own set of error codes starting from index 0:

```c
typedef int32_t dtl_json_error_t;

#define DTL_JSON_NO_ERROR                 ((dtl_json_error_t) 0) /**< No error */
#define DTL_JSON_MEM_ERROR                ((dtl_json_error_t) 1) /**< Memory allocation error */
#define DTL_JSON_UNEXPECTED_CHAR_ERROR    ((dtl_json_error_t) 2) /**< Unexpected character in input */
#define DTL_JSON_UNEXPECTED_EOB_ERROR     ((dtl_json_error_t) 3) /**< Premature end of buffer */
#define DTL_JSON_EMPTY_KEY_ERROR          ((dtl_json_error_t) 4) /**< Empty object key error */
#define DTL_JSON_UNMATCHED_STRING_LITERAL ((dtl_json_error_t) 5) /**< Unmatched string literal */
```

This creates numerical collisions and conceptual friction with `dtl_error_t` defined in `dtl-type` (`dtl_error.h`):

* `DTL_JSON_MEM_ERROR` (1) collides with `DTL_INVALID_ARGUMENT_ERROR` (1).
* `DTL_JSON_UNEXPECTED_CHAR_ERROR` (2) collides with `DTL_MEM_ERROR` (2).
* `DTL_JSON_UNEXPECTED_EOB_ERROR` (3) collides with `DTL_NOT_IMPLEMENTED_ERROR` (3).
* `DTL_JSON_EMPTY_KEY_ERROR` (4) collides with `DTL_TYPE_ERROR` (4).
* `DTL_JSON_UNMATCHED_STRING_LITERAL` (5) collides with `DTL_CONVERSION_ERROR` (5).

Additionally, writer functions like `dtl_json_dump` return `dtl_error_t` (from `dtl-type`), while reader components internally track `dtl_json_error_t`.

### Proposed Solution
Unify the error type hierarchy by having `dtl_json_error_t` extend `dtl_error_t`:

1. **Type Alias**:
   ```c
   typedef dtl_error_t dtl_json_error_t;
   ```

2. **Reuse Standard Error Codes**:
   - `DTL_NO_ERROR` (0) for success.
   - `DTL_MEM_ERROR` for memory allocation failures.
   - `DTL_INVALID_ARGUMENT_ERROR` for invalid pointer/argument inputs.

3. **Offset JSON-Specific Error Codes**:
   Define JSON-specific parser errors with a distinct base offset (e.g., `DTL_JSON_ERROR_BASE = 100` or starting after core DTL errors) to prevent collisions:
   ```c
   #define DTL_JSON_ERROR_BASE               ((dtl_error_t) 100)
   #define DTL_JSON_UNEXPECTED_CHAR_ERROR    (DTL_JSON_ERROR_BASE + 1)
   #define DTL_JSON_UNEXPECTED_EOB_ERROR     (DTL_JSON_ERROR_BASE + 2)
   #define DTL_JSON_EMPTY_KEY_ERROR          (DTL_JSON_ERROR_BASE + 3)
   #define DTL_JSON_UNMATCHED_STRING_LITERAL (DTL_JSON_ERROR_BASE + 4)
   #define DTL_JSON_SYNTAX_ERROR             (DTL_JSON_ERROR_BASE + 5)
   ```

4. **Error Formatting Utility**:
   Provide a helper function to convert error codes into human-readable descriptions:
   ```c
   const char* dtl_json_error_str(dtl_json_error_t err);
   ```

### Tasks
- [ ] Redefine `dtl_json_error_t` as `dtl_error_t` in [include/dtl_json.h](file:///home/cogu/repo/dtl_json/include/dtl_json.h).
- [ ] Rebase JSON parser error codes with a dedicated offset to eliminate collisions with core `dtl_error_t`.
- [ ] Update [src/dtl_json_reader.c](file:///home/cogu/repo/dtl_json/src/dtl_json_reader.c) to use the updated error definitions.
- [ ] Implement `dtl_json_error_str` in `src/dtl_json_reader.c` and declare it in `include/dtl_json.h`.
- [ ] Add unit tests verifying error code values and string formatting.

---

## 2. Expose Parser Diagnostics & Error Reporting to Callers

### Background
Currently, the public loader functions ([dtl_json_load](file:///home/cogu/repo/dtl_json/include/dtl_json.h#L66), `dtl_json_loads`, `dtl_json_load_cstr`, `dtl_json_load_bstr`) return `NULL` on failure without providing the caller with any indication of why or where parsing failed.

Internally, `dtl_json_reader_t` in [src/dtl_json_reader.c](file:///home/cogu/repo/dtl_json/src/dtl_json_reader.c#L65-L66) already tracks:
* `dtl_json_error_t last_error;`
* `uint32_t line_number;`

However, this diagnostic state is discarded when `dtl_json_reader_destroy(&reader)` is called before returning `NULL`.

### Proposed Solution
Introduce a diagnostics struct and extended parser functions (or out-parameter support):

1. **Diagnostic Struct**:
   ```c
   typedef struct dtl_json_error_info_tag {
       dtl_json_error_t error;
       uint32_t line;
       uint32_t column;
   } dtl_json_error_info_t;
   ```

2. **Extended Parse Functions**:
   Add extended functions allowing callers to receive failure diagnostics:
   ```c
   dtl_dv_t* dtl_json_load_cstr_ext(const char *str, dtl_json_error_info_t *error_info);
   dtl_dv_t* dtl_json_load_bstr_ext(const uint8_t *begin, const uint8_t *end, dtl_json_error_info_t *error_info);
   dtl_dv_t* dtl_json_load_ext(FILE *fh, dtl_json_error_info_t *error_info);
   ```
   Or an explicit error-returning signature:
   ```c
   dtl_error_t dtl_json_parse_cstr(const char *str, dtl_dv_t **out_dv, dtl_json_error_info_t *error_info);
   ```

3. **Add Column Tracking**:
   Enhance `dtl_json_reader_t` to track column offset within the line for precise syntax error reporting.

### Tasks
- [ ] Define `dtl_json_error_info_t` in [include/dtl_json.h](file:///home/cogu/repo/dtl_json/include/dtl_json.h).
- [ ] Add column tracking in [src/dtl_json_reader.c](file:///home/cogu/repo/dtl_json/src/dtl_json_reader.c).
- [ ] Implement `*_ext` parser variants with optional `dtl_json_error_info_t *` parameter.
- [ ] Add unit tests verifying line number, column, and error code reporting on malformed JSON inputs.

---

## 3. Error Handling Consistency Between Reader and Writer

### Background
The current API has asymmetric error handling styles:
* Writer: `dtl_json_dump` returns `dtl_error_t`, while `dtl_json_dumps` returns `adt_str_t*` (`NULL` on failure).
* Reader: Returns `dtl_dv_t*` (`NULL` on failure).

### Tasks
- [ ] Consider adding an extended variant for string serialization:
  ```c
  dtl_error_t dtl_json_dumps_ext(const dtl_dv_t *dv, int32_t indent, bool sort_keys, adt_str_t **out_str);
  ```
- [ ] Ensure all writer functions consistently propagate `dtl_error_t` from underlying container operations.

---

## 4. Repository & Documentation Updates

- [ ] Rename repository and references to `dtl-json` once GitHub repo rename occurs.
- [ ] Update [README.md](file:///home/cogu/repo/dtl_json/README.md) to reflect `dtl-json` and `dtl-type`.
- [ ] Update Sphinx documentation `project = 'dtl-json'` in [docs/conf.py](file:///home/cogu/repo/dtl_json/docs/conf.py) and [docs/Doxyfile](file:///home/cogu/repo/dtl_json/docs/Doxyfile).
