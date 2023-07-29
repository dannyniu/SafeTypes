/* DannyNiu/NJF, 2023-03-02. Public Domain. */

#include "stDict.h"

#include <stdio.h>

int main(void)
{
    int i, ret;
    int fails = 0;
    
    char key_raw[16];
    stData_t key = {0};
    
    stDict_t x = {0};
    stData_t *v;

    stDictInit(&x);
    
    for(i=0; i<512; i++)
    {
        snprintf(key_raw, sizeof(key_raw), "%d", i);
        key.len = strlen(key_raw);
        key.buf = key_raw;
        
        v = calloc(1, sizeof(stData_t));
        stDataInit(v, sizeof(long));
        *(long *)v->buf = i;
        stDictSet(&x, &key, (void *)v, st_setter_gave);
    }
    
    for(i=0; i<512; i++)
    {
        snprintf(key_raw, sizeof(key_raw), "%d", i);
        key.len = strlen(key_raw);
        key.buf = key_raw;
        
        ret = stDictGet(&x, &key, (void *)&v);
        
        if( ret != st_access_success )
        {
            fails ++;
            printf("Unable to get: %s, ret: %d.\n", key_raw, ret);
        }

        if( (ret = (int)*(long *)v->buf) != i )
        {
            fails ++;
            printf("Wrong value: expected: %d, had %d.\n", i, ret);
        }
    }

    for(i=0; i<512; i++)
    {
        snprintf(key_raw, sizeof(key_raw), "!%d", i);
        key.len = strlen(key_raw);
        key.buf = key_raw;
        
        ret = stDictGet(&x, &key, (void *)&v);

        if( ret != st_access_nullval )
        {
            fails ++;
            printf("Shouldn't have been set for key: %s\n", key_raw);
        }
    }

    x.hdrObj.iterp=NULL;
    stDictIterate(&x);
    while( x.hdrObj.iterp )
    {
        v = (stData_t *)x.hdrObj.iterp;
        printf("Value from iterator: %d.\n", (int)*(long *)v->buf);
        stDictIterate(&x);
    }

    for(i=256; i<512; i++)
    {
        snprintf(key_raw, sizeof(key_raw), "%d", i);
        key.len = strlen(key_raw);
        key.buf = key_raw;
        
        ret = stDictUnset(&x, &key);
        if( ret != st_access_success )
        {
            fails ++;
            printf("Failed to unset for key: %s\n", key_raw);
        }

        ret = stDictUnset(&x, &key);
        if( ret != st_access_nullval )
        {
            fails ++;
            printf("Failed to verify unset for key: %s\n", key_raw);
        }

        ret = stDictGet(&x, &key, (void *)&v);
        if( ret != st_access_nullval )
        {
            fails ++;
            printf("Should've been unset for key: %s\n", key_raw);
        }
    }

    if( fails ) return EXIT_FAILURE; else return EXIT_SUCCESS;
}
