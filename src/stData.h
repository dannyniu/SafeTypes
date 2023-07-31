/* DannyNiu/NJF, 2023-06-30. Public Domain. */

#ifndef SafeTypes_Data_H
#define SafeTypes_Data_H 1

#include "stObj.h"

typedef struct stData {
    stObj_t hdrObj;
    size_t len;
    void *buf;
    long mapcnt;
} stData_t;

stData_t *stDataInit(stData_t *ctx, size_t len);
size_t stDataLen(stData_t const *ctx);

// ``stDataMap'' does a simple range check - if it passes,
// a pointer at appropriate offset is returned, otherwise,
// a NULL pointer is returned.
// ``stDataMap'' doesn't record the regions requested - it only
// keeps a number, which is checked by ``stDataTrunc''.
void *stDataMap(stData_t *ctx, size_t offset, size_t len);

// ``stDataUnmap'' decreases what's increased by ``stDataMap''.
// The numbers of ``stDataMap'' and ``stDataUnmap'' should match.
int stDataUnmap(stData_t *ctx);

// If there's someone mapping the data (i.e. more ``stDataMap''
// than ``stDataUnmap''), then this function returns -1.
// Otherwise, the internal buffer is resized. Contrary to what
// the name suggests, len can be greater than 'len', and this is
// in fact a resize operation (it invokes ``realloc'').
int stDataTrunc(stData_t *ctx, size_t len);

void stDataFinal(stData_t *ctx);

#endif /* SafeTypes_Data_H */
