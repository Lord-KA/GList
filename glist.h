#ifndef GLIST_H
#define GLIST_H

#include "stdio.h"

typedef int GLIST_TYPE;              //TODO remove it

struct gList_Node {
    GLIST_TYPE data;
    size_t next;
    size_t prev;
} typedef gList_Node;

typedef gList_Node GOBJPOOL_TYPE;
#include "gobjpool.h"

struct gList {
    size_t size;
    size_t zero;
    gObjPool pool;
    FILE *logStream;
} typedef gList;

enum gList_status {
    gList_status_OK,
    gList_status_AllocErr,
    gList_status_BadCapacity, 
    gList_status_BadStructPtr,
    gList_status_BadId,
};

#define CHECK_POOL_STATUS(status) ({      \
    if ((status) != gObjPool_status_OK)      \
        return (gList_status)(status);        \
})              //TODO

gList_status gList_ctor(gList *list, FILE *newLogStream) {
    if (!gPtrValid(list)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream))                                
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to list ctor!\n");
        return gList_status_BadStructPtr;                         
    }

    if (!gPtrValid(newLogStream))
        list->logStream = stderr;
    else 
        list->logStream = newLogStream;

    list->size = 0;
    gObjPool_status status = gObjPool_ctor(&list->pool, 1, newLogStream);
    CHECK_POOL_STATUS(status);
    status = gObjPool_alloc(&list->pool, &list->zero);
    CHECK_POOL_STATUS(status);
    gList_Node *node;
    status = gObjPool_get(&list->pool, list->zero, &node);
    CHECK_POOL_STATUS(status);
    node->next = -1;
    node->prev = -1;
    //node->data = {};      //TODO?
    return gList_status_OK;
}

gList_status gList_dtor(gList *list) {
    list->size = 0; 
    gList_Node *node = NULL;
    gObjPool_get(&list->pool, list->zero, &node);
    if (node != NULL) {
        node->next = -1;
        node->prev = -1;
    }
    gObjPool_dtor(&list->pool);
    return gList_status_OK;
}

#endif /* GLIST_H */
