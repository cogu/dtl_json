JSON Parser and Writer (dtl_json)
=================================

The ``dtl_json`` module provides JSON serialization (dumping) and deserialization (parsing) functions for the ``dtl-type`` dynamic data model.

Overview
--------

``dtl_json`` translates between JSON text and dynamic value trees:

* **JSON Objects** are represented as ``dtl_hv_t`` (hash values / string-keyed maps).
* **JSON Arrays** are represented as ``dtl_av_t`` (array values / dynamic arrays).
* **JSON Strings, Numbers, and Booleans** are represented as ``dtl_sv_t`` (scalar values).
* **JSON null** is represented as the singleton null object ``dtl_dv_null()``.

Memory Ownership
----------------

* **Parsing**: All loader functions return a newly created ``dtl_dv_t`` with an initial reference count of 1. When finished, the caller must release the root dynamic value using ``dtl_dec_ref()``, which recursively frees all nested dynamic values.
* **Serialization**: ``dtl_json_dumps`` returns a newly allocated dynamic string (``adt_str_t``). The caller takes ownership of the string and is responsible for freeing it using ``adt_str_delete()``.

Error Codes
-----------

The following error codes may be returned by parsing and serialization routines:

.. list-table::
   :header-rows: 1
   :widths: 40 15 45

   * - Constant
     - Value
     - Description
   * - ``DTL_JSON_NO_ERROR``
     - 0
     - Operation completed successfully.
   * - ``DTL_JSON_MEM_ERROR``
     - 1
     - Memory allocation failed.
   * - ``DTL_JSON_UNEXPECTED_CHAR_ERROR``
     - 2
     - Unexpected or invalid character encountered in input.
   * - ``DTL_JSON_UNEXPECTED_EOB_ERROR``
     - 3
     - Premature end of buffer or input file.
   * - ``DTL_JSON_EMPTY_KEY_ERROR``
     - 4
     - An empty object key was encountered.
   * - ``DTL_JSON_UNMATCHED_STRING_LITERAL``
     - 5
     - Unterminated string literal or mismatched quotation marks.

API Reference
-------------

Data Types
~~~~~~~~~~

.. doxygentypedef:: dtl_json_error_t

Error Defines
~~~~~~~~~~~~~

.. doxygendefine:: DTL_JSON_NO_ERROR
.. doxygendefine:: DTL_JSON_MEM_ERROR
.. doxygendefine:: DTL_JSON_UNEXPECTED_CHAR_ERROR
.. doxygendefine:: DTL_JSON_UNEXPECTED_EOB_ERROR
.. doxygendefine:: DTL_JSON_EMPTY_KEY_ERROR
.. doxygendefine:: DTL_JSON_UNMATCHED_STRING_LITERAL

Parsing Functions
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: dtl_json_load
.. doxygenfunction:: dtl_json_loads
.. doxygenfunction:: dtl_json_load_cstr
.. doxygenfunction:: dtl_json_load_bstr

Serialization Functions
~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: dtl_json_dump
.. doxygenfunction:: dtl_json_dumps
