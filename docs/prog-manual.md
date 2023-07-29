API Document
============

This document complements the comments in the source code header files in
describing the API of the SafeTypes library.

Creating Objects
================

There are 2 recommended ways to create objects

1. declare a typed pointer and allocate its space from heap like this:

```
st{SpecificType}_t *x = calloc(1, st{SpecificType}_t);
st{SpecificType}Init(x, ...);

...  using the object ...

st{SpecificType}Final(x);
free(x);
```

2. declare the typed object directly in a function or as
   an external declaration like this:

```
st{SpecificType}_t x;
st{SpecificType}Init(&x, ...);

... using the object ...

st{SpecificType}Final(&x);
```

For the second way of creating objects, its reference count should
always be greater than 0, and when assigining to containers, always
use the "keep" semantic.

Operating on Reference Count
============================

In general, users should be using `stObjRetain` and `stObjRelease` to
operate on objects in lexical variables. `stObjKeep` and `stObjLeave` are
reserved internally for implementing container types - they should not be
used unless you're implementing container.

Enumerating Container Members
=============================

All specific types contain the `stObj_t` type as the first member with
the name `hdrObj`. On how to iterate through the members of an object, 
one can reference the following code (assuming the 1st method of 
object creation):

```
obj->hdrObj.iterp = NULL;
obj->hdrObj.iterf(obj);
for(; obj->hdrObj.iterp; obj->hdrObj.iterf(obj))
{
    // current dict key or list index can be accessed through
    // "obj->hdrObj.iterk.p" or "obj->hdrObj.iterk.i".
    ... process 1 member ...
}
```

Accessing Container
===================

The meanings of the 3 setter semantics are explained in the "README.md" file,
and in the source code header comments.

```
enum st_setter_semantics {
    st_setter_kept = 1, // ``++ keptcnt''.
    st_setter_gave = 2, // ``-- refcnt, ++keptcnt''.
    st_setter_fast = 3, // same as '*_gave', except no GC.
};
```

There are 3 access function return values:

```
enum st_access_retvals {
    st_access_error = -1,
    st_access_nullval = 0,
    st_access_success = 1,
};
```

Success returns are `st_access_success` and `st_access_nullval`.

- `*_nullval` is returned by getter functions when a non-existent
  member is requested, otherwise
  
- `*_success` is returned.

- `st_access_error` is only returned when **internal** errors occur, 
  which are distinct from what normally would occur with container
  access and are considered exceptional.
