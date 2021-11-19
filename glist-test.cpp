#define GLIST_TYPE int
#define GLIST_PRINTF_CODE "%d"

#include "glist.h"
#include "gtest/gtest.h"

#include <vector>
#include <random>
#include <iostream>

std::mt19937 rnd(179);


TEST(manual, basic)
{
    gList list;

    gObjPool_status pool_s = gObjPool_status_BadStructPtr;
    gList_status list_s = (gList_status)pool_s;

    // fprintf(stderr, "pool_s = %d\nlist_s = %d\n", pool_s, list_s);

    gList_ctor(&list, NULL);

    gList_insertByPos(&list, 0, 200);
    // fprintf(stderr, "zero = %lu\n", list.zero);
    
    gList_insertByPos(&list, 0, 100);
    // goto finish;

    //gList_insertByPos(&list, 2, 700);
    
    gList_insertByPos(&list, 2, 600);
    
    gList_insertByPos(&list, 2, 500);
    gList_insertByPos(&list, 2, 400);
    gList_insertByPos(&list, 2, 300);
    // gList_dump(&list);

    FILE *out = fopen("dumpBefore.gv", "w");
    gList_dumpPoolGraphViz(&list, out);
    fclose(out);


    gList_posSort(&list);
    
    int val = 0;
    gList_popByPos(&list, 2, &val);
    EXPECT_EQ(val, 200);

    gList_popByPos(&list, 4, &val);
    EXPECT_EQ(val, 500);

    out = fopen("dumpAfter.gv", "w");
    gList_dumpPoolGraphViz(&list, out);
    fclose(out);

finish:
    // gObjPool_dumpFree(&list.pool, list.logStream);
    gList_dtor(&list);
}


TEST(Auto, basic) 
{
    gList listStruct = {};
    gList *list = &listStruct;
    gList_ctor(list, NULL);
    std::vector<GLIST_TYPE> stdList = {};

    for (size_t i = 0; i < 30000; ++i) {
        if (rnd() % 5 != 1) {
            size_t pos = 0;
            if (list->size)
                pos = (rnd() % 1000 + 1) % list->size;
                
            int val = rnd();
            stdList.insert(stdList.begin() + pos, val);
            gList_insertByPos(list, pos, val);
        } else {
            size_t pos = 0;
            if (list->size)
                pos = (rnd() % 1000 + 1) % list->size;
            int val_1 = 0;
            int val_2 = stdList[pos];
            stdList.erase(stdList.begin() + pos);
            gList_popByPos(list, pos + 1, &val_1);
            EXPECT_EQ(val_1, val_2);
        }
    }
    while (list->size) {
        size_t pos = 0;
        if (list->size)
            pos = (rnd() % 1000 + 1) % list->size;
        int val_1 = 0;
        int val_2 = stdList[pos];
        stdList.erase(stdList.begin() + pos);
        gList_popByPos(list, pos + 1, &val_1);
        EXPECT_EQ(val_1, val_2);
    }
    gList_dtor(list);
}
