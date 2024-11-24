#include "objtree.h"
#include <stdio.h>
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
        printf("on places : %lx we get %d\n", node->places, 64 - countSetBitsUL(node->places));
        fprintf(file, "\nchild {node {%d}}", 64 - countSetBitsUL(node->places));
    }

    else {
        fprintf(file, "\nchild {node {0}");
        printf("left\n");
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
    fprintf(file, "  \\usetikzlibrary{graphs,graphdrawing} \\usegdlibrary{trees} \\tikz [tree layout, grow'=right, level distance=11mm, sibling distance=3mm,nodes={draw,fill=cyan!40,circle,inner sep=1pt, scale=0.6}]");
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