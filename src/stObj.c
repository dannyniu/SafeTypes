/* DannyNiu/NJF, 2023-07-13. Public Domain. */

#include "stObj.h"

stObj_t *stObjInit(stObj_t *obj)
{
    obj->type = ST_OBJ_TYPE_NULL; // to be set by caller afterwards.
    obj->mark = 0;

    obj->dbg_c = (uint64_t)obj >> 32;
    obj->dbg_i = (uint64_t)obj;
    
    obj->refcnt = 1;
    obj->keptcnt = 0;
    obj->finalf = NULL; // to be set by caller later.
    obj->iterp = NULL;
    obj->iterf = NULL; // may be set by caller later.
    return obj;
}

int stObjTraverse(
    stObj_t *obj, stObj_t *parent, int markval,
    stTraversalCallback_t cb, void *cbctx)
{
    if( obj->mark == -1 )
        goto end;
    
    if( cb ) cb(cbctx, obj, parent);
    
    if( obj->mark == markval )
        goto end;

    obj->mark = markval;

    if( !obj->iterf )
        goto end;

    obj->iterp = NULL;
    obj->iterf(obj);
    while( obj->iterp )
    {
        stObjTraverse(obj->iterp, obj, markval, cb, cbctx);
        obj->iterf(obj);
    }
    
end:
    return 0;
}

stObj_t *stObjRetain(stObj_t *obj)
{
    ++ obj->refcnt;
    return obj;
}
stObj_t *stObjKeep(stObj_t *obj)
{
    ++ obj->keptcnt;
    return obj;
}

// TODO: implement traversal free.

typedef struct {
    size_t unleft;
    stObj_t *root;
} stObjGC_TraverseCtx_t;

static int stObjGC_Traverse(
    stObjGC_TraverseCtx_t *ctx,
    stObj_t *current, stObj_t *parent)
{
    if( parent && current == ctx->root )
        -- ctx->unleft;
    
    return 0;
}

static bool stObjCanLeave(stObj_t *obj)
{
    int ret;
    stObjGC_TraverseCtx_t wx = {
        .unleft = obj->keptcnt,
        .root = obj,
    };

    stObjTraverse(obj, NULL, 1, (stTraversalCallback_t)stObjGC_Traverse, &wx);

    assert( wx.unleft <= obj->keptcnt );
    ret = wx.unleft == 0;

    stObjTraverse(obj, NULL, 0, NULL, NULL);
    return ret;
}

static void stObj_MayDoRelease(stObj_t *obj)
{
    if( obj->refcnt ) return;

    if( obj->mark != -1 && (!obj->keptcnt || stObjCanLeave(obj)) )
    {
        obj->mark = -1;
        obj->finalf(obj);
        free(obj);
    }
}

void stObjRelease(stObj_t *obj)
{
    -- obj->refcnt;
    stObj_MayDoRelease(obj);
}

void stObjLeave(stObj_t *obj)
{
    -- obj->keptcnt;
    stObj_MayDoRelease(obj);
}

void stObjFinal(stObj_t *obj)
{
    obj->finalf(obj);
}
