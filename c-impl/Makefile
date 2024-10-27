STD = -std=c2x
#OPT = -O3
OPT= -g
CFLAGS= -Wall ${STD} ${OPT}

phyeng: main.o

main.o: hit_detect.o cpu_update.o

cpu_update.o: objtree.o tree_alloc.o

ALL: phyeng