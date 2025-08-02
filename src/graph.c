#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define basefriendsize 8

typedef struct node {
    char * name;
    char ** intrests;
    char * recentactivity;
    struct node * friends;
} node ;

node *nodelist;
uint64_t nodelistcapacity;
uint64_t nodelistzize;

int initnodelist(uint64_t basesize){
    //allocate memory for graph nodes
    nodelist = (node *)malloc(sizeof(node)*basesize);
    if (nodelist == NULL) {
        int errcode = errno;
        printf("Failed to Allocate Memory 'initnodelist()' errorcode: %d %s",errcode,strerror(errcode));
        return -1;
    }
    nodelistcapacity = basesize;
    nodelistzize = 0;
    return 0;
}
int grownodelist(){
    uint64_t newnodelistcapacity = nodelistcapacity*2;
    node * newnodelist = (node *)realloc(nodelist,sizeof(node)*newnodelistcapacity);
    if (newnodelist == NULL) {
        int errcode = errno;
        printf("Failed to Allocate Memory 'grownodelist()' errorcode: %d %s",errcode,strerror(errcode));
        return -1;
    }
    nodelist = newnodelist;
    nodelistcapacity = newnodelistcapacity;
    return 0;
}
int shrinknodelist(){
    uint64_t newnodelistcapacity = nodelistcapacity/2;
    node * newnodelist = (node *)realloc(nodelist,sizeof(node)*newnodelistcapacity);
    if (newnodelist == NULL) {
        int errcode = errno;
        printf("Failed to Allocate Memory 'shrinknodelist()' errorcode: %d %s",errcode,strerror(errcode));
        return -1;
    }
    nodelist = newnodelist;
    nodelistcapacity = newnodelistcapacity;
    return 0;
}
int addnode(uint64_t *handle){
    //perform a size check
    if(nodelistzize==nodelistcapacity){
        if(grownodelist()==-1){
            return -1;
        }
    }
    nodelistzize = nodelistzize+1;
    *handle = nodelistzize-1;
    return 0;
}
void copynodedata(node *destination,node * source){
    destination->name=source->name;
    destination->intrests=source->intrests;
    destination->recentactivity=source->recentactivity;
    destination->friends=source->friends;
}
///impliment swapback array for node list
int removenode(uint64_t nodehandle){
    //perform a size check

    //if(nodelistzize<nodelistcapacity/2){
    //    if(shrinknodelist()==-1){
    //        return -1;
    //    }
    //}
    uint64_t lastnode = nodelistzize-1;
    if(nodehandle < 0 || nodehandle > lastnode){
        printf("Invalid node Handle %lu",nodehandle);
        return -1;
    }
    //overwrite the data of element to be deleted with last element
    copynodedata(&nodelist[nodehandle], &nodelist[lastnode]);
    nodelistzize = nodelistzize - 1;

    return 0;
}
