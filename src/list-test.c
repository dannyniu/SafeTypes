/* DannyNiu/NJF, 2023-03-02. Public Domain. */

#include "stList.h"

#include <stdio.h>

int main(void)
{
    int i; //, ret;
    int fails = 0;
        
    stList_t x = {0};
    stData_t *v;

    stListInit(&x);

    if( stListSeek(&x, 0, ST_LIST_SEEK_SET) == -1 )
        printf("SET Seek on Empty List Failed!\n"), fails ++;

    if( stListSeek(&x, 0, ST_LIST_SEEK_END) == -1 )
        printf("END Seek on Empty List Failed!\n"), fails ++;
    
    if( stListSeek(&x, 0, ST_LIST_SEEK_CUR) == -1 )
        printf("CUR Seek on Empty List Failed!\n"), fails ++;

    for(i=4; i<=8; i+=2)
    {
        v = calloc(1, sizeof(stData_t));
        stDataInit(v, sizeof(long));
        *(long *)v->buf = i;

        if( stListPush(&x, (void *)v, st_setter_gave) != st_access_success )
            printf("List Push Failed!\n"), fails ++;
    }

    for(i=7; i>=5; i-=2)
    {
        v = calloc(1, sizeof(stData_t));
        stDataInit(v, sizeof(long));
        *(long *)v->buf = i;

        if( stListSeek(&x, -1, ST_LIST_SEEK_CUR) == -1 )
            printf("CUR Seek on Live List Failed!\n"), fails ++;
        
        if( stListInsert(&x, (void *)v, st_setter_gave) != st_access_success )
            printf("List Insert Failed!\n"), fails ++;
    }

    if( stListSeek(&x, 0, ST_LIST_SEEK_SET) == -1 )
        printf("SET Seek on Live List Failed!\n"), fails ++;

    for(i=3; i>=0; i--)
    {
        v = calloc(1, sizeof(stData_t));
        stDataInit(v, sizeof(long));
        *(long *)v->buf = i;

        if( stListInsert(&x, (void *)v, st_setter_gave) != st_access_success )
            printf("List Insert Failed!\n"), fails ++;
    }

    if( stListSeek(&x, 0, ST_LIST_SEEK_END) == -1 )
        printf("END Seek on Live List Failed!\n"), fails ++;

    for(i=9; i<=10; i++)
    {
        v = calloc(1, sizeof(stData_t));
        stDataInit(v, sizeof(long));
        *(long *)v->buf = i;

        if( stListPush(&x, (void *)v, st_setter_gave) != st_access_success )
            printf("List Push Failed!\n"), fails ++;
    }

    x.hdrObj.iterp = NULL;
    for(stListIterate(&x), i=0; x.hdrObj.iterp; stListIterate(&x), i++)
    {
        v = (void *)x.hdrObj.iterp;
        if( *(long *)v->buf != i )
        {
            printf("List Enumeration Encountered Wrong Value: %ld != %d.\n",
                   *(long *)v->buf, i), fails ++;
        }
        if( x.hdrObj.iterk.i != i )
        {
            printf("List Enumeration Encountered Wrong Index: %ld != %d.\n",
                   x.hdrObj.iterk.i, i), fails ++;
        }
    }

    stListSeek(&x, 0, ST_LIST_SEEK_SET);
    for(i=0; i<3; i++)
    {
        if( stListShift(&x, (void *)&v) != st_access_success )
            printf("List Shift Failed!\n"), fails ++;

        if( *(long *)v->buf != i )
        {
            printf("List Shift Encountered Wrong Value: %ld != %d.\n",
                   *(long *)v->buf, i), fails ++;
        }

        stObjRelease((void *)v);
    }
   
    stListSeek(&x, 0, ST_LIST_SEEK_END);
    for(i=10; i>7; i--)
    {
        if( stListPop(&x, (void *)&v) != st_access_success )
            printf("List Pop Failed!\n"), fails ++;

        if( *(long *)v->buf != i )
        {
            printf("List Pop Encountered Wrong Value: %ld != %d.\n",
                   *(long *)v->buf, i), fails ++;
        }
        
        stObjRelease((void *)v);
    }

    stListSeek(&x, -1, ST_LIST_SEEK_END);
    
    if( stListPop(&x, (void *)&v) != st_access_success )
        printf("List Pop Failed!\n"), fails ++;
    if( *(long *)v->buf != 6 )
    {
        printf("List Pop Encountered Wrong Value: %ld != %d.\n",
               *(long *)v->buf, i), fails ++;
    }

    if( stListShift(&x, (void *)&v) != st_access_success )
        printf("List Shift Failed!\n"), fails ++;
    if( *(long *)v->buf != 7 )
    {
        printf("List Shift Encountered Wrong Value: %ld != %d.\n",
               *(long *)v->buf, i), fails ++;
    }
   
    if( fails ) return EXIT_FAILURE; else return EXIT_SUCCESS;
}
