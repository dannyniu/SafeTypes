/* DannyNiu/NJF, 2023-06-30. Public Domain. */

#include "stData.h"

stData_t *stDataInit(stData_t *ctx, size_t len)
{
    if( !stObjInit(&ctx->hdrObj) ) return NULL;
    
    if( !(ctx->buf = calloc(1, len)) )
    {
        return NULL;
    }

    ctx->hdrObj.type = ST_OBJ_TYPE_BLOB;
    ctx->hdrObj.finalf = (stFinalFunc_t)stDataFinal;
    ctx->len = len;
    ctx->mapcnt = 0;
    return ctx;
}

size_t stDataLen(stData_t const *ctx)
{
    return ctx->len;
}

void *stDataMap(stData_t *ctx, size_t offset, size_t len)
{
    // 2024-03-27:
    // Back-ported from SafeTypes2.
    //
    // 2024-03-26:
    // The first check *was*: ``offset >= ctx->len''.
    // This caused a bit of surprise for codes in the real world.
    if( offset       > ctx->len ) return NULL;
    if( offset + len > ctx->len ) return NULL;

    ctx->mapcnt++;
    assert( ctx->mapcnt > 0 );
    return ((uint8_t *)ctx->buf) + offset;
}

int stDataUnmap(stData_t *ctx)
{
    assert( ctx->mapcnt > 0 );
    ctx->mapcnt--;
    return 0;
}

int stDataTrunc(stData_t *ctx, size_t len)
{
    void *tmp;
    
    if( ctx->mapcnt > 0 )
    {
        errno = EBUSY;
        return -1;
    }

    if( !(tmp = realloc(ctx->buf, len)) ) return -1;

    ctx->buf = tmp;
    ctx->len = len;
    return 0;
}

void stDataFinal(stData_t *ctx)
{
    free(ctx->buf);
    memset((uint8_t *)ctx + sizeof(stObj_t), 0,
           sizeof(stData_t) - sizeof(stObj_t));
    stObjFinalSuper(&ctx->hdrObj);
}
