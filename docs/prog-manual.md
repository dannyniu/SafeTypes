API Document
============

This document complements the comments in the source code header files in
describing the API of the SafeTypes library.

General Usage
=============

This section describe features that're common to more than one data type.

Creating Objects
----------------

There are 2 recommended ways to create objects

1. declare a typed pointer and allocate its space from heap like this:

```
st{SpecificType}_t *x = calloc(1, sizeof(st{SpecificType}_t));
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
----------------------------

In general, users should be using `stObjRetain` and `stObjRelease` to
operate on objects in lexical variables. `stObjKeep` and `stObjLeave` are
reserved internally for implementing container types - they should not be
used unless you're implementing container.

Enumerating Container Members
-----------------------------

All specific types contain the `stObj_t` type as the first member with
the name `hdrObj`. On how to iterate through the members of an object, 
one can reference the following code (assuming the 1st method of 
object creation):

```
obj->hdrObj.iterp = NULL; // reset enumerator state.
obj->hdrObj.iterf(obj); // (re-)initialize enumerator state.
for(; obj->hdrObj.iterp; obj->hdrObj.iterf(obj)) // loop control statement.
{
    // current dict key or list index can be accessed through
    // "obj->hdrObj.iterk.p" - a pointer to void, or
    // "obj->hdrObj.iterk.i" - an unsigned machine word.
    ... process 1 member ...
}
```

Accessing Container
-------------------

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

(No) Thread Safety
------------------

All interfaces in SafeTypes are unsafe with respect to concurrent access from 
multiple threads. This is a deliberate design decision, intended to decrease 
unecessary overhead. 

Data Types' Description
=======================

This section describe each specific types.

`stObj_t`
---------

This is generic "root" type of all types under the SafeTypes type hierarchy.

`stData_t`
----------

This type is used to hold arbitrary data. The design goal has been to make it
easy to safely access datum within the address range of the underlaying buffer,
and to safely resize the buffer when needed.

To this end, 3 most essential functions are devised, they are:
`stDataMap`, `stDataUnmap`, and `stDataTrunc`. 

`stDataMap` takes an offset and a length, check it against the currently 
allocated buffer length. If the offset and the length are within range,
it returns a pointer to the initial address of the buffer plus the offset,
and add 1 to an internal count. Caller is responsible for not accessing 
beyond the range it had requested of.

`stDataUnmap` decreases the internal count mentioned above. This internal
count is used to make sure when resizing the buffer, no one accidentally access
the old invalid buffer address should the underlaying `realloc` call actually
relocate the buffer somewhere else. To this end, this function does not need
to know the exact ranges mapped, only a total is required.

`stDataTrunc` takes a length parameter, checks no one is currently mapping
any range of the buffer, and resize it.

`stDict_t`
----------

The dictionary type `stDict_t` is keyed by `stData_t`, as a 
pridefully shameless practice of dogfooding. The most essential functions 
of this type are `stDictGet`, `stDictSet`, and `stDictUnset`.

`stDictGet` places the requested member in a pointer to `stObj_t`, and returns
one of the `st_access_*` value described in: 
"General Usage >> Accessing Container"
The obtained member is unretained - caller has to explicitly call `stObjRetain`
on it, as SafeTypes assume the acquisition is temporary.

`stDictSet` places (or replaces if there's an existing value) into the 
dictionary under they specified key, using the specified `st_setter_*` 
semantic.

`stDictUnset` unsets (a.k.a. deletes) the value at the specified key. Like
many existing language type systems, this differs from setting the key to null
in that, unsetting the slot causes the getter to return indication that the
slot has no value, where as setting it to null indicates that the slot has a
value.

`stList_t`
----------

This is the *ordered* sequence type. As there's no silver bullet for a type
to support both fast arbitrary indexed access, and fast arbitrary slicing,
the direction of design decision lean towards more primitive operations that
can readily compose into useful sequential operations.

To this end, 4 functions are provided to insert and remove items from the list
(with 1 being dis-recommended), and 2 providing helper functionalities.

The list has a cursor, which is a non-negative integer value no greater than 
the length of the list. The list contains a pointer to the element at the 
position indicated by the cursor.

The `stListInsert` and `stListPush` functions insert elements at the current
cursor position. `stListInsert` does not advance the cursor position, where as
`stListPush` does.

The `stListShift` function removes an element from the current cursor position
without changing it, and place the removed element in a pointer to `stObj_t`.
Because the element is no longer in the list, reference count is increased
(while the kept count is simultaneously decreased).

The `stListPop` function retreats the cursor position by 1 and remove the
element at the then cursor position and place it in a pointer to `stObj_t`.
As noted in the comments for this function in the "stList.h" header,
the function is implemented in terms of `stListShift`, and is inefficient and
redundant.

`stListSeek` alters the current cursor position:
- when `whence` is `ST_LIST_SEEK_SET`, it's set to 0+offset,
- when `whence` is `ST_LIST_SEEK_END`, it's set to [length]+offset,
- when `whence` is `ST_LIST_SEEK_CUR`, it's set to [cursor]+offset.

`stListSort` is a primitive implementation of sorting over the list.
Because theoretically, all sorting algorithms require O(n^2) time+memory
resource, insertion sort is used for its
- memory-space efficiency,
- implementation simplilcity.

Other Info
==========

The SafeTypes library is free and open-source software place in the 
Public Domain by DannyNiu/NJF. Refer to header files for function prototypes,
and source code for detailed implementation information
