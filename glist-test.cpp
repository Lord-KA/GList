#include "glist.h"

int main()
{
    gList list;

    gObjPool_status pool_s = gObjPool_status_BadStructPtr;
    gList_status list_s = (gList_status)pool_s;

    fprintf(stderr, "pool_s = %d\nlist_s = %d\n", pool_s, list_s);

    gList_ctor(&list, NULL);

    gList_insertByPos(&list, 0, 200);
    fprintf(stderr, "zero = %lu\n", list.zero);
    
    gList_insertByPos(&list, 0, 100);
    // goto finish;

    //gList_insertByPos(&list, 2, 700);
    
    gList_insertByPos(&list, 2, 600);
    
    gList_insertByPos(&list, 2, 500);
    gList_insertByPos(&list, 2, 400);
    gList_insertByPos(&list, 2, 300);
    gList_dump(&list);

    FILE *out = fopen("dumpBefore.gv", "w");
    gList_dumpPoolGraphViz(&list, out);
    fclose(out);


    gList_posSort(&list);
    gList_dump(&list);
    /*
    int val = 0;
    gList_popByPos(&list, 2, &val);
    gList_dump(&list);

    gList_popByPos(&list, 2, &val);
    gList_dump(&list);
    */

    out = fopen("dumpAfter.gv", "w");
    gList_dumpPoolGraphViz(&list, out);
    fclose(out);

finish:
    gObjPool_dumpFree(&list.pool, list.logStream);
    gList_dtor(&list);
}
