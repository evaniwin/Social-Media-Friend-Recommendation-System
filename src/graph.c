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
int64_t nodelistcapacity;
int64_t nodelistzize;

int initnodelist(int64_t basesize){
    //allocate memory for graph nodes
    nodelist = (node *)malloc(sizeof(node)*basesize);
    if (nodelist == NULL) {
        int errcode = errno;
        printf("Failed to Allocate Memory 'initnodelist()' errorcode: %d %s",errcode,strerror(errcode));
        return -1;
    }
    nodelistcapacity = basesize;
    nodelistzize = -1;
    return 0;
}
int grownodelist(){
    int64_t newnodelistcapacity = nodelistcapacity*2;
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
    int64_t newnodelistcapacity = nodelistcapacity/2;
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
int addnode(int64_t *handle){
    //perform a size check
    if(nodelistzize==nodelistcapacity-1){
        if(grownodelist()==-1){
            return -1;
        }
    }
    nodelistzize = nodelistzize+1;
    *handle = nodelistzize;
    return 0;
}
