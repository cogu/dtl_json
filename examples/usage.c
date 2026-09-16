/*****************************************************************************
* \file      usage.c
* \author    Conny Gustafsson
* \date      2019-08-01
* \brief     Example usage of dtl_json library
*
* Copyright (c) 2019-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dtl_json.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
void vfree(void *arg)
{
   free(arg);
}
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void list_state_population(void);
static void list_books(void);
static dtl_hv_t *make_dtl_book(const char *title, const char *author, int32_t year_published);
static dtl_hv_t *make_dtl_book_double_author(const char *title, const char *author1, const char *author2, int32_t year_published);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
   (void) argc;
   (void) argv;

   list_state_population();
   printf("\n---\n\n");
   list_books();

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

/**
 * Stores and reads object containing US state population count for the 10 most populated states
 */
static void list_state_population(void)
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
      bool sort_keys = true;
      dtl_json_dump((dtl_dv_t*) state_pop, fh, indent, sort_keys);
      fclose(fh);
   }
   dtl_dec_ref(state_pop);
   hv = NULL;
   fh = fopen("state_pop.json", "r");
   if (fh != NULL)
   {
      hv = (dtl_hv_t*) dtl_json_load(fh);
      fclose(fh);
   }
   if (hv != NULL)
   {
      int32_t i;
      int32_t num_keys;
      dtl_av_t *keys = dtl_hv_keys(hv);
      dtl_av_sort(keys, NULL, false);
      num_keys = dtl_av_length(keys);
      for (i = 0; i < num_keys; i++)
      {
         dtl_sv_t *key = (dtl_sv_t*) dtl_av_value(keys, i);
         dtl_sv_t *value = (dtl_sv_t*) dtl_hv_get_cstr(hv, dtl_sv_to_cstr(key, NULL));
         printf("%s : %d\n", dtl_sv_to_cstr(key, NULL), dtl_sv_to_i32(value, NULL));
      }
      dtl_dec_ref(keys);
      dtl_dec_ref(hv);
   }
}

static dtl_hv_t *make_dtl_book(const char *title, const char *author, int32_t year_published)
{
   dtl_hv_t *book = dtl_hv_new();
   dtl_hv_set_cstr(book, "Title", (dtl_dv_t*) dtl_sv_make_cstr(title), false);
   dtl_hv_set_cstr(book, "Author", (dtl_dv_t*) dtl_sv_make_cstr(author), false);
   dtl_hv_set_cstr(book, "YearPublished", (dtl_dv_t*) dtl_sv_make_i32(year_published), false);
   return book;
}

static dtl_hv_t *make_dtl_book_double_author(const char *title, const char *author1, const char *author2, int32_t year_published)
{
   dtl_hv_t *book = dtl_hv_new();
   dtl_av_t *authors = dtl_av_new();
   dtl_av_push(authors, (dtl_dv_t*) dtl_sv_make_cstr(author1), false);
   dtl_av_push(authors, (dtl_dv_t*) dtl_sv_make_cstr(author2), false);
   dtl_hv_set_cstr(book, "Title", (dtl_dv_t*) dtl_sv_make_cstr(title), false);
   dtl_hv_set_cstr(book, "Author", (dtl_dv_t*) authors, false);
   dtl_hv_set_cstr(book, "YearPublished", (dtl_dv_t*) dtl_sv_make_i32(year_published), false);
   return book;
}

/**
 * Dumps then loads a list of objects containing book information
 */
static void list_books(void)
{
   FILE *fh;
   dtl_av_t *books = dtl_av_new();
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book("Code Complete", "Steve McConnel", 1993), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book("Clean Code", "Robert C. Martin", 2008), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book_double_author("The Pragmatic Programmer", "Andy Hunt", "Dave Thomas", 1999), false);
   dtl_av_push(books, (dtl_dv_t*) make_dtl_book_double_author("Refactoring", "Martin Fowler", "Kent Beck", 1999), false);
   fh = fopen("books.json", "w");
   if (fh != NULL)
   {
      const int32_t indent = 3;
      bool sort_keys = true;
      dtl_json_dump((dtl_dv_t*) books, fh, indent, sort_keys);
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
      int32_t num_books = dtl_av_length(books);
      for (i = 0; i < num_books; i++)
      {
         dtl_dv_t *dv_author;
         dtl_sv_t *sv_author;
         dtl_hv_t *book = (dtl_hv_t*) dtl_av_value(books, i);

         printf("%s:\n", dtl_sv_to_cstr((dtl_sv_t*) dtl_hv_get_cstr(book, "Title"), NULL));
         dv_author = dtl_hv_get_cstr(book, "Author");
         if (dtl_dv_type(dv_author) == DTL_DV_ARRAY)
         {
            int32_t j;
            int32_t num_authors;
            dtl_av_t *authors = (dtl_av_t*) dv_author;
            num_authors = dtl_av_length(authors);
            printf("   Authors: ");
            for (j = 0; j < num_authors; j++)
            {
               sv_author = (dtl_sv_t*) dtl_av_value(authors, j);
               if (j > 0)
               {
                  printf(", ");
               }
               printf("%s", dtl_sv_to_cstr(sv_author, NULL));
            }
            printf("\n");
         }
         else
         {
            sv_author = (dtl_sv_t*) dv_author;
            printf("   Author: %s\n", dtl_sv_to_cstr(sv_author, NULL));
         }
         printf("   Published: %d\n", dtl_sv_to_i32((dtl_sv_t*) dtl_hv_get_cstr(book, "YearPublished"), NULL));
         printf("\n");
      }
      dtl_dec_ref(books);
   }
}
