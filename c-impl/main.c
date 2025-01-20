#include "generaldefs.h"
#include "hit_detect.h"
#include "cpu_update.h"
#include "treeutils.h"
#include "settings.h"
#define GRAPHICS_ON
#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <tgmath.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define ever ;;
volatile unsigned long frames = 0; //for diagnostic purposes
void printFramesAtSegfault(int signal){
    printf("Frames rendered: %ld\n", frames);
    printf("SEGMENTATION FAULT\n");
    exit(EXIT_FAILURE);
}

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
        printf("node rect at x: %lf -- %lf  y : %lf -- %lf\n", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
        SDL_Rect rect = {
            .x = node->bindrect.lowlow.x,
            .y = node->bindrect.lowlow.y,
            .w = node->bindrect.highhigh.x - node->bindrect.lowlow.x,
            .h = node->bindrect.highhigh.y - node->bindrect.lowlow.y,
        };
        SDL_RenderDrawRect(renderer, &rect);
        printf("buffer is null. Left: %p, Right: %p\n", node->left, node->right);
        printf("going left\n");
        renderObjects_rec(node->left, renderer);
        printf("going right\n");
        renderObjects_rec(node->right, renderer);
        printf("UP ( %p -> %p)\n", node, node->up);
        return;
    }
    else {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    printf("node rect at x: %lf -- %lf  y : %lf -- %lf\n", node->bindrect.lowlow.x, node->bindrect.highhigh.x, node->bindrect.lowlow.y, node->bindrect.highhigh.y);
    SDL_Rect rect = {
        .x = node->bindrect.lowlow.x,
        .y = node->bindrect.lowlow.y,
        .w = node->bindrect.highhigh.x - node->bindrect.lowlow.x,
        .h = node->bindrect.highhigh.y - node->bindrect.lowlow.y,
    };
    SDL_RenderDrawRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    
    printf("\ndrawing no. ");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        if(node->buf[i].s){
                if(node->buf[i].id == 24){
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    drawCircle(renderer, node->buf[i].x, node->buf[i].y, node->buf[i].s);
                    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
                    continue;
                }
            printf("draw at x:%lf y:%lf s:%f  ", node->buf[i].x, node->buf[i].y, node->buf[i].s);
            drawCircle(renderer, node->buf[i].x, node->buf[i].y, node->buf[i].s);
        }
    }
    printf("\n\n");
    printf("UP ( %p -> %p)\n", node, node->up);
    return;
    }
}
int main(int argc, char* argv[static 1]){
    signal(SIGSEGV, printFramesAtSegfault);
    int numObjects = 40;
    unsigned limitedcycles = 0;
    long int maxcycles = 0;
    printf("argc: %d\n", argc);
    if(argc > 1)
        numObjects =  atoi(argv[1]); 
    if(argc > 2){
        maxcycles = atol(argv[2]);
        limitedcycles = 1;
        printf("limited cycles to %ld.\n", maxcycles);
    }
    printf("start of main\n");
    globalInformation* globalInfo = calloc(1, sizeof(globalInformation));
    globalInfo->bufferCount = 3;
    printf("init tree\n");
    globalInfo->tree = tree_initTree();
    printf("start loop\n");
    for(unsigned i = 0; i < numObjects; i++){
        fprintf(stderr, "iteration %d\n", i);
        printf("inserting object with id %d", i);
        tree_insertObject(globalInfo->tree, &(object){
            .m = 5, 
            .s = 5, 
            .x = rand() % (RIGHTBORDER - LEFTBORDER - 10) + 5,
            .y = rand() % (TOPBORDER - BOTTOMBORDER- 10) + 5,
            .v = (speed){
                .x = ((rand() % 10) - 5) / 2.0f,
                .y = ((rand() % 10) - 5) / 2.0f,
            },
            .id = i,
        });
        globalInfo->objectCount++;
    }
    tree_printTree(globalInfo->tree);
    tree_printTreeBoxes(globalInfo->tree);
    printf("Buffer count: %d\n", globalInfo->tree->bufCount);
    #ifdef GRAPHICS_ON
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("2D Canvas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Event e;
    #endif    
    unsigned running = 1;


    unsigned cycles = 0;
    struct timespec stoptime, starttime;
    timespec_get(&starttime, TIME_UTC);
    while(running){
        #ifdef GRAPHICS_ON
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
                continue;
            }
        }
        #endif
        hit_flagObjects(globalInfo->tree);
        vector_update(globalInfo->tree);
        scalar_update(globalInfo->tree);
        if(limitedcycles == 1 && cycles > maxcycles) running = 0;
        #ifdef GRAPHICS_ON
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        printf("Drawing.\n");
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        renderObjects_rec(globalInfo->tree->root, renderer);


        // Update and draw sprites
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        printf("presenting frame\n");
        tree_printTreeBoxes(globalInfo->tree);
        // Present rendered frame
        SDL_RenderPresent(renderer);
        // Delay to cap frame rate
        SDL_Delay(12); // ~60 FPS
        #endif
        cycles++;
        frames++;
    }
            
    #ifdef GRAPHICS_ON
    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    #endif

    return 0;
    timespec_get(&stoptime, TIME_UTC);
    unsigned long deltatime = (stoptime.tv_sec  - starttime.tv_sec)  * 1000000
                       + (stoptime.tv_nsec - starttime.tv_nsec) / 1000 ;
    printf("Average FPS: %f\n", (double)cycles/((double)deltatime / 1000000));
    tree_printTree(globalInfo->tree);
    tree_deleteTree(globalInfo->tree);
    free(globalInfo);
}