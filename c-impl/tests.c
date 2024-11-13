#include "objtree.h"
int test_tree_allocInitDestroy(){
    tree_allocDestroyPool(tree_allocInitPool(1000));
}

//depends on the allocator tests

int test_tree_initTree(){
    objTree* ret = tree_initTree();
    printf("objectpool first, last byte: %c, %c\n", *(char*){ret->objectAllocPool->pStart}, *(char*){ret->objectAllocPool->pStart+ret->objectAllocPool->allocated-5});
    printf("nodepool first, last byte: %c, %c\n", *(char*){ret->nodeAllocPool->pStart}, *(char*){ret->nodeAllocPool->pStart+ret->nodeAllocPool->allocated-5});
    printf("root: %p\n", ret->root);
    tree_deleteTree(ret);
    printf("still alive and kicking\n");
    return 0;
}
int main(void){
    test_tree_allocInitDestroy();
    printf("PASS:   ALLOC INIT DESTROY\n");
    test_tree_initTree();
}