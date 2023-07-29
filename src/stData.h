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
void *stDataMap(stData_t *ctx, size_t offset, size_t len);
int stDataUnmap(stData_t *ctx);
int stDataTrunc(stData_t *ctx, size_t len);
void stDataFinal(stData_t *ctx);

#endif /* SafeTypes_Data_H */
