![unit tests](https://github.com/cogu/dtl_json/workflows/unit%20tests/badge.svg)

# dtl_json
This a JSON parser and writer library built on top of [dtl_type](https://github.com/cogu/dtl_type).

## Where is it used?

* [cogu/c-apx](https://github.com/cogu/c-apx)

This repo is a submodule of the [cogu/c-apx](https://github.com/cogu/c-apx) (top-level) project.

## Dependencies

* [cogu/adt](https://github.com/cogu/adt)
* [cogu/bstr](https://github.com/cogu/bstr)
* [cogu/dtl_type](https://github.com/cogu/dtl_type)
* [cogu/cutil](https://github.com/cogu/cutil)

The unit test project(s) assume that the repos are cloned side-by-side to a common directory as seen below.

* adt
* bstr
* cutil
* dtl_type
* dtl_json (this repo)

### Git Example

```bash
cd ~
mkdir repo && cd repo
git clone https://github.com/cogu/adt.git
git clone https://github.com/cogu/bstr.git
git clone https://github.com/cogu/cutil.git
git clone https://github.com/cogu/dtl_json.git
git clone https://github.com/cogu/dtl_type.git
cd dtl_json
```

## Building with CMake

First clone this repo and its dependencies into a common directory (such as ~/repo) as seen above. Alternatively the repos can be submodules of a top-level repo (as seen in [cogu/c-apx](https://github.com/cogu/c-apx)).

### Running unit tests (Linux)

Configure:

```sh
cmake -S . -B build -DUNIT_TEST=ON
```

Build:

```sh
cmake --build build --target dtl_json_unit
```

Run test cases:

```cmd
cd build && ctest
```

### Running unit tests (Windows with Visual Studio)

Use a command prompt provided by your Visual Studio installation.
For example, I use "x64 Native Tools Command Prompt for VS2019" which is found on the start menu.
It conveniently comes with CMake pre-installed which generates Visual Studio projects by default.

Configure:

```cmd
cmake -S . -B VisualStudio -DUNIT_TEST=ON
```

Build:

```cmd
cmake --build VisualStudio --config Debug --target dtl_json_unit
```

Run test cases:

```cmd
cd VisualStudio && ctest
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

**`int32_t dtl_json_dump(const dtl_dv_t *dv, FILE *fh, int32_t indent, bool sortKeys)`**

Writes the dynamic value (dv) to the file fh. The file must have already been openend for writing.
If the indent variable is greater than zero it uses that many spaces as indentation (with additional newlines).
If sortKeys is true it will alphabetically sort object keys before they are written to the file.
The caller is also responsible for closing the openend file upon the return of this function.

**`adt_str_t* dtl_json_dumps(const dtl_dv_t *dv, int32_t indent, bool sortKeys)`**

Writes the dynamic value (dv) to a string which is returned by the function. The caller is responsible for deleting the string when it's no longer needed.
Remaining arguments are the same as above.

### Reading JSON

**`dtl_dv_t* dtl_json_load(FILE *fh)`**

Parses the JSON document from the file in fh. The file must have previously been openend with read-access before calling this function.
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for closing the file as well as deleting the dynamic value once it's no longer needed (use dtl_dec_ref).


**`dtl_dv_t* dtl_json_load_cstr(const char *cstr)`**

Parses the JSON document from the null-terminated string (cstr).
It returns a dynamic value containing a data structure based on the parsed content.
The caller is responsible for deleting the dynamic value when it's no longer needed (use dtl_dec_ref(dv) to decrease reference count to 0).

## Known Limitations

This library is in early stages of development and has many limitations:

### Numbers

* Only supports (signed/unsigned) integers as valid number format.

### Strings

* UTF-8 strings should work but needs more testing.
* No support for UTF-16.
* Escaping unicode literals using the \uxxxx format is only partially implemented and is discouraged.

## Usage Example

``` C
#include <stdio.h>
#include <stdlib.h>
#include "dtl_json.h"

/**
 * Stores and reads object containing US state population count for the 10 most populated states
 */
void list_state_population(void)
{
   FILE *fh;
   dtl_hv_t *hv;
   dtl_hv_t *state_pop = dtl_hv_new();
   dtl_hv_set_cstr(state_pop, "California", (dtl_dv_t*) dtl_sv_make_i32(39557045), false);
   dtl_hv_set_cstr(state_pop, "Texas", (dtl_dv_t*) dtl_sv_make_i32(28701845), false);
   dtl_hv_set_cstr(state_pop, "Florida", (dtl_dv_t*) dtl_sv_make_i32(21299325), false);
   dtl_hv_set_cstr(state_pop, "New York", (dtl_dv_t*) dtl_sv_make_i32(19542209), false);
   dtl_hv_set_cstr(state_pop, "Pennsylvania", (dtl_dv_t*) dtl_sv_make_i32(12807060), false);
   dtl_hv_set_cstr(state_pop, "Illinois", (dtl_dv_t*) dtl_sv_make_i32(12741080), false);
   dtl_hv_set_cstr(state_pop, "Ohio", (dtl_dv_t*) dtl_sv_make_i32(11689442), false);
   dtl_hv_set_cstr(state_pop, "Georgia", (dtl_dv_t*) dtl_sv_make_i32(10519475), false);
   dtl_hv_set_cstr(state_pop, "North Carolina", (dtl_dv_t*) dtl_sv_make_i32(10383620), false);
   dtl_hv_set_cstr(state_pop, "Michigan", (dtl_dv_t*) dtl_sv_make_i32(9995915), false);
   fh = fopen("state_pop.json", "w");
   if (fh != NULL)
   {
      const int32_t indent = 3;
      bool sortKeys = true;
      dtl_json_dump((dtl_dv_t*) state_pop, fh, indent, sortKeys);
      fclose(fh);
   }
   dtl_dec_ref(state_pop);
   hv = NULL;
   fh = fopen("state_pop.json", "r");
   if (fh != 0)
   {
      hv = (dtl_hv_t*) dtl_json_load(fh);
      fclose(fh);
   }
   if (hv != NULL)
   {
      int32_t i;
      int32_t numKeys;
      dtl_av_t *keys = dtl_hv_keys(hv);
      dtl_av_sort(keys, NULL, false);
      numKeys = dtl_av_length(keys);
      for (i = 0; i < numKeys; i++)
      {
         dtl_sv_t *key = (dtl_sv_t*) dtl_av_value(keys, i);
         dtl_sv_t *value = (dtl_sv_t*) dtl_hv_get_cstr(hv, dtl_sv_to_cstr(key));
         printf("%s : %d\n", dtl_sv_to_cstr(key), dtl_sv_to_i32(value, NULL));
      }
      dtl_dec_ref(keys);
      dtl_dec_ref(hv);
   }
}

dtl_hv_t *make_dtl_book(const char *title, const char *author, int32_t yearPublished)
{
   dtl_hv_t *book = dtl_hv_new();
   dtl_hv_set_cstr(book, "Title", (dtl_dv_t*) dtl_sv_make_cstr(title), false);
   dtl_hv_set_cstr(book, "Author", (dtl_dv_t*) dtl_sv_make_cstr(author), false);
   dtl_hv_set_cstr(book, "YearPublished", (dtl_dv_t*) dtl_sv_make_i32(yearPublished), false);
   return book;
}

dtl_hv_t *make_dtl_book_double_author(const char *title, const char *author1, const char *author2, int32_t yearPublished)
{
   dtl_hv_t *book = dtl_hv_new();
   dtl_av_t *authors = dtl_av_new();
   dtl_av_push(authors, (dtl_dv_t*) dtl_sv_make_cstr(author1), false);
   dtl_av_push(authors, (dtl_dv_t*) dtl_sv_make_cstr(author2), false);
   dtl_hv_set_cstr(book, "Title", (dtl_dv_t*) dtl_sv_make_cstr(title), false);
   dtl_hv_set_cstr(book, "Author", (dtl_dv_t*) authors, false);
   dtl_hv_set_cstr(book, "YearPublished", (dtl_dv_t*) dtl_sv_make_i32(yearPublished), false);
   return book;
}


/**
 * Dumps then loads a list of objects containing book information
 */
void list_books(void)
{
   FILE *fh;
   dtl_av_t *books = dtl_av_new();
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book("Code Complete", "Steve McConnel", 1993), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book("Clean Code", "Robert C. Martin", 2008), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book_double_author("The Pragmatic Programmer", "Andy Hunt", "Dave Thomas", 1999), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book_double_author("Refactoring", "Martin Fowler", "Kent Beck", 1999), false);
   fh = fopen("books.json", "w");
   if (fh != 0)
   {
      const int32_t indent = 3;
      bool sortKeys = true;
      dtl_json_dump((dtl_dv_t*) books, fh, indent, sortKeys);
      fclose(fh);
   }
   dtl_dec_ref(books);
   books = NULL;
   fh = fopen("books.json", "r");
   if (fh != NULL)
   {
      books = (dtl_av_t*) dtl_json_load(fh);
      fclose(fh);
   }
   if (books != NULL)
   {
      int32_t i;
      int32_t numBooks = dtl_av_length(books);
      for (i=0; i < numBooks; i++)
      {
         dtl_dv_t *dv_author;
         dtl_sv_t *sv_author;
         dtl_hv_t *book = (dtl_hv_t*) dtl_av_value(books, i);

         printf("%s:\n", dtl_sv_to_cstr((dtl_sv_t*) dtl_hv_get_cstr(book, "Title")));
         dv_author = dtl_hv_get_cstr(book, "Author");
         if (dtl_dv_type(dv_author) == DTL_DV_ARRAY)
         {
            int32_t j;
            int32_t numAuthors;
            dtl_av_t *authors = (dtl_av_t*) dv_author;
            numAuthors = dtl_av_length(authors);
            printf("   Authors: ");
            for (j=0; j<numAuthors; j++)
            {
               sv_author = (dtl_sv_t*) dtl_av_value(authors, j);
               if (j > 0)
               {
                  printf(", ");
               }
               printf("%s", dtl_sv_to_cstr(sv_author));
            }
            printf("\n");
         }
         else
         {
            sv_author = (dtl_sv_t*) dv_author;
            printf("   Author: %s\n", dtl_sv_to_cstr(sv_author));
         }
         printf("   Published: %s\n", dtl_sv_to_cstr((dtl_sv_t*) dtl_hv_get_cstr(book, "YearPublished")));
         printf("\n");
      }
      dtl_dec_ref(books);
   }
}

int main(int argc, char **argv)
{
   list_state_population();
   printf("\n---\n\n");
   list_books();

   return 0;
}
```

Output:

``` text
California : 39557045
Florida : 21299325
Georgia : 10519475
Illinois : 12741080
Michigan : 9995915
New York : 19542209
North Carolina : 10383620
Ohio : 11689442
Pennsylvania : 12807060
Texas : 28701845

---

Code Complete:
   Author: Steve McConnel
   Published: 1993

Clean Code:
   Author: Robert C. Martin
   Published: 2008

The Pragmatic Programmer:
   Authors: Andy Hunt, Dave Thomas
   Published: 1999

Refactoring:
   Authors: Martin Fowler, Kent Beck
   Published: 1999
```
