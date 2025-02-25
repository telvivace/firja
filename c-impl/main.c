#include "generaldefs.h"
#include "hit_detect.h"
#include "cpu_update.h"
#include "treeutils.h"
#include "settings.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <tgmath.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "liblogs.h"
volatile unsigned long frames = 0; //for diagnostic purposes
struct timespec starttime = {}; //so it can be printed out if interrupted
struct timespec stoptime = {}; //so it can be printed out if interrupted
double g_leftborder = LEFTBORDER; //   provide definitions for the
double g_rightborder = RIGHTBORDER; // header-declared modifiable borders
double g_topborder = TOPBORDER; //     the macros are default values.
double g_bottomborder = BOTTOMBORDER;
void printFramesAtInterrupt(int signal){
    timespec_get(&stoptime, TIME_UTC);
    if(signal == SIGSEGV){
        orb_logf(PRIORITY_ERR, "Segmentation fault!");
    }
    else {
        orb_logf(PRIORITY_OK, "Interrupted.");
    }
    orb_logf(PRIORITY_OK, "Frames rendered: %ld", frames);
    unsigned long deltatime = (stoptime.tv_sec  - starttime.tv_sec)  * 1000000 + (stoptime.tv_nsec - starttime.tv_nsec) / 1000 ;
    orb_logf(PRIORITY_OK,"Average FPS: %f", (double)frames/((double)deltatime / 1000000));
    exit(EXIT_SUCCESS);
}
#if GRAPHICS_ON
// Function to draw a circle using the Midpoint Circle Algorithm
void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius) {
    int x = radius;
    int y = 0;
    int radiusError = 1 - x;

    while (x >= y) {
        // Draw symmetric points in all eight octants
        SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY + y);
        SDL_RenderDrawPoint(renderer, centerX + x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX - x, centerY - y);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY + x);
        SDL_RenderDrawPoint(renderer, centerX + y, centerY - x);
        SDL_RenderDrawPoint(renderer, centerX - y, centerY - x);

        y++;
        if (radiusError < 0) {
            radiusError += 2 * y + 1;
        } else {
            x--;
            radiusError += 2 * (y - x) + 1;
        }
    }
}
void renderObjects_rec(treeNode* node, SDL_Renderer* renderer){
    if(!node->buf){
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        orb_logf(PRIORITY_TRACE,"node rect at x: %lf -- %lf  y : %lf -- %lf", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
        SDL_Rect rect = {
            .x = node->bindrect.lowlow.x,
            .y = node->bindrect.lowlow.y,
            .w = node->bindrect.highhigh.x - node->bindrect.lowlow.x,
            .h = node->bindrect.highhigh.y - node->bindrect.lowlow.y,
        };
        SDL_RenderDrawRect(renderer, &rect);
        orb_logf(PRIORITY_TRACE,"buffer is null. Left: %p, Right: %p", node->left, node->right);
        orb_logf(PRIORITY_DBUG,"going left");
        renderObjects_rec(node->left, renderer);
        orb_logf(PRIORITY_DBUG,"going right");
        renderObjects_rec(node->right, renderer);
        orb_logf(PRIORITY_DBUG,"UP ( %p -> %p)", node, node->up);
        return;
    }
    else {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    orb_logf(PRIORITY_TRACE,"node rect at x: %lf -- %lf  y : %lf -- %lf", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
    SDL_Rect rect = {
        .x = node->bindrect.lowlow.x,
        .y = node->bindrect.lowlow.y,
        .w = node->bindrect.highhigh.x - node->bindrect.lowlow.x,
        .h = node->bindrect.highhigh.y - node->bindrect.lowlow.y,
    };
    SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        if(node->buf[i].s){
                if(node->buf[i].id == 24){
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    drawCircle(renderer, node->buf[i].x, node->buf[i].y, node->buf[i].s);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                    continue;
                }
            orb_logf(PRIORITY_TRACE,"draw at x:%lf y:%lf s:%f  ", node->buf[i].x, node->buf[i].y, node->buf[i].s);
            drawCircle(renderer, node->buf[i].x, node->buf[i].y, node->buf[i].s);
        }
    }
    orb_logf(PRIORITY_BLANK,"UP ( %p -> %p)", node, node->up);
    return;
    }
}
#endif
int main(int argc, char* argv[static 1]){
    signal(SIGSEGV, printFramesAtInterrupt);
    signal(SIGINT, printFramesAtInterrupt);
    long int numObjects = 40;
    unsigned limitedcycles = 0;
    long int maxcycles = 0;
    orb_logf(PRIORITY_OK,"argc: %d", argc);
    if(argc > 1){
        numObjects =  atol(argv[1]); 
        orb_logf(PRIORITY_OK, "running with %ld objects.", numObjects);
    }
    if(argc > 2){
        maxcycles = atol(argv[2]);
        limitedcycles = 1;
        orb_logf(PRIORITY_OK,"limited cycles to %ld.", maxcycles);
    }
    double simulationDimensions = sqrt(15000 * numObjects);
    g_rightborder = simulationDimensions;
    g_topborder = simulationDimensions;
    orb_logf(PRIORITY_WARN, "x:%lf -- %lf y:%lf -- %lf", g_leftborder, g_rightborder, g_bottomborder, g_topborder);
    orb_logf(PRIORITY_DBUG,"start of main");
    globalInformation* globalInfo = calloc(1, sizeof(globalInformation));
    globalInfo->bufferCount = 3;
    orb_logf(PRIORITY_DBUG,"init tree");
    globalInfo->tree = tree_initTree();
    orb_logf(PRIORITY_DBUG,"start loop");
    for(unsigned i = 0; i < numObjects; i++){
        orb_logf(PRIORITY_TRACE, "iteration %d", i);
        orb_logf(PRIORITY_TRACE,"inserting object with id %d", i);
        tree_insertObject(globalInfo->tree, &(object){
            .m = 5, 
            .s = 5, 
            .x = rand() % ((unsigned long)g_rightborder - (unsigned long)g_leftborder - 10) + 5,
            .y = rand() % ((unsigned long)g_topborder - (unsigned long)g_bottomborder- 10) + 5,
            .v = (speed){
                .x = ((rand() % 10) - 5) / 2.0f,
                .y = ((rand() % 10) - 5) / 2.0f,
            },
            .id = i,
        });
        globalInfo->objectCount++;
    }
    #if DBUG_MODE == 1
    tree_printTree(globalInfo->tree);
    tree_printTreeBoxes(globalInfo->tree);
    #endif
    orb_logf(PRIORITY_TRACE,"Buffer count: %d", globalInfo->tree->bufCount);
    #if GRAPHICS_ON == 1
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        orb_logf(PRIORITY_ERR,"SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("2D Canvas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        orb_logf(PRIORITY_ERR,"Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        orb_logf(PRIORITY_ERR,"Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Event e;
    #endif    
    unsigned running = 1;


    unsigned long cycles = 0;
    timespec_get(&starttime, TIME_UTC);
    struct timespec dynamictime;
    struct timespec dynamictime_prev = starttime;
    while(running){
        #if GRAPHICS_ON == 1
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
                continue;
            }
        }
        #endif
        if(limitedcycles == 1 && cycles >= maxcycles) break;
        hit_flagObjects(globalInfo->tree);
        globalInfo->tree->validObjCount = 0;
        globalInfo->tree->relocations = 0;
        vector_update(globalInfo->tree);
        scalar_update(globalInfo->tree);
        #if RECYCLE == 1
        tree_optimizeNodes(globalInfo->tree);
        #endif
        #if GRAPHICS_ON == 1
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        orb_logf(PRIORITY_DBUG,"Drawing.");
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        renderObjects_rec(globalInfo->tree->root, renderer);


        // Update and draw sprites
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        orb_logf(PRIORITY_DBUG,"presenting frame");
        tree_printTreeBoxes(globalInfo->tree);
        // Present rendered frame
        SDL_RenderPresent(renderer);
        // Delay to cap frame rate
        SDL_Delay(10); // ~60 FPS
        #endif
        cycles++;
        frames++;
        if(limitedcycles){
            timespec_get(&dynamictime, TIME_UTC);
            printf("\rProgress: %3f%%, Average FPS: %f FPS: %lf Object count: %.1ld Opt_queue length: %u           ", (float)cycles/(float)maxcycles * 100, 
            (double)cycles/((double)((dynamictime.tv_sec  - starttime.tv_sec)  * 1000000
                       + (dynamictime.tv_nsec - starttime.tv_nsec) / 1000) / 1000000), 1000000/(double)(((dynamictime.tv_sec  - dynamictime_prev.tv_sec)  * 1000000
                       + (dynamictime.tv_nsec - dynamictime_prev.tv_nsec) / 1000)), globalInfo->tree->validObjCount, globalInfo->tree->opt_queue.nodecount);
            fflush(stdout);
            dynamictime_prev = dynamictime;
            if(globalInfo->tree->validObjCount > numObjects) orb_logf(PRIORITY_ERR, "boom bam beowm big bad objects too much for understeawm %lu/%lu, relocations: %lu", globalInfo->tree->validObjCount, numObjects, globalInfo->tree->relocations);
        }
    }
            
    #if GRAPHICS_ON == 1
    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    #endif

    fputc('\n', stdout);
    timespec_get(&stoptime, TIME_UTC);
    unsigned long deltatime = (stoptime.tv_sec  - starttime.tv_sec)  * 1000000
                       + (stoptime.tv_nsec - starttime.tv_nsec) / 1000 ;
    orb_logf(PRIORITY_OK,"Average FPS: %f", (double)cycles/((double)deltatime / 1000000));
    orb_log(PRIORITY_OK, "Reinsertions:", globalInfo->tree->relocations);
    #if DBUG_MODE == 1
    tree_printTree(globalInfo->tree);
    #endif
    tree_deleteTree(globalInfo->tree);
    free(globalInfo);
    return 0;
}