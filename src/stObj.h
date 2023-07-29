/* DannyNiu/NJF, 2023-07-13. Public Domain. */

#ifndef SafeTypes_Obj_H
#define SafeTypes_Obj_H 1

#include "common.h"

typedef struct stObj stObj_t;

// - An iterator function yields its children in a certain arbitrary order.
// - Currently, no return value from this function has been assigned any
//   meaning and it should return 0 as default.
// - This function can be reset to beginning by setting ``iterp'' to NULL.
// - When the object has a NULL ``iterp'' after a call to the function,
//   then all (direct) children had been enumerated.
// - If an object have a NULL ``iterf'', then it has no children.
// - A separate traversal function descends recursively, and is used in
//   reference loop resolution.
typedef int (*stIterFunc_t)(stObj_t *obj);

typedef int (*stTraversalCallback_t)(
    void *ctx, stObj_t *current, stObj_t *parent);

// When both the reference and the kept count of an object is zero, its
// finalization routine is invoked and object is freed.
typedef void (*stFinalFunc_t)(stObj_t *obj);

// Rules of composition:
// - 0x3000: container/plain/opaque type class flag mask.
// - 0x0000: plain type.
// - 0x1000: container type.
// - 0x2000: opaque (application-defined) type.
// - 0x0f00: plain type class mask.
// - 0x0000: string/blob/misc.
// - 0x0100: integer type.
// - 0x0200: floating point type.
// - 0x00c0: endianness mask - not applicable to strings and blobs.
// - 0x0000: host endianness.
// - 0x0040: little-endian.
// - 0x0080: big-endian.
// - 0x003f: plain type width mask.
// - 0x0000: exact value for the null type.
// - 0x0001: exact value for the raw data type.
// - 0x0002: exact value for utf-8 string type.
// - 0x0003: exact value for 8-bit data type.
#define ST_OBJ_TYPE_NULL        0x0000

// No linter enforces the correctness of coding, they're only different in
// serialization to JSON:
// - *_BLOB is base-64 encoded,
// - *_UTF8 is converted to UTF-16, then with
//   Unicode characters converted to code point literal.
// - *_8BIT is encoded with non-printable characters
//   encoded in hex char literal.
#define ST_OBJ_TYPE_BLOB        0x0001
#define ST_OBJ_TYPE_UTF8        0x0002
#define ST_OBJ_TYPE_8BIT        0x0003

// width is a power of 2 between 1 and 8.
#define ST_OBJ_TYPE_INT(width)  (0x0100 | (width))

// width is a power of 2 between 2 and 16.
// values other than 4 and 8 may not necessarily be supported.
#define ST_OBJ_TYPE_FLT(width)  (0x0200 | (width))

#define ST_OBJ_TYPE_DICT        0x1001
#define ST_OBJ_TYPE_LIST        0x1002

struct stObj {
    unsigned type;

    // used by garbage collector.
    int mark;

    // used for debugging.
    int dbg_i;
    int dbg_c;
    
    // referenced by a lexical variable.
    size_t refcnt;
    
    // referenced by a container object.
    // used for reference loop resolution, which is an opt-in feature.
    size_t keptcnt;

    stFinalFunc_t finalf;

    stObj_t *iterp;
    union {
        void *p;
        uintptr_t i;
    } iterk;
    stIterFunc_t iterf;
};

// Invoked by subclasses (i.e. specific types inheriting the ``stObj_t''
// header in their data structure). Does some basic setup.
stObj_t *stObjInit(stObj_t *obj);

// - Traverse the container graph rooted in ``obj''
//   and mark every object with ``markval'', and
//   skipping every object marked -1.
// - If an object is found to be marked with ``marked'',
//   then it's not descended into.
// - ``cb'' is called on every object encountered.
//   Including those already encountered and marked,
//   but only once everytime they're encountered. 
int stObjTraverse(
    stObj_t *obj, stObj_t *parent, int markval,
    stTraversalCallback_t cb, void *cbctx);

stObj_t *stObjRetain(stObj_t *obj); // ``++refcnt''.
stObj_t *stObjKeep(stObj_t *obj); // ``++keptcnt''.

void stObjRelease(stObj_t *obj); // ``--refcnt''.
void stObjLeave(stObj_t *obj); // ``--keptcnt''.

// This function simply invokes ``finalf''.
// Therefore implementations shouldn't
// call this super function.
void stObjFinal(stObj_t *obj);

#endif /* SafeTypes_Obj_H */
