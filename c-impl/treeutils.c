#include "objtree.h"
#include <stdio.h>
#include "liblogs.h"
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
        orb_logf(PRIORITY_TRACE,"on places : %lx we get %d\n", node->places, 64 - countSetBitsUL(node->places));
        fprintf(file, "\nchild {node {%d}}", 64 - countSetBitsUL(node->places));
    }

    else {
        fprintf(file, "\nchild {node {0}");
        orb_logf(PRIORITY_TRACE,"left\n");
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
static int tree_printTreeBoxes_aux(treeNode* node){
    if(node->buf){
        orb_logf(PRIORITY_TRACE,"(%lf, %lf) x (%lf, %lf)\n", node->bindrect.lowlow.x, node->bindrect.lowlow.y, node->bindrect.highhigh.x, node->bindrect.highhigh.y);
        orb_logf(PRIORITY_DBUG,"up\n");
        return 0;
    }
    else {
        orb_logf(PRIORITY_DBUG,"left\n");
        tree_printTreeBoxes_aux(node->left);
        orb_logf(PRIORITY_DBUG,"right\n");
        tree_printTreeBoxes_aux(node->right);
    }
    return 0;
}
int tree_printTreeBoxes(objTree* tree){
    orb_logf(PRIORITY_DBUG,"OBJECT TREE PRINTOUT\n");
    tree_printTreeBoxes_aux(tree->root);
    return 0;
}