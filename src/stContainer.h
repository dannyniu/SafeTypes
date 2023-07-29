/* DannyNiu/NJF, 2023-07-16. Public Domain. */

#ifndef SafeTypes_Container_H
#define SafeTypes_Container_H 1

enum st_access_retvals {
    st_access_error = -1,
    st_access_nullval = 0,
    st_access_success = 1,
};

enum st_setter_semantics {
    st_setter_kept = 1, // ``++ keptcnt''.
    st_setter_gave = 2, // ``-- refcnt, ++keptcnt''.
    st_setter_fast = 3, // same as '*_gave', except no GC.
};

#endif /* SafeTypes_Container_H */
