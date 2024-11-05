#include "generaldefs.h"
#include <tgmath.h>
#include <unistd.h>
#define ever ;;
int main(){
    printf("start of main\n");
    globalInfo* globalInfo = calloc(sizeof(globalInfo), 1);
    printf("init tree\n");
    globalInfo->tree = tree_initTree();
    printf("start loop\n");
    for(unsigned i = 0; i < 40; i++){
        fprintf(stderr, "iteration %d\n", i);
        tree_insertObject(globalInfo->tree, &(object){.m = 5, .s = 5, .x = 100/40*i, .y = sin(100/40*i), .v = (speed){.x = i/100, .y = -i/100}});
        globalInfo->objectCount++;
    }
    
    for(ever){
        hit_flagObjects(globalInfo->tree);
        vector_update(globalInfo->tree->objectAllocPool->pStart, globalInfo->objectCount);
        scalar_update(globalInfo->tree->objectAllocPool->pStart, globalInfo->objectCount);
    }

}