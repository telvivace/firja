#include "objtree.h"
#include <stdio.h>
#include "liblogs.h"
#include "settings.h"
#include "treeutils.h"
//from GFG count set bits in integer
unsigned countSetBitsUL(unsigned long N)
{
    unsigned count = 0;
   
    for (unsigned i = 0; i < sizeof(unsigned long) * 8; i++) {
        if (N & (1UL << i))
            count++;
    }
    return count;
}

static int tree_printTree_aux(treeNode* node, FILE* file){
    if(node->buf){
        orb_logf(PRIORITY_TRACE,"on places : %lx we get %d", node->places, 64 - countSetBitsUL(node->places));
        fprintf(file, "\nchild {node {%d}}", 64 - countSetBitsUL(node->places));
    }

    else {
        fprintf(file, "\nchild {node {0}");
        orb_logf(PRIORITY_TRACE,"left");
        tree_printTree_aux(node->left, file);
        tree_printTree_aux(node->right, file);
        fprintf(file, "}");
    }
    return 0;
}
int tree_printTree(objTree* tree){
    FILE *file;
    const char *filename = "nodetrees_auto.tex";
    file = fopen(filename, "w");
    if (file == (void*)0) {
        perror("Error opening file");
        return 1;
    }
    fprintf(file, "  \\usetikzlibrary{graphs,graphdrawing} \\usegdlibrary{trees} \\tikz [tree layout, grow'=down, level distance=11mm, sibling distance=3mm,nodes={draw,fill=cyan!40,circle,inner sep=1pt, scale=0.6}]");
    if(tree->root->buf){
        fprintf(file, "\\node {%d}", 64 - countSetBitsUL(tree->root->places));
    }
    else{
    fprintf(file, "\\node {%d}", 64 - countSetBitsUL(tree->root->places));
    tree_printTree_aux(tree->root->left, file);
    tree_printTree_aux(tree->root->right, file);
    }
    
    fprintf(file, ";\n");
    fclose(file);
    return 0;
}
static int tree_printTreeBoxes_aux(treeNode* node){
    if(node->buf){
        orb_logf(PRIORITY_TRACE,"(%lf, %lf) x (%lf, %lf)", node->bindrect.lowlow.x, node->bindrect.lowlow.y, node->bindrect.highhigh.x, node->bindrect.highhigh.y);
        orb_logf(PRIORITY_DBUG,"up");
        return 0;
    }
    else {
        orb_logf(PRIORITY_DBUG,"left");
        tree_printTreeBoxes_aux(node->left);
        orb_logf(PRIORITY_DBUG,"right");
        tree_printTreeBoxes_aux(node->right);
    }
    return 0;
}
int tree_printTreeBoxes(objTree* tree){
    orb_logf(PRIORITY_DBUG,"OBJECT TREE PRINTOUT");
    tree_printTreeBoxes_aux(tree->root);
    return 0;
}
//chatgpt-ed
int tree_writeUintBufferToFile(const unsigned* buf, size_t size, char* foldername, char* filename){
    if (!buf || !foldername || !filename) {
        return 1; // Error: Invalid input
    }
    char fname[256] = {};
    snprintf(fname, sizeof(fname), "%s%s", foldername, filename);
    FILE *file = fopen(fname, "w"); //wipes old files
    if (!file) {
        return 1; // Error: File could not be opened
    }
    fprintf(file, "col1, col2\n");
    for (size_t i = 0; i < size; i++) {
        if (fprintf(file, "%lu,%u\n", i, buf[i]) < 0) {
            fclose(file);
            return 1; // Error: Writing to file failed
        }
    }

    fclose(file);
    return 0; // Success
}
void tree_printBufferContents(enum log_priorities priority, object* buf){
    orb_logf(priority, "Object buffer printout");
    for(unsigned i = 0; i < OBJBUFSIZE; i++){
        orb_logf(priority, "Object %d", i);
        tree_printObject(priority, buf + i);
    } 
}