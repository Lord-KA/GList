#include "glist.h"

int main()
{
    gList list;

    gObjPool_status pool_s = gObjPool_status_BadStructPtr;
    gList_status list_s = (gList_status)pool_s;

    fprintf(stderr, "pool_s = %d\nlist_s = %d\n", pool_s, list_s);

    gList_ctor(&list, NULL);

    gList_dump(&list);
    gObjPool_dumpFree(&list.pool, list.logStream);

    gList_insertByPos(&list, 0, 12);
    fprintf(stderr, "zero = %lu\n", list.zero);
    gList_dump(&list);
    
    gList_insertByPos(&list, 0, 17);
    gList_dump(&list);
    // goto finish;

    gList_insertByPos(&list, 2, 179);
    gList_dump(&list);

    
    gList_insertByPos(&list, 2, 1990);
    gList_dump(&list);


    int val = 0;
    gList_popByPos(&list, 2, &val);
    gList_dump(&list);

    gList_popByPos(&list, 2, &val);
    gList_dump(&list);

    FILE *out = fopen("dump.gv", "w");
    gList_dumpGraphViz(&list, out);
    fclose(out);

finish:
    gObjPool_dumpFree(&list.pool, list.logStream);
    gList_dtor(&list);
}
