#ifndef GLIST_H
#define GLIST_H

#include "stdio.h"
#include "stdlib.h"

typedef int GLIST_TYPE;              //TODO remove it
#define GLIST_PRINTF_CODE "%d"

struct gList_Node 
{
    GLIST_TYPE data;
    size_t next;
    size_t prev;
    size_t id;
} typedef gList_Node;

typedef gList_Node GOBJPOOL_TYPE;

static const size_t MAX_MSG_LEN = 64;

static const char LOG_DELIM[] = "=============================";

#include "gobjpool.h"

struct gList 
{
    size_t size;
    size_t zero;
    gObjPool pool;
    FILE *logStream;
} typedef gList;

enum gList_status 
{
    gList_status_OK,
    gList_status_AllocErr,
    gList_status_BadCapacity, 
    gList_status_BadStructPtr,
    gList_status_BadId,
    gList_status_BadPos,
    gList_status_BadNodePtr,
    gList_status_BadDumpOutPtr,
    gList_status_Cnt,
};

static const char gList_statusMsg[gList_status_Cnt][MAX_MSG_LEN] = {
    "OK",
    "Allocation error",
    "Bad capacity error",
    "Bad structure pointer provided",
    "Bad id provided",
    "Bad position requested",
    "Bad node pointer provided",
    "Bad FILE pointer provided to graphViz dump",
};

#define CHECK_POOL_STATUS(status) ({        \
    if ((status) != gObjPool_status_OK)      \
        return (gList_status)(status);        \
})

#ifndef NDEBUG
#define ASSERT_LOG(expr, errCode, logStream) ({                                   \
    if (!(expr)) {                                                                 \
        fprintf((logStream),  "%s in %s!\n", gList_statusMsg[(errCode)], __func__); \
        return (gList_status)(errCode);                                              \
    }                                                                                 \
})
#else
#define ASSERT_LOG(expr, errCode, logStream) 
#endif

gList_status gList_ctor(gList *list, FILE *newLogStream) 
{
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
    gObjPool_status status = gObjPool_ctor(&list->pool, -1, newLogStream);
    CHECK_POOL_STATUS(status);
    status = gObjPool_alloc(&list->pool, &list->zero);
    CHECK_POOL_STATUS(status);
    gList_Node *node;
    status = gObjPool_get(&list->pool, list->zero, &node);
    CHECK_POOL_STATUS(status);
    node->next = list->zero;
    node->prev = list->zero;
    node->id = list->zero;
    return gList_status_OK;
}

gList_status gList_dtor(gList *list) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
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

gList_status gList_dump(const gList *list);
/*
int gList_NodeComp(const void *one, const void *other) 
{
    if (((gObjPool_Node*)one)->val.pos < ((gObjPool_Node*)other)->val.pos)
        return -1;
    if (((gObjPool_Node*)one)->val.pos > ((gObjPool_Node*)other)->val.pos)
        return 1;
    return 0;
}
*/

gList_status gList_posSort(gList *list)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node    = NULL;
    gList_Node *newNode = NULL;
    size_t newId = 0;
    gObjPool newPool;
    gObjPool_ctor(&newPool, list->pool.capacity, list->logStream);
    size_t i = 0;
    gList_status status = gList_status_OK;

    status = (gList_status)gObjPool_get(&list->pool, list->zero, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    do {
        status =(gList_status)gObjPool_alloc(&newPool, &newId);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        status =(gList_status)gObjPool_get(&newPool, newId, &newNode);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        newNode->id = i;
        newNode->next = i + 1;
        newNode->prev = i - 1;
        newNode->data = node->data;
        ++i;
        
        status =(gList_status)gObjPool_get(&list->pool, node->next, &node);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    } while (node->id != list->zero);

    gList_Node *zeroNode = NULL;
    
    status =(gList_status)gObjPool_get(&newPool, 0, &zeroNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    
    newNode->next = 0;
    zeroNode->prev = newNode->id;

    gObjPool_dtor(&list->pool);
    list->pool = newPool;

    gList_dump(list);

    return gList_status_OK;
}

gList_status gList_getNextId(const gList *list, const size_t id, size_t *nextId) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = NULL;
    gObjPool_status status = gObjPool_get(&list->pool, id, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    
    *nextId = node->next;

    return gList_status_OK;
}

gList_status gList_getNextNode(const gList *list, const size_t id, gList_Node **nextNode) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = NULL;
    gObjPool_status status = gObjPool_get(&list->pool, id, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    
    status = gObjPool_get(&list->pool, node->next, nextNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}

gList_status gList_getNode(const gList *list, const size_t pos, gList_Node **node) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    size_t curId = list->zero;
    gList_status status = gList_status_OK;
    for (size_t i = 0; i < pos; ++i) {                          //TODO calibrate
        status = gList_getNextId(list, curId, &curId);
        if (status == gList_status_BadId)
            ASSERT_LOG(status == gList_status_OK, gList_status_BadPos, list->logStream);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    }
    status = (gList_status)gObjPool_get(&list->pool, curId, node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}

gList_status gList_getData(const gList *list, const size_t pos, GLIST_TYPE **data) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_Node *node = NULL;
    gList_status status = gList_getNode(list, pos, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    *data = &node->data;

    return gList_status_OK;
}

gList_status gList_insertByNode(gList *list, gList_Node *node, const GLIST_TYPE data) 
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    ASSERT_LOG(gPtrValid(node), gList_status_BadNodePtr, list->logStream);
    gList_status status = gList_status_OK;

    size_t nextNodeId = node->next;
    size_t prevNodeId = node->id;

    size_t newNodeId = 0;
    status = (gList_status)gObjPool_alloc(&list->pool, &newNodeId);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
        
    gList_Node *nextNode = NULL;
    status = (gList_status)gObjPool_get(&list->pool, nextNodeId, &nextNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
 
    gList_Node *newNode = NULL;
    status = (gList_status)gObjPool_get(&list->pool, newNodeId, &newNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    
    status = (gList_status)gObjPool_get(&list->pool, prevNodeId, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    newNode->next = nextNodeId;
    newNode->prev = prevNodeId;
    newNode->id   = newNodeId;
    newNode->data = data;
    
    node->next     = newNodeId;
    nextNode->prev = newNodeId;
    
    return gList_status_OK;
}

gList_status gList_insertByPos(gList *list, const size_t pos, GLIST_TYPE data)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_status status = gList_status_OK;
    
    gList_Node *node = NULL;
    status = gList_getNode(list, pos, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    status = gList_insertByNode(list, node, data);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}

gList_status gList_popByNode(gList *list, gList_Node *node, GLIST_TYPE *data)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    ASSERT_LOG(gPtrValid(node), gList_status_BadNodePtr, list->logStream);
    gList_status status = gList_status_OK;

    gList_Node *prevNode = NULL;
    status = (gList_status)gObjPool_get(&list->pool, node->prev, &prevNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    fprintf(stderr, "node->next = %lu\n", node->next);
 
    gList_Node *nextNode = NULL;
    status = (gList_status)gObjPool_get(&list->pool, node->next, &nextNode);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    prevNode->next = nextNode->id;
    nextNode->prev = prevNode->id;

    node->next = -1;
    node->prev = -1;
    
    if (data != NULL)
        *data = node->data;

    status = (gList_status)gObjPool_free(&list->pool, node->id);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}

gList_status gList_popByPos(gList *list, const size_t pos, GLIST_TYPE *data)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_status status = gList_status_OK;
    
    gList_Node *node = NULL;
    status = gList_getNode(list, pos, &node);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    status = gList_popByNode(list, node, data);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}

gList_status gList_dump(const gList *list)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = NULL;
    gList_status status = gList_status_OK;
    fprintf(list->logStream, "%s\ngList dump:\n%s\nsize = %lu\nzero_id = %lu\ndata:", LOG_DELIM, LOG_DELIM, list->size, list->zero);
    
    status = (gList_status)gObjPool_get(&list->pool, list->zero, &node);
    fprintf(list->logStream, "( id = %lu | data = " GLIST_PRINTF_CODE " | prev = %lu | next = %lu ) -> ", node->id, node->data, node->prev, node->next);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    do {
        status = gList_getNextNode(list, node->id, &node);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        fprintf(list->logStream, "( id = %lu | data = " GLIST_PRINTF_CODE " | prev = %lu | next = %lu ) -> ", node->id, node->data, node->prev, node->next);
    } while (node->id != list->zero);
    
    fprintf(list->logStream, "\n%s\n", LOG_DELIM);
    return gList_status_OK;
}

gList_status gList_dumpPoolGraphViz(const gList *list, FILE *fout)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr,  stderr);
    ASSERT_LOG(gPtrValid(fout), gList_status_BadDumpOutPtr, stderr);
    
    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n\tsubgraph cluster {\n");
    
    for (size_t i = 0; i < list->pool.capacity; ++i) {
        gList_Node *node = &list->pool.data[i].val;
        fprintf(fout, "\t\tnode%lu [label=\"Node %lu | {node_id | %lu} | {data | " GLIST_PRINTF_CODE "}\"]\n", i, i, node->id, node->data);
    }

    fprintf(fout, "\t}\n");
    
    for (size_t i = 0; i < list->pool.capacity; ++i) {
        gList_Node *node = &list->pool.data[i].val;
        if (node->id != 0 || node->next != 0) {
            fprintf(fout, "\tnode%lu -> node%lu\n", i, node->next);
            fprintf(fout, "\tnode%lu -> node%lu\n", node->next, i);
        }
    }

    fprintf(fout, "}\n");
    return gList_status_OK;
}

gList_status gList_dumpGraphViz(const gList *list, FILE *fout)
{
    ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr,  stderr);
    ASSERT_LOG(gPtrValid(fout), gList_status_BadDumpOutPtr, stderr);

    gList_Node *node = NULL;
    gList_status status = gList_status_OK;
    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n");
    
    status = (gList_status)gObjPool_get(&list->pool, list->zero, &node);
    fprintf(fout, "\tnode%lu [label=\"Node %lu | {data | " GLIST_PRINTF_CODE "}\"]\n", node->id, node->id, node->data);
    fprintf(fout, "\tnode%lu -> node%lu\n", node->id, node->next);
    fprintf(fout, "\tnode%lu -> node%lu\n", node->next, node->id);
    ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    do {
        status = gList_getNextNode(list, node->id, &node);
        ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        fprintf(fout, "\tnode%lu [label=\"Node %lu | {data | " GLIST_PRINTF_CODE "}\"]\n", node->id, node->id, node->data);
        fprintf(fout, "\tnode%lu -> node%lu\n", node->id, node->next);
        fprintf(fout, "\tnode%lu -> node%lu\n", node->next, node->id);
    } while (node->id != list->zero);
    
    fprintf(fout, "}\n");
    return gList_status_OK;
}

#endif /* GLIST_H */
