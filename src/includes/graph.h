#ifndef GRAPH_H_
#define GRAPH_H_

//includes
#include <stdint.h>

//types
typedef struct node {
    char * name;
    char ** intrests;
    char * recentactivity;
    struct node * friends;
} node ;

//function prototypes
int initnodelist(uint64_t basesize);
int grownodelist();
int shrinknodelist();


#endif // GRAPH_H_