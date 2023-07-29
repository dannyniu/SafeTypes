/* DannyNiu/NJF, 2023-07-28. Public Domain. */

#define INTERCEPT_MEM_CALLS 1

#include "mem-intercept.c"
#include "siphash.c"
#include "stObj.c"
#include "stData.c"
#include "stDict.c"
#include "stList.c"

int main()
{
    stList_t *root = calloc(1, sizeof(stList_t));
    stDict_t *frame;
    stDict_t *view;
    stData_t *toc = calloc(1, sizeof(stData_t));

    stData_t k_toc, k_content, k_parent, k_root;

    int ret = EXIT_SUCCESS;
    int i;

    stListInit(root);
    root->hdrObj.dbg_c = 'L';
    root->hdrObj.dbg_i = 'R';
        
    stDataInit(toc, 4);
    *(int *)stDataMap(toc, 0, 0) = 12;
    stDataUnmap(toc);
    toc->hdrObj.dbg_c = 't';
    toc->hdrObj.dbg_i = 65535;
    
    stDataInit(&k_toc, 3);
    stDataInit(&k_content, 3);
    stDataInit(&k_parent, 3);
    stDataInit(&k_root, 3);

    k_toc.hdrObj.dbg_c = 'k';
    k_content.hdrObj.dbg_c = 'k';
    k_parent.hdrObj.dbg_c = 'k';
    k_root.hdrObj.dbg_c = 'k';

    memcpy(stDataMap(&k_toc, 0, 0), "toc", 3);
    memcpy(stDataMap(&k_content, 0, 0), "con", 3);
    memcpy(stDataMap(&k_parent, 0, 0), "par", 3);
    memcpy(stDataMap(&k_root, 0, 0), "roo", 3);

    stDataUnmap(&k_toc);
    stDataUnmap(&k_content);
    stDataUnmap(&k_parent);
    stDataUnmap(&k_root);

    for(i=0; i<8; i++)
    {
        frame = calloc(1, sizeof(stDict_t));
        view = calloc(1, sizeof(stDict_t));
        stDictInit(frame);
        stDictInit(view);
        frame->hdrObj.dbg_c = 'f';
        frame->hdrObj.dbg_i = i;
        view->hdrObj.dbg_c = 'v';
        view->hdrObj.dbg_i = i;
        
        stDictSet(view,  &k_parent,  &frame->hdrObj, st_setter_kept);
        stDictSet(view,  &k_root,    &root ->hdrObj, st_setter_kept);
        stDictSet(frame, &k_toc,     &toc  ->hdrObj, st_setter_kept);
        stDictSet(frame, &k_content, &view ->hdrObj, st_setter_fast);

        stListPush(root, &frame->hdrObj, st_setter_fast);
    }
    printf("toc: %p, root: %p.\n", toc, root);

    stObjRelease(&toc->hdrObj);
    stListPush(root, &root->hdrObj, st_setter_gave);

    stDataFinal(&k_toc);
    stDataFinal(&k_content);
    stDataFinal(&k_parent);
    stDataFinal(&k_root);

    for(i=0; i<4; i++)
    {
        if( mh[i] ) ret = EXIT_FAILURE;
        printf("%08lx%c", (long)mh[i], i==4 ? '\n' : ' ');
    }

    printf("allocs: %d, frees: %d.\n", allocs, frees);

    return ret;
}
