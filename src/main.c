#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define basefriendsize 8
#define memorygrowthmultiplier 2
#define memoryshrinkmultiplier 2

//types
typedef struct node {
	char *name;
	char **intrests;
	char *recentactivity;
	struct node **friends;
	int intrestcount;
} node_t;
typedef struct {
	node_t *nodelist;
	uint64_t *refrencetable;
	uint64_t nodelistcapacity;
	uint64_t nodelistzize;
} graph_t;
//function prototypes
int initnodelist(uint64_t basesize, graph_t **graph);
int deinitnodelist(graph_t **graph);
int grownodelist(graph_t *graph);
int shrinknodelist(graph_t *graph);

int initnodelist(uint64_t basesize, graph_t **graph)
{
	if (graph == NULL || *graph != NULL) {
		printf("Invalid arguement passed to 'initnodelist()' in graph NULL check triggered");
		return -1;
	}
	*graph = (graph_t *)malloc(sizeof(graph_t));
	if (*graph == NULL) {
		perror("Failed to Allocate Memory 'initnodelist()' ");
		return -1;
	}
	//allocate memory for graph nodes
	(*graph)->nodelist = (node_t *)malloc(sizeof(node_t) * basesize);
	if ((*graph)->nodelist == NULL) {
		perror("Failed to Allocate Memory 'initnodelist()' ");
		free(*graph);
		return -1;
	}
	(*graph)->refrencetable = (uint64_t *)malloc(sizeof(uint64_t) * basesize);
	if ((*graph)->refrencetable == NULL) {
		perror("Failed to Allocate Memory 'initnodelist()' ");
		free((*graph)->nodelist);
		free(*graph);
		return -1;
	}
	(*graph)->nodelistcapacity = basesize;
	(*graph)->nodelistzize = 0;
	return 0;
}
int deinitnodelist(graph_t **graph)
{
	node_t *node = NULL;
	if (graph == NULL || *graph == NULL) {
		printf("Invalid arguement passed to 'deinitnodelist()' in graph NULL check triggered");
		return -1;
	}
	if ((*graph)->nodelist == NULL) {
		printf("Improperly initilized nodelist detected may be corruption");
		goto graphfree;
	}
	for (uint64_t i = 0; i < (*graph)->nodelistzize; i++) {
		node = &((*graph)->nodelist[i]);
		free(node->name);
		free(node->recentactivity);
		free((void *)node->friends);
		if (node->intrests == NULL) {
			continue;
		}
		for (int j = 0; j < node->intrestcount; j++) {
			free(node->intrests[j]);
		}
		free((void *)node->intrests);
	}
graphfree:
	free((*graph)->refrencetable);
	free((*graph)->nodelist);
	free(*graph);
	*graph = NULL;
	return 0;
}
int grownodelist(graph_t *graph)
{
	uint64_t newnodelistcapacity = graph->nodelistcapacity * memorygrowthmultiplier;
	uint64_t *newrefrencetable = NULL;
	node_t *newnodelist = NULL;

	if (graph->nodelistcapacity > UINT64_MAX / memorygrowthmultiplier) {
		printf("Warning Possibility of Intiger overflow Detected Aborting 'grownodelist()' without making changes");
		return -1;
	}
	newnodelist = (node_t *)realloc(graph->nodelist, sizeof(node_t) * newnodelistcapacity);
	if (newnodelist == NULL) {
		perror("Failed to Allocate Memory 'grownodelist()' ");
		return -1;
	}
	graph->nodelist = newnodelist;

	newrefrencetable = (uint64_t *)realloc(graph->refrencetable, sizeof(uint64_t) * newnodelistcapacity);
	if (newrefrencetable == NULL) {
		perror("Failed to Allocate Memory 'grownodelist()' ");
		return -1;
	}
	graph->refrencetable = newrefrencetable;

	graph->nodelistcapacity = newnodelistcapacity;
	return 0;
}
int shrinknodelist(graph_t *graph)
{
	uint64_t newnodelistcapacity = graph->nodelistcapacity / memoryshrinkmultiplier;
	uint64_t *newrefrencetable = NULL;
	node_t *newnodelist = NULL;

	if (newnodelistcapacity < graph->nodelistzize) {
		printf("Warning Data Truncation Detected Aborting 'shrinknodelist()' without making changes");
		return -1;
	}
	if (newnodelistcapacity < basefriendsize) {
		printf("Warning Minimum Size Reached Aborting 'shrinknodelist()' without making changes");
		return -1;
	}
	newnodelist = (node_t *)realloc(graph->nodelist, sizeof(node_t) * newnodelistcapacity);
	if (newnodelist == NULL) {
		perror("Failed to Allocate Memory 'shrinknodelist()' ");
		return -1;
	}
	graph->nodelist = newnodelist;
	graph->nodelistcapacity = newnodelistcapacity;
	newrefrencetable = (uint64_t *)realloc(graph->refrencetable, sizeof(uint64_t) * newnodelistcapacity);
	if (newrefrencetable == NULL) {
		perror("Failed to Allocate Memory 'shrinknodelist()' ");
		return -1;
	}
	graph->refrencetable = newrefrencetable;
	return 0;
}

int main(void)
{
	return 0;
}
