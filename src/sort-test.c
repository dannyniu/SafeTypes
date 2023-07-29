/* DannyNiu/NJF, 2023-07-28. Public Domain. */

#include "stList.h"
#include <time.h>

typedef struct {
    stObj_t hdrObj;
    int v;
} stVal_t;

#define VALS_LEN 120
#define VAL_MOD 97
stVal_t vals[VALS_LEN];

stList_t list;

int stValCmp(stVal_t *a, stVal_t *b)
{
    return a->v - b->v;
}

int main()
{
    int i, l, s;
    
    srand(time(NULL));

    for(l=0; l<VALS_LEN; l+=l/10+1)
    {
        stListInit(&list);
        for(i=0; i<l; i++)
        {
            stObjInit(&vals[i].hdrObj);
            vals[i].v = rand() % VAL_MOD;
            stListPush(&list, &vals[i].hdrObj, st_setter_kept);
        }

        stListSort(&list, (stListSort_CmpFunc_t)stValCmp);

        list.hdrObj.iterp = NULL;
        stListIterate(&list);

        assert( !l || list.hdrObj.iterp );
        
        if( list.hdrObj.iterp )
            s = ((stVal_t *)list.hdrObj.iterp)->v;
        
        for(i=0; i<l; i++)
        {
            if( s > ((stVal_t *)list.hdrObj.iterp)->v )
                exit(EXIT_FAILURE);
            stListIterate(&list);
        }

        stListFinal(&list);
    }

    return EXIT_SUCCESS;
}
