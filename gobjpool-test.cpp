typedef int GOBJPOOL_TYPE;  //TODO remove

#include "gobjpool.h"


int main()
{
    gObjPool pool;
    gObjPool_ctor(&pool, -1, NULL);
    gObjPool_dumpFree(&pool, NULL);


    size_t id;
    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);
    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);
    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);
    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);

    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);

    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);
    gObjPool_dumpFree(&pool, NULL);


    gObjPool_free(&pool, 3);
    gObjPool_free(&pool, 5);
    gObjPool_dumpFree(&pool, NULL);


    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);


    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);


    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);


    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);
    
    gObjPool_dumpFree(&pool, NULL);

    gObjPool_alloc(&pool, &id);
    printf("%d\n", id);

    gObjPool_dumpFree(&pool, NULL);

    gObjPool_free(&pool, 1);
    gObjPool_free(&pool, 7);
    gObjPool_free(&pool, 3);
    gObjPool_free(&pool, 5);
    gObjPool_dumpFree(&pool, NULL);

    gObjPool_dtor(&pool);
}
