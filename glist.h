#ifndef GLIST_H
#define GLIST_H

/**
 * @file Header containing generalized linked list data structure
 */

#include "stdio.h"
#include "stdlib.h"

#include "gutils.h"


/**
 * @brief basic list node containing next and prev id and the data
 */
struct gList_Node
{
    GLIST_TYPE data;            /// Stored user-provided data
    size_t next;                /// Id of the next node in list
    size_t prev;                /// Id of the previos node in list
    size_t id;                  /// Id of this node
} typedef gList_Node;

typedef gList_Node GOBJPOOL_TYPE;           /// Type for utility Object Pool data structure

static const size_t MAX_MSG_LEN = 64;       /// Max log message length

static const char LOG_DELIM[] = "=============================";    /// Delim line for text logs

#include "gobjpool.h"           // including utility Object Pool data structure


/**
 * @brief main linked list structure
 */
struct gList
{
    size_t size;                /// current size of the list
    size_t zero;                /// id of the service zero node
    gObjPool *pool;              /// Object Pool for memory management
    FILE *logStream;            /// Log stream for centralized logging
} typedef gList;


/**
 * @brief status codes for gList
 */
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
    gList_status_BadPool,
    gList_status_Cnt,
};


/**
 * @brief status codes explanations and error msgs for logs
 */
static const char gList_statusMsg[gList_status_Cnt][MAX_MSG_LEN] = {
    "OK",
    "Allocation error",
    "Bad capacity error",
    "Bad structure pointer provided",
    "Bad id provided",
    "Bad position requested",
    "Bad node pointer provided",
    "Bad FILE pointer provided to graphViz dump",
    "Bad objPool ptr provided",
};


/**
 * @brief macro that checks if objPool status is OK and convers error code to compatible gList_status otherwize
 */
#define CHECK_POOL_STATUS(status) ({        \
    if ((status) != gObjPool_status_OK)      \
        return (gList_status)(status);        \
})


/**
 * @brief Local version of ASSERT_LOG macro
 */
#ifndef NLOGS
#define GLIST_ASSERT_LOG(expr, errCode, logStream) ({                                   \
    if (!(expr)) {                                                                 \
        fprintf((logStream),  "%s in %s!\n", gList_statusMsg[(errCode)], __func__); \
        return (gList_status)(errCode);                                              \
    }                                                                                 \
})
#else
#define GLIST_ASSERT_LOG(...)
#endif

#define GLIST_ID_VAL(id) GLIST_ASSERT_LOG(gObjPool_idValid(list->pool, id), gList_status_BadId, list->logStream)

#define GLIST_NODE_BY_ID(id) ({                                     \
    GLIST_ID_VAL(id);                                                \
    &(list->pool->data[id].val);                                      \
})


/**
 * @brief gList constructor that initiates objPool and logStream and creates zero node
 * @param list pointer to structure to construct on
 * @param newLogStream new log stream, could be `NULL`, then logs will be written to `stderr`
 * @return gList status code
 */
gList_status gList_ctor(gList *list, FILE *newLogStream, gObjPool *pool)
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
    list->pool = pool;
    GLIST_ASSERT_LOG(gPtrValid(pool), gList_status_BadPool, list->logStream);
    CHECK_POOL_STATUS(gObjPool_alloc(list->pool, &list->zero));
    gList_Node *node;
    CHECK_POOL_STATUS(gObjPool_get(list->pool, list->zero, &node));
    node->next = list->zero;
    node->prev = list->zero;
    node->id = list->zero;
    return gList_status_OK;
}

gList *gList_new(FILE *newLogStream, size_t capacity)
{
    gList    *list = (gList*)calloc(1, sizeof(gList));
    gObjPool *pool = (gObjPool*)calloc(1, sizeof(gObjPool));
    if (gObjPool_ctor(pool, capacity, newLogStream) != 0)
        return NULL;

    if (gList_ctor(list, newLogStream, pool) != 0) {
        free(list);
        return NULL;
    }
    return list;
}


/**
 * @brief gList destructor
 * @param list pointer to structure to destruct
 * @return gList status code
 */
gList_status gList_dtor(gList *list)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    list->size = -1;
    gList_Node *node = GLIST_NODE_BY_ID(list->zero);
    if (node != NULL) {
        node->next = -1;
        node->prev = -1;
    }
    return gList_status_OK;
}


gList *gList_delete(gList *list)
{
    if (gList_dtor(list) != 0 || gObjPool_dtor(list->pool) != 0)
        return list;
    free(list->pool);
    free(list);
    return NULL;
}

/**
 * @brief return the id of the next Node (could be zero Node, if list is empty or used on the last element)
 * @param list pointer to structure
 * @param id id of the current Node
 * @param nextId pointer to id of the next Node, will be overriden
 * @return gList status code
 */
gList_status gList_getNextId(const gList *list, const size_t id, size_t *nextId)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    *nextId = GLIST_NODE_BY_ID(id)->next;

    return gList_status_OK;
}

/**
 * @brief return the pointer of the next Node (could be zero Node, if list is empty or used on the last element)
 * @param list pointer to structure
 * @param id id of the current Node
 * @param nextNode pointer to pointer to the next Node, will be overriden
 * @return gList status code
 */
gList_status gList_getNextNode(const gList *list, const size_t id, gList_Node **nextNode)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = GLIST_NODE_BY_ID(id);

    *nextNode = GLIST_NODE_BY_ID(node->next);

    return gList_status_OK;
}


/**
 * @brief gets list Node by position [takes O(n) time]
 * @param list pointer to structure
 * @param pos position of the desired Node
 * @param node pointer to pointer to the desired Node, will be overriden
 * @return gList status code
 */
gList_status gList_getNode(const gList *list, const size_t pos, gList_Node **node)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    size_t curId = list->zero;
    gList_status status = gList_status_OK;
    for (size_t i = 0; i < pos; ++i) {                          //TODO calibrate
        status = gList_getNextId(list, curId, &curId);
        if (status == gList_status_BadId)
            GLIST_ASSERT_LOG(status == gList_status_OK, gList_status_BadPos, list->logStream);
        GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    }
    *node = GLIST_NODE_BY_ID(curId);

    return gList_status_OK;
}


/**
 * @brief gets the data in the Node by the desired position [takes O(n) time]
 * @param list pointer to structure
 * @param pos position of the desired Node
 * @param data pointer to pointer to the Data by desired position, will be overriden
 * @return gList status code
 */
gList_status gList_getData(const gList *list, const size_t pos, GLIST_TYPE **data)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_Node *node = NULL;
    gList_status status = gList_getNode(list, pos, &node);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);
    *data = &node->data;

    return gList_status_OK;
}


/**
 * @brief insert Node after given one
 * @param list pointer to structure
 * @param id the id of the node to insert after (could be zero Node)
 * @param data data to put into the new node
 * @return gList status code
 */
gList_status gList_insertByNode(gList *list, size_t nodeId, const GLIST_TYPE data)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = GLIST_NODE_BY_ID(nodeId);

    size_t nextNodeId = node->next;
    size_t prevNodeId = node->id;

    size_t newNodeId = 0;
    gList_status status = (gList_status)gObjPool_alloc(list->pool, &newNodeId);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    gList_Node *nextNode = GLIST_NODE_BY_ID(nextNodeId);

    gList_Node *newNode = GLIST_NODE_BY_ID(newNodeId);

    node = GLIST_NODE_BY_ID(prevNodeId);

    newNode->next = nextNodeId;
    newNode->prev = prevNodeId;
    newNode->id   = newNodeId;
    newNode->data = data;

    node->next     = newNodeId;
    nextNode->prev = newNodeId;

    ++list->size;

    return gList_status_OK;
}


/**
 * @brief insert Node after given one
 * @param list pointer to structure
 * @param id the id of the node to insert after (could be zero Node)
 * @param data data to put into the new node
 * @return gList status code
 */
gList_status gList_insertByPos(gList *list, const size_t pos, GLIST_TYPE data)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_status status = gList_status_OK;

    gList_Node *node = NULL;
    status = gList_getNode(list, pos, &node);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    status = gList_insertByNode(list, node->id, data);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}


/**
 * @brief pop Node by id
 * @param list pointer to structure
 * @param id the id of the Node to pop
 * @param data pointer to put into the data to, if `NULL`, discards the data
 * @return gList status code
 */
gList_status gList_popByNode(gList *list, size_t nodeId, GLIST_TYPE *data)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = GLIST_NODE_BY_ID(nodeId);

    gList_Node *prevNode = GLIST_NODE_BY_ID(node->prev);

    gList_Node *nextNode = GLIST_NODE_BY_ID(node->next);

    prevNode->next = nextNode->id;
    nextNode->prev = prevNode->id;

    node->next = -1;
    node->prev = -1;

    if (data != NULL)
        *data = node->data;

    gList_status status = (gList_status)gObjPool_free(list->pool, node->id);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    --list->size;

    return gList_status_OK;
}


/**
 * @brief pop Node by position
 * @param list pointer to structure
 * @param pos the position of the Node to pop
 * @param data pointer to put into the data to, if `NULL`, discards the data
 * @return gList status code
 */
gList_status gList_popByPos(gList *list, const size_t pos, GLIST_TYPE *data)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);
    gList_status status = gList_status_OK;

    gList_Node *node = NULL;
    status = gList_getNode(list, pos, &node);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    status = gList_popByNode(list, node->id, data);
    GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

    return gList_status_OK;
}


/**
 * @brief dumps gList to logStream
 * @param list pointer to structure
 * @return gList status code
 */
gList_status gList_dump(const gList *list)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr, stderr);

    gList_Node *node = NULL;
    gList_status status = gList_status_OK;
    fprintf(list->logStream, "%s\ngList dump:\n%s\nsize = %lu\nzero_id = %lu\ndata:", LOG_DELIM, LOG_DELIM, list->size, list->zero);

    node = GLIST_NODE_BY_ID(list->zero);
    fprintf(list->logStream, "( id = %lu | data = " GLIST_PRINTF_CODE " | prev = %lu | next = %lu ) -> ", node->id, node->data, node->prev, node->next);
    do {
        status = gList_getNextNode(list, node->id, &node);
        GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        fprintf(list->logStream, "( id = %lu | data = " GLIST_PRINTF_CODE " | prev = %lu | next = %lu ) -> ", node->id, node->data, node->prev, node->next);
    } while (node->id != list->zero);

    fprintf(list->logStream, "\n%s\n", LOG_DELIM);
    return gList_status_OK;
}


/**
 * @brief dumps objPool of the list to fout stream in GraphViz format
 * @param list pointer to structure
 * @param fout stream to write dump to
 * @return gList status code
 */
gList_status gList_dumpPoolGraphViz(const gList *list, FILE *fout)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr,  stderr);
    GLIST_ASSERT_LOG(gPtrValid(fout), gList_status_BadDumpOutPtr, stderr);

    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n\tsubgraph cluster {\n");

    for (size_t i = 0; i < list->pool->capacity; ++i) {
        gList_Node *node = &list->pool->data[i].val;
        fprintf(fout, "\t\tnode%lu [label=\"Node %lu | {node_id | %lu} | {data | " GLIST_PRINTF_CODE "}\"]\n", i, i, node->id, node->data);
    }

    fprintf(fout, "\t}\n");

    for (size_t i = 0; i < list->pool->capacity; ++i) {
        gList_Node *node = &list->pool->data[i].val;
        if (node->id != 0 || node->next != 0) {
            fprintf(fout, "\tnode%lu -> node%lu\n", i, node->next);
            fprintf(fout, "\tnode%lu -> node%lu\n", node->next, i);
        }
    }

    fprintf(fout, "}\n");
    return gList_status_OK;
}


/**
 * @brief dumps gList to logStream
 * @param list pointer to structure
 * @return gList status code
 */
gList_status gList_dumpGraphViz(const gList *list, FILE *fout)
{
    GLIST_ASSERT_LOG(gPtrValid(list), gList_status_BadStructPtr,  stderr);
    GLIST_ASSERT_LOG(gPtrValid(fout), gList_status_BadDumpOutPtr, stderr);

    gList_Node *node = NULL;
    gList_status status = gList_status_OK;
    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n");

    node = GLIST_NODE_BY_ID(list->zero);
    fprintf(fout, "\tnode%lu [label=\"Node %lu | {size | %lu} | {data | " GLIST_PRINTF_CODE "}\"]\n", node->id, node->id, list->size, node->data);
    fprintf(fout, "\tnode%lu -> node%lu\n", node->id, node->next);
    fprintf(fout, "\tnode%lu -> node%lu\n", node->next, node->id);
    do {
        status = gList_getNextNode(list, node->id, &node);
        GLIST_ASSERT_LOG(status == gList_status_OK, status, list->logStream);

        fprintf(fout, "\tnode%lu [label=\"Node %lu | {data | " GLIST_PRINTF_CODE "}\"]\n", node->id, node->id, node->data);
        fprintf(fout, "\tnode%lu -> node%lu\n", node->id, node->next);
        fprintf(fout, "\tnode%lu -> node%lu\n", node->next, node->id);
    } while (node->id != list->zero);

    fprintf(fout, "}\n");
    return gList_status_OK;
}

#endif /* GLIST_H */
