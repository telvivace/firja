#include <stddef.h>
typedef struct tree_allocSubpool_s tree_allocSubpool;
struct tree_allocSubpool_s{
    void* pStart;
    size_t allocated;
    size_t used;
};
typedef struct {
    void* ptr;
    size_t size;
} free_hole;
typedef struct tree_allocPool_s* tree_allocPool;
struct tree_allocPool_s{
    size_t basesize;
    unsigned currentSubpool; //the subpool currently being filled
    tree_allocSubpool subpools[34];// ensures > 16GB space even with tiny pools
                                    // (exponentially^2 larger pools allocated)
    unsigned holecount;
    unsigned holesallocated;
    free_hole* holes;

};
tree_allocPool tree_allocInitPool(size_t size);
void tree_allocDestroyPool(tree_allocPool pool);
void* tree_allocate(tree_allocPool pool, size_t size);
void tree_free(void* pointer, size_t size, tree_allocPool pool);