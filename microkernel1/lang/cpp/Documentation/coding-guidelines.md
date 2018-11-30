Coding Guidelines
=================
This document describes the coding style of C (C11) and C++ (C++14) source code for Resea.  It
aims to enhance readability and avoid misinterpretation.


Header files
------------
- Write inline functions instead of macro functions.
- Add a include guard into a header file like: `#ifndef __VIRTIO_REGS_H__`


Indentations
------------
- 4 spaces per indentation level.
- The maximum of a line is 79 characters.


Identifiers
------------
- Identifiers should be `lower_case_with_underscores`.
- Identifiers should be brief without ambiguous abbreviation.
- Identifiers of global variables and non-static functions must be
  starts with package name: `FOO_`, `foo_`, `_foo_`, for instance.


Comments
--------
- Use `//` for most comments. For description of a function, use `/** */`, a
  comment style recognized by Doxygen.
- Write comments a lot in brief and easy English.


Statements
----------
- Add spaces like: `if (expr) {`.


C++ features
------------
- Do not use exceptions.
- Do not use RTTI for now. It is not supported yet.


Articles you should read
------------------------
- [Effective Modern C++](http://shop.oreilly.com/product/0636920033707.do)


Examples
--------
```c
/**
 *  Returns the sum of arguments
 *
 *  The first line should be descriptive (e.g. `Returns`, not `Return`) and should
 *  not include a period the end of line like above.
 *
 *  Others are not you. Write comments in detail in brief and easy English!
 *
 *  @param[in] a  The integer.
 *  @param[in] b  The integer.
 *  @returns  The sum of arguments.
 *
 */
int sum(int a, int b) {

    return a + b;
}
```

