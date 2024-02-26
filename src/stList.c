/* DannyNiu/NJF, 2023-07-17. Public Domain. */

#include "stList.h"

stList_t *stListInit(stList_t *list)
{
    if( !stObjInit(&list->hdrObj) ) return NULL;
    
    list->hdrObj.type = ST_OBJ_TYPE_LIST;
    list->hdrObj.finalf = (stFinalFunc_t)stListFinal;
    list->hdrObj.iterf = (stIterFunc_t)stListIterate;
    
    list->len = 0;
    list->pos = 0;
    list->cursor = &list->anch_tail;

    list->anch_head.collection = list;
    list->anch_head.value = NULL;
    list->anch_head.prev = NULL;
    list->anch_head.next = &list->anch_tail;
    
    list->anch_tail.collection = list;
    list->anch_tail.value = NULL;
    list->anch_tail.prev = &list->anch_head;
    list->anch_tail.next = NULL;
    
    return list;
}

#define ST_LIST_SETTER_ASSERTIONS                                       \
    assert( (ptrdiff_t)list->pos >= 0 && list->pos <= list->len);       \
    assert( semantic == st_setter_kept ||                               \
            semantic == st_setter_gave ||                               \
            semantic == st_setter_fast );

#define ST_LIST_GETTER_ASSERTIONS                                       \
    assert( (ptrdiff_t)list->pos >= 0 && list->pos <= list->len);

int stListInsert(stList_t *list, stObj_t *obj, int semantic)
{
    struct st_list_member *M;

    ST_LIST_SETTER_ASSERTIONS;

    if( !(M = calloc(1, sizeof(struct st_list_member))) )
        return st_access_error;

    M->collection = list;
    M->value = obj;

    ++ list->len;
    
    M->next = list->cursor;
    M->prev = list->cursor->prev;
    list->cursor->prev->next = M;
    list->cursor->prev = M;
    list->cursor = M;

    switch( semantic ){
    case st_setter_kept:
        stObjKeep(obj);
        break;

    case st_setter_gave:
        stObjKeep(obj);
        stObjRelease(obj);
        break;
        
    case st_setter_fast:
        ++ obj->keptcnt;
        -- obj->refcnt;
        break;
    }

    return st_access_success;
}

int stListPush(stList_t *list, stObj_t *obj, int semantic)
{
    struct st_list_member *M;

    ST_LIST_SETTER_ASSERTIONS;

    if( !(M = calloc(1, sizeof(struct st_list_member))) )
        return st_access_error;

    M->collection = list;
    M->value = obj;

    ++ list->len;
    ++ list->pos;
    
    M->next = list->cursor;
    M->prev = list->cursor->prev;
    list->cursor->prev->next = M;
    list->cursor->prev = M;

    switch( semantic ){
    case st_setter_kept:
        stObjKeep(obj);
        break;

    case st_setter_gave:
        stObjKeep(obj);
        stObjRelease(obj);
        break;
        
    case st_setter_fast:
        ++ obj->keptcnt;
        -- obj->refcnt;
        break;
    }
    
    return st_access_success;
}

int stListShift(stList_t *list, stObj_t **out)
{
    struct st_list_member *M;
    
    ST_LIST_GETTER_ASSERTIONS;

    if( list->len <= 0 || list->pos >= list->len )
    {
        *out = NULL;
        return st_access_nullval;
    }

    M = list->cursor;
    *out = M->value;
    
    // 2023-07-27:
    // It was previously noted that, due to efficiency reasons,
    // garbage collecting calls was not used in container setters and getters.
    // Considering the possibility of edge cases where the "gave" semantic
    // may cause the undesired behavior of reference and kept counts to
    // drop to 0 without invoking the garbage collector, that note has been
    // obsoleted and removed.
    // However, when there's guarantee that reference count will be non-zero
    // after the operation, an exception is established for it for the same
    // efficiency reason and purpose.
    ++ (*out)->refcnt;
    -- (*out)->keptcnt;

    -- list->len;

    list->cursor->prev->next = list->cursor->next;
    list->cursor->next->prev = list->cursor->prev;
    list->cursor = list->cursor->next;

    free(M);
    return st_access_success;
}

int stListPop(stList_t *list, stObj_t **out)
{
    ST_LIST_GETTER_ASSERTIONS;

    if( list->len <= 0 || list->pos <= 0 || list->len < list->pos )
    {
        *out = NULL;
        return st_access_nullval;
    }

    list->pos --;
    list->cursor = list->cursor->prev;

    return stListShift(list, out);
}

extern int indent;

int stListIterate(stList_t *list)
{
    if( !list->hdrObj.iterp )
    {
        list->hdrObj.iterk.i = list->pos = 0;
        list->cursor = list->anch_head.next;
    }

    if( list->cursor != &list->anch_tail )
    {
        list->hdrObj.iterp = list->cursor->value;
        list->hdrObj.iterk.i = (list->pos ++);
        list->cursor = list->cursor->next;
    }
    else
    {
        list->hdrObj.iterp = NULL;
        list->hdrObj.iterk.i = list->pos = 0;
        list->cursor = NULL;
    }

    return st_access_success;
}

ptrdiff_t stListSeek(stList_t *list, ptrdiff_t offset, int whence)
{
    switch( whence )
    {
    case ST_LIST_SEEK_SET:
        if( offset < 0 || (size_t)offset > list->len )
        {
            return -1;
        }
        else
        {
            list->pos = offset;
            list->cursor = list->anch_head.next;
            
            while( offset > 0 )
            {
                list->cursor = list->cursor->next;
                offset --;
            }
            
            return list->pos;
        }
        break;

    case ST_LIST_SEEK_END:
        if( offset + (ptrdiff_t)list->len < 0 || offset > 0 )
        {
            return -1;
        }
        else
        {
            list->pos = list->len + offset;
            list->cursor = &list->anch_tail;
            
            while( offset < 0 )
            {
                list->cursor = list->cursor->prev;
                offset ++;
            }
            
            return list->pos;
        }
        break;

    case ST_LIST_SEEK_CUR:
        if( offset + (ptrdiff_t)list->pos < 0 ||
            offset + (ptrdiff_t)list->pos > (ptrdiff_t)list->len )
        {
            return -1;
        }
        else
        {
            list->pos += offset;
            
            while( offset > 0 )
            {
                list->cursor = list->cursor->next;
                offset --;
            }
            
            while( offset < 0 )
            {
                list->cursor = list->cursor->prev;
                offset ++;
            }
            
            return list->pos;
        }
        break;

    default:
        return -1;
    }
}

stList_t *stListSort(stList_t *list, stListSort_CmpFunc_t cmpfunc)
{
    /* A conjecture credited to DannyNiu/NJF (that's me, hi) says:
     * for a given class of problem, the asymptotic time-space
     * overall complexity of the most optimal algorithms that
     * solve the problem cannot be optimized beyond a certain
     * limit. In plain words, if you optimize the time complexity
     * of an algorithm to solve a specific problem, the space
     * complexity will grow up by as much as is optimized away in
     * the time dimension.
     *
     * This is akin to data compression - where entropy of the
     * data source dictates the minimum size of the compressed
     * file that represents the original data; and akin to
     * "Kolmogorov Complexity" - a similar concept where the
     * subject is a data-producing "functional" description.
     *
     * Therefore, after comparing the time/space complexity of
     * popular sorting algorithms, and noting that all of them
     * have worst-case complexity of at least O(n^2), a dumb
     * algorithm with O(1) space complexity is chosen for
     * implementation for SafeTypes library.
     */
     
    struct st_list_member *o, *t;
    
    if( list->len <= 1 ) return list;

    list->pos = 0;
    list->cursor = list->anch_head.next;
    o = t = NULL;
    list->anch_head.next->prev = NULL;
    list->anch_tail.prev->next = NULL;
    list->anch_head.next = &list->anch_tail;
    list->anch_tail.prev = &list->anch_head;

    while( list->cursor )
    {
        for(o=list->anch_head.next; o->next; o=o->next)
        {
            // using the ``<='' operator to preserve the relative order
            // of equal elements scattered over the list.
            // !!not a guaranteed feature!!
            if( cmpfunc(o->value, list->cursor->value) <= 0 )
                continue;
            else break;
        }

        // insert cursor head just before ``o''.

        t = list->cursor;
        list->cursor = list->cursor->next;

        t->next = o;
        t->prev = o->prev;
        o->prev->next = t;
        o->prev = t;
    }

    return list;
}

void stListFinal(stList_t *list)
{
    struct st_list_member *M, *L;

    for(M=list->anch_head.next; M!=&list->anch_tail; )
    {
        stObjLeave(M->value);
        L = M;
        M = M->next;
        free(L);
    }

    memset((uint8_t *)list + sizeof(stObj_t), 0,
           sizeof(stList_t) - sizeof(stObj_t));
    stObjFinalSuper(&list->hdrObj);
}
