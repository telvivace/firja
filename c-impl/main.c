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
                if(node->buf[i].id == 42){
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
        printf("inserting object at x: %d y: %lf v: x: %lf y: %lf", 100/40*i, 100*sin(100/40*i), (float)i/100, -(float)i/100);
        tree_insertObject(globalInfo->tree, &(object){
            .m = 5, 
            .s = 5, 
            .x = rand() % (RIGHTBORDER - LEFTBORDER - 5),
            .y = rand() % (TOPBORDER - BOTTOMBORDER- 5),
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
    unsigned running = 1;
    SDL_Event e;

    unsigned cycles = 0;
    struct timespec stoptime, starttime;
    timespec_get(&starttime, TIME_UTC);
    while(running){
        hit_flagObjects(globalInfo->tree);
        vector_update(globalInfo->tree);
        scalar_update(globalInfo->tree);
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = 0;
                continue;
            }
        }
        hit_flagObjects(globalInfo->tree);
        vector_update(globalInfo->tree);
        scalar_update(globalInfo->tree);
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
        cycles++;
        frames++;
    }
            

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
    timespec_get(&stoptime, TIME_UTC);
    unsigned long deltatime = (stoptime.tv_sec  - starttime.tv_sec)  * 1000000
                       + (stoptime.tv_nsec - starttime.tv_nsec) / 1000 ;
    printf("Average FPS: %f\n", (double)cycles/((double)deltatime / 1000000));
    tree_printTree(globalInfo->tree);
    tree_deleteTree(globalInfo->tree);
    free(globalInfo);
}