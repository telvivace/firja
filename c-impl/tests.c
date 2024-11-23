#include "objtree.h"
int test_tree_allocInitDestroy(){
    tree_allocDestroyPool(tree_allocInitPool(1000));
    return 0;
}

//depends on the allocator tests

int test_tree_initTree(){
    objTree* ret = tree_initTree();
    printf("objectpool first, last byte: %c, %c\n", *(char*){ret->objectAllocPool->pStart}, *(char*){ret->objectAllocPool->pStart+ret->objectAllocPool->allocated-1});
    printf("nodepool first, last byte: %c, %c\n", *(char*){ret->nodeAllocPool->pStart}, *(char*){ret->nodeAllocPool->pStart+ret->nodeAllocPool->allocated-1});
    printf("root: %p\n", ret->root);
    tree_deleteTree(ret);
    printf("still alive and kicking\n");
    return 0;


}
int test_countBits(){
    printf("1: %d\n", countSetBitsUL(1));
    printf("7: %d\n", countSetBitsUL(7));
    printf("8: %d\n", countSetBitsUL(8));
    printf("10:%d\n", countSetBitsUL(10));
}
int test_printTree(){
    tree_allocPool* pool = tree_allocInitPool(2000);
    
}
int main(void){
    test_tree_allocInitDestroy();
    printf("PASS:   ALLOC INIT DESTROY\n");
    test_tree_initTree();
    printf("PASS:   TREE_INITTREE\n");
    test_countBits();
    return 0;
}

