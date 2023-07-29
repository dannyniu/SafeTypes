/* DannyNiu/NJF, 2023-07-14. Public Domain. */

#include "stObj.c"

typedef struct stObjTrav stObjTrav_t;

#define ST_OBJ_TRAV_CHLD_CNT 8

struct stObjTrav {
    stObj_t hdrObj;
    int iteri;
    int val;
    stObjTrav_t *chld[ST_OBJ_TRAV_CHLD_CNT];
};

int stObjTravIter(stObjTrav_t *obj)
{
    if( !obj->hdrObj.iterp )
        obj->iteri = 0;
    
    for(;;)
    {
        int i = obj->iteri++;
        if( i >= ST_OBJ_TRAV_CHLD_CNT )
        {
            obj->hdrObj.iterp = NULL;
            obj->iteri = 0;
            return 0;
        }
        else if( obj->chld[i] )
        {
            obj->hdrObj.iterp = (stObj_t *)obj->chld[i];
            return 0;
        }
    }
}

#define TRAV_SET_SIZE 6

static stObjTrav_t travset[TRAV_SET_SIZE];

void travset_init()
{
    int i;

    for(i=0; i<TRAV_SET_SIZE; i++) travset[i].val = i + 'A';
    
    travset[0].chld[2] = travset + 1;
    travset[0].hdrObj.keptcnt = 2;

    travset[1].chld[3] = travset + 2;
    travset[1].chld[6] = travset + 3;
    travset[1].hdrObj.keptcnt = 1;

    travset[2].chld[0] = travset + 0;
    travset[2].chld[1] = travset + 3;
    travset[2].chld[5] = travset + 4;
    travset[2].hdrObj.keptcnt = 1;
    
    travset[3].chld[4] = travset + 4;
    travset[3].hdrObj.keptcnt = 2;

    travset[4].chld[7] = travset + 5;
    travset[4].hdrObj.keptcnt = 2;
    
    travset[5].chld[0] = travset + 0;
    travset[5].hdrObj.keptcnt = 1;
    
    for(i=0; i<TRAV_SET_SIZE; i++)
        travset[i].hdrObj.iterf = (stIterFunc_t)stObjTravIter;
}

int trav_callback1(void *ctx, stObjTrav_t *current, stObjTrav_t *parent)
{
    (void)ctx;
    int p = parent ? parent->val : '-';
    int c = current ? current->val : '-';
    long pd = parent - travset;
    long cd = current - travset;
    printf("%ld %c(%03o) --> %ld %c(%03o)\n", pd, p, p, cd, c, c);
    return 0;
}

int trav_callback2(void *ctx, stObjTrav_t *current, stObjTrav_t *parent)
{
    if( !parent ) *(int *)ctx = current->hdrObj.keptcnt;
    else if( current == travset + 0 ) -- *(int *)ctx;
    return 0;
}

int main(int argc, char *argv[])
{
    int mark = argc > 1 ? atoi(argv[1]) : 12;
    int i, s;
    int ret = EXIT_SUCCESS;

    travset_init();
    
    stObjTraverse(
        (stObj_t *)travset, NULL, mark,
        (stTraversalCallback_t)trav_callback2, &s);
    
    for(i=0; i<TRAV_SET_SIZE; i++)
    {
        printf("travset[%d].hdrObj.mark == %d\n", i, travset[i].hdrObj.mark);
        if( travset[i].hdrObj.mark != mark ) ret = EXIT_FAILURE;
    }

    printf("keptcnt = %d\n", s);
    if( s != 0 ) ret = EXIT_FAILURE;
    
    return ret;
}
