dtl-json
========

**dtl-json** is a lightweight, high-performance JSON parser and serializer written in C (C99 and later). It is built directly on top of the `dtl-type <https://github.com/cogu/dtl-type>`_ dynamic type system and provides seamless bidirectional conversion between JSON documents and reference-counted dynamic value trees.

Features
--------

- **Complete JSON Data Model**: Maps JSON objects, arrays, strings, numbers, booleans, and null directly to polymorphic dynamic values (``dtl_dv_t``, ``dtl_hv_t``, ``dtl_av_t``, ``dtl_sv_t``).
- **Multiple Input Sources**: Parse JSON directly from open file streams (``FILE*``), null-terminated C strings, length-bounded byte buffers, or managed strings (``adt_str_t``).
- **Flexible Serialization**: Serialize dynamic data trees to files or newly allocated strings with configurable indentation (pretty-printing or compact) and optional alphabetical key sorting.
- **Reference-Counted Memory Management**: Uses the hierarchical reference-counting model provided by ``dtl-type`` for clean memory cleanup.

Usage Examples
--------------

Parsing a JSON String
~~~~~~~~~~~~~~~~~~~~~

The following snippet demonstrates parsing a JSON object from a C string and inspecting its fields:

.. code-block:: c

   #include <stdio.h>
   #include "dtl_json.h"

   int main(void)
   {
       const char *json_text = "{\"name\": \"Alice\", \"age\": 30, \"active\": true}";
       dtl_dv_t *dv = dtl_json_load_cstr(json_text);
       if (dv == NULL)
       {
           fprintf(stderr, "Failed to parse JSON\n");
           return 1;
       }

       if (dtl_dv_type(dv) == DTL_DV_TYPE_HASH)
       {
           dtl_hv_t *hv = (dtl_hv_t*) dv;
           dtl_sv_t *name = (dtl_sv_t*) dtl_hv_get_cstr(hv, "name");
           dtl_sv_t *age = (dtl_sv_t*) dtl_hv_get_cstr(hv, "age");

           printf("Name: %s, Age: %d\n", dtl_sv_to_cstr(name, NULL), dtl_sv_to_i32(age, NULL));
       }

       dtl_dec_ref(dv);
       return 0;
   }

Writing JSON
~~~~~~~~~~~~

The following snippet demonstrates building a dynamic hash map and serializing it to formatted JSON:

.. code-block:: c

   #include <stdio.h>
   #include "dtl_json.h"

   int main(void)
   {
       dtl_hv_t *hv = dtl_hv_new();
       dtl_hv_set_cstr(hv, "title", (dtl_dv_t*) dtl_sv_make_cstr("The Hobbit"), false);
       dtl_hv_set_cstr(hv, "year", (dtl_dv_t*) dtl_sv_make_i32(1937), false);

       // Dump to stdout with 2 spaces indentation and sorted keys
       dtl_json_dump((const dtl_dv_t*) hv, stdout, 2, true);

       // Alternatively, dump to an adt_str_t
       adt_str_t *str = dtl_json_dumps((const dtl_dv_t*) hv, 0, false);
       if (str != NULL)
       {
           printf("Compact: %s\n", adt_str_cstr(str));
           adt_str_delete(str);
       }

       dtl_dec_ref(hv);
       return 0;
   }

.. toctree::
   :maxdepth: 2
   :hidden:
   :caption: API Reference

   dtl_json

Components Catalog
==================

Below is a summary of the module provided by the dtl-json library:

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - Module
     - Header
     - Description
   * - :doc:`dtl_json`
     - ``dtl_json.h``
     - JSON parsing and serialization interface
