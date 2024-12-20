#include "generaldefs.h"
#include "hit_detect.h"
#include "cpu_update.h"
#include "treeutils.h"
#include <stdio.h>
#include <time.h>
#include <tgmath.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define ever ;;
int main(int argc, char* argv[static 1]){
    int numObjects = 40;
    if(argc > 1)
        numObjects =  atoi(argv[1]); 
    printf("start of main\n");
    globalInformation* globalInfo = malloc(sizeof(globalInformation));
    globalInfo->bufferCount = 3;
    memset(globalInfo, 0, sizeof(*globalInfo));
    printf("init tree\n");
    globalInfo->tree = tree_initTree();
    printf("start loop\n");
    for(unsigned i = 0; i < numObjects; i++){
        fprintf(stderr, "iteration %d\n", i);
        tree_insertObject(globalInfo->tree, &(object){
            .m = 5, 
            .s = 5, 
            .x = 100/40*i, 
            .y = 100*sin(100/40*i), 
            .v = (speed){
                .x = i/100, 
                .y = -i/100
            }
        });
        globalInfo->objectCount++;
    }
    tree_printTree(globalInfo->tree);
    printf("Buffer count: %d\n", globalInfo->tree->bufCount);
    return 0;
    unsigned cycles = 0;
    struct timespec stoptime, starttime;
    timespec_get(&starttime, TIME_UTC);
    for(ever){
        hit_flagObjects(globalInfo->tree);
        vector_update(globalInfo->tree->objectAllocPool->pStart, globalInfo->bufferCount);
        scalar_update(globalInfo->tree->objectAllocPool->pStart, globalInfo->bufferCount);
        cycles++;
        if(cycles > 1000) break;
    }
    timespec_get(&stoptime, TIME_UTC);
    unsigned long deltatime = (stoptime.tv_sec  - starttime.tv_sec)  * 1000000
                       + (stoptime.tv_nsec - starttime.tv_nsec) / 1000 ;
    printf("Average FPS: %f\n", (double)cycles/((double)deltatime / 1000000));
    tree_printTree(globalInfo->tree);
}