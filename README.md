![unit tests](https://github.com/cogu/dtl-json/workflows/unit%20tests/badge.svg)

# dtl-json
This is a JSON parser and writer library built on top of [dtl-type](https://github.com/cogu/dtl-type).

## Where is it used?

* [cogu/c-apx](https://github.com/cogu/c-apx)

This repo is a submodule of the [cogu/c-apx](https://github.com/cogu/c-apx) (top-level) project.

## Dependencies

* [cogu/adt](https://github.com/cogu/adt)
* [cogu/bstr](https://github.com/cogu/bstr)
* [cogu/dtl-type](https://github.com/cogu/dtl-type)
* [cogu/cutil](https://github.com/cogu/cutil)

The unit test project(s) assume that the repos are cloned side-by-side to a common directory as seen below.

* adt
* bstr
* cutil
* dtl-type
* dtl-json (this repo)

### Git Example

```bash
cd ~
mkdir repo && cd repo
git clone https://github.com/cogu/adt.git
git clone https://github.com/cogu/bstr.git
git clone https://github.com/cogu/cutil.git
git clone https://github.com/cogu/dtl-type.git
git clone https://github.com/cogu/dtl-json.git
cd dtl-json
```

## Building with CMake

First clone this repo and its dependencies into a common directory (such as `~/repo`) as seen above. Alternatively the repos can be submodules of a top-level repo (as seen in [cogu/c-apx](https://github.com/cogu/c-apx)).

### Using CMake Presets (Clang 18 + Ninja)

```bash
# Run unit tests
cmake --preset clang-test
cmake --build --preset clang-test
ctest --preset clang-test

# Address and Undefined Behavior Sanitizers (ASan + UBSan)
cmake --preset clang-asan
cmake --build --preset clang-asan
ctest --preset clang-asan

# Static Analysis
cmake --preset clang-tidy
cmake --build --preset clang-tidy
```

### Manual CMake Workflows (Linux and Windows)

For Windows, use a "Native tools command prompt" from your Visual Studio installation. It comes with a cmake binary that
by default chooses the appropriate compiler version.

#### Running unit tests

Configure:

```sh
cmake -S . -B build-test -GNinja -DUNIT_TEST=ON
```

Build:

```sh
cmake --build build-test
```

Run test cases:

```sh
ctest --test-dir build-test --output-on-failure
```
## JSON and DTL type mapping

Type mapping is straightforward between JSON and DTL.

| JSON        |   DTL    |
| ------------|----------|
| Number      | dtl_sv_t |
| String      | dtl_sv_t |
| Boolean     | dtl_sv_t |
| List        | dtl_av_t |
| Object      | dtl_hv_t |

## API

The API is simple and is inspired by the Python JSON module.

### Writing JSON

**`int32_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sort_keys)`**

Writes the dynamic value (`dv`) to the file `fh`. The file must have already been opened for writing.
If the `indent` variable is greater than zero it uses that many spaces as indentation (with additional newlines).
If `sort_keys` is true it will alphabetically sort object keys before they are written to the file.
The caller is also responsible for closing the opened file upon the return of this function.

**`adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sort_keys)`**

Writes the dynamic value (`dv`) to a string which is returned by the function. The caller is responsible for deleting the string when it is no longer needed.
Remaining arguments are the same as above.

### Reading JSON

**`dtl_dv_t* dtl_json_load(FILE *fh)`**

Parses the JSON document from the file in `fh`. The file must have previously been opened with read-access before calling this function.
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for closing the file as well as deleting the dynamic value once it is no longer needed (use `dtl_dec_ref`).

**`dtl_dv_t* dtl_json_loads(const adt_str_t *str)`**

Parses the JSON document from an `adt_str_t` string.
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for deleting the dynamic value when it is no longer needed (use `dtl_dec_ref`).

**`dtl_dv_t* dtl_json_load_cstr(const char *str)`**

Parses the JSON document from the null-terminated string (`str`).
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for deleting the dynamic value when it is no longer needed (use `dtl_dec_ref(dv)` to decrease reference count to 0).

**`dtl_dv_t* dtl_json_load_bstr(const uint8_t *begin, const uint8_t *end)`**

Parses the JSON document from a bounded byte range (`[begin, end)`).
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for deleting the dynamic value when it is no longer needed (use `dtl_dec_ref(dv)` to decrease reference count to 0).

## Known Limitations

This library is in early stages of development:

### Numbers

* Supports (signed/unsigned) integers up to 64-bit.

### Strings

* UTF-8 strings are supported.
* Escaped sequences (`\"`, `\\`, `\/`, `\b`, `\f`, `\n`, `\r`, `\t`, `\uXXXX`) are supported.
* No direct support for UTF-16 encodings.

