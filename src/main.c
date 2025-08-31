#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#define minimumlistsize 8
#define memorygrowthmultiplier 2
#define memoryshrinkmultiplier 2
#define growthreshold 100
#define shrinkthreshold 25
#define basefriendssize 8
#define topercent(nominator, denominatior) ((nominator * 100) / denominatior)
//types
typedef struct node {
	char *name;
	char **intrests;
	char *recentactivity;
	uint64_t *friends;
	uint64_t friendCount;
	uint64_t friendCapacity;
	uint64_t intrestcount;
} node_t;
typedef struct {
	node_t *nodelist;
	//TODO remove refrence table
	uint64_t *refrencetable;
	uint64_t nodelistcapacity;
	uint64_t nodelistzize;
} graph_t;
//function prototypes
int initnodelist(uint64_t basesize, graph_t **graph);
int deinitnodelist(graph_t **graph);
int grownodelist(graph_t *graph);
int shrinknodelist(graph_t *graph);
int addnode(graph_t *graph, uint64_t *noderef);
int removenode(graph_t *graph, uint64_t noderef);
int checkifalreadyfriend(node_t *node, uint64_t refid);
int swapbackarrayremove(uint64_t *array, uint64_t arraysize, uint64_t target);
int addfriend(graph_t *graph, uint64_t friend1, uint64_t friend2);
int removefriend(graph_t *graph, uint64_t friend1, uint64_t friend2, int attemptshrinking);

int initnodelist(uint64_t basesize, graph_t **graph)
{
	if (graph == NULL || *graph != NULL) {
		printf("Invalid arguement passed to 'initnodelist()' in graph NULL check triggered\n");
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
		printf("Invalid arguement passed to 'deinitnodelist()' in graph NULL check triggered\n");
		return -1;
	}
	if ((*graph)->nodelist == NULL) {
		printf("Improperly initilized nodelist detected may be corruption\n");
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
		for (uint64_t j = 0; j < node->intrestcount; j++) {
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
		printf("Warning Possibility of Intiger overflow Detected Aborting 'grownodelist()' without making changes\n");
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
		printf("Warning Data Truncation Detected Aborting 'shrinknodelist()' without making changes\n");
		return -1;
	}
	if (newnodelistcapacity < minimumlistsize) {
		printf("Warning Minimum Size Reached Aborting 'shrinknodelist()' without making changes\n");
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
int addnode(graph_t *graph, uint64_t *noderef)
{
	node_t *newnode = NULL;
	if (graph->nodelistcapacity == graph->nodelistzize) {
		if (grownodelist(graph)) {
			return -1;
		}
	}
	newnode = &(graph->nodelist[graph->nodelistzize]);
	newnode->name = NULL;
	newnode->recentactivity = NULL;
	newnode->friendCount = 0;
	newnode->intrestcount = 0;
	newnode->friends = (uint64_t *)malloc(sizeof(uint64_t) * basefriendssize);
	if (newnode->friends == NULL) {
		perror("Failed to Allocate Memory 'addnode()' ");
		return -1;
	}
	newnode->friendCapacity = basefriendssize;
	newnode->intrests = NULL;
	*noderef = graph->nodelistzize;
	graph->nodelistzize++;
	return 0;
}
int removenode(graph_t *graph, uint64_t noderef)
{
	node_t *node = &(graph->nodelist[noderef]);
	uint64_t size = 0;
	uint64_t refrence = 0;
	//logic to remove node
	if (graph->nodelistzize <= noderef) {
		printf("Error Out of bounds access in friend does not exist 'removenode()'\n");
		return -1;
	}
	free(node->name);
	node->name = NULL;
	free(node->recentactivity);
	node->recentactivity = NULL;

	while (node->friendCount != 0) {
		removefriend(graph, noderef, node->friends[0], 0);
	}
	free(node->friends);
	node->friends = NULL;

	for (uint64_t i = 0; i < node->intrestcount; i++) {
		free(node->intrests[i]);
	}
	free((void *)node->intrests);
	node->intrests = NULL;
	graph->nodelistzize--;
	graph->nodelist[noderef] = graph->nodelist[graph->nodelistzize];
	//update old refrences
	size = graph->nodelist[noderef].friendCount;
	for (uint64_t i = 0; i < size; i++) {
		refrence = graph->nodelist[noderef].friends[i];
		for (uint64_t j = 0; j < graph->nodelist[refrence].friendCount; j++) {
			if (graph->nodelist[refrence].friends[j] == graph->nodelistzize) {
				graph->nodelist[refrence].friends[j] = noderef;
				break;
			}
		}
	}
	return 0;
}
int checkifalreadyfriend(node_t *node, uint64_t refid)
{
	for (uint64_t i = 0; i < node->friendCount; i++) {
		if (node->friends[i] == refid) {
			return 1;
		}
	}
	return 0;
}
int addfriend(graph_t *graph, uint64_t friend1, uint64_t friend2)
{
	uint64_t *friendlist = NULL;
	node_t *nodef1 = NULL;
	node_t *nodef2 = NULL;
	int f1exist = 0;
	int f2exist = 0;
	if (graph == NULL) {
		printf("Error Null given as parameter array in 'addfriend()'\n");
		return -1;
	}
	if (friend1 == friend2) {
		printf("Attept to add self as friend detected\n");
		return -1;
	}
	if (graph->nodelistzize <= friend1 || graph->nodelistzize <= friend2) {
		printf("Error Out of bounds access in friend does not exist 'addfriend()'\n");
		return -1;
	}
	nodef1 = &(graph->nodelist[friend1]);
	nodef2 = &(graph->nodelist[friend2]);
	//check if friend
	f1exist = checkifalreadyfriend(nodef1, friend2);
	f2exist = checkifalreadyfriend(nodef2, friend1);
	if (f1exist || f2exist) {
		if (f1exist && f2exist) {
			printf("Already friend\n");
			return -1;
		}
		printf("Already friend, possible corruption detected\n");
		return -1;
	}

	//grow the friend list of first friend if full
	if (graph->nodelist[friend1].friendCount == graph->nodelist[friend1].friendCapacity) {
		if (nodef1->friendCapacity > UINT64_MAX / memorygrowthmultiplier) {
			printf("Warning Possibility of Intiger overflow Detected Aborting 'addfriend()'\n");
			return -1;
		}
		friendlist = (uint64_t *)realloc(nodef1->friends, sizeof(uint64_t) * nodef1->friendCapacity * memorygrowthmultiplier);
		if (friendlist == NULL) {
			perror("Failed to Allocate Memory for grow operation in 'addfriend()' ");
			return -1;
		}
		nodef1->friends = friendlist;
		nodef1->friendCapacity = nodef1->friendCapacity * memorygrowthmultiplier;
	}

	//grow the friend list of second friend if full
	if (graph->nodelist[friend2].friendCount == graph->nodelist[friend2].friendCapacity) {
		if (nodef2->friendCapacity > UINT64_MAX / memorygrowthmultiplier) {
			printf("Warning Possibility of Intiger overflow Detected Aborting 'addfriend()'\n");
			return -1;
		}
		friendlist = (uint64_t *)realloc(nodef2->friends, sizeof(uint64_t) * nodef2->friendCapacity * memorygrowthmultiplier);
		if (friendlist == NULL) {
			perror("Failed to Allocate Memory for grow operation in 'addfriend()' ");
			return -1;
		}
		nodef2->friends = friendlist;
		nodef2->friendCapacity = nodef2->friendCapacity * memorygrowthmultiplier;
	}
	nodef1->friends[nodef1->friendCount] = friend2;
	nodef1->friendCount++;
	nodef2->friends[nodef2->friendCount] = friend1;
	nodef2->friendCount++;
	return 0;
}
int swapbackarrayremove(uint64_t *array, uint64_t arraysize, uint64_t target)
{
	uint64_t foundind = 0;
	int found = 0;
	if (array == NULL) {
		printf("Error Null given as parameter array in 'swapbackarrayreplace()'\n");
		return -1;
	}
	for (foundind = 0; foundind < arraysize; foundind++) {
		if (array[foundind] == target) {
			found = 1;
			break;
		}
	}
	if (found) {
		array[foundind] = array[arraysize - 1];
	} else {
		printf("Error Element to remove not found in 'swapbackarrayreplace()'\n");
		return -1;
	}
	return 0;
}
int removefriend(graph_t *graph, uint64_t friend1, uint64_t friend2, int attemptshrinking)
{
	uint64_t *friendlist = NULL;
	node_t *nodef1 = NULL;
	node_t *nodef2 = NULL;
	int f1exist = 0;
	int f2exist = 0;
	if (graph == NULL) {
		printf("Error Null given as parameter array in 'removefriend()'\n");
		return -1;
	}
	if (friend1 == friend2) {
		printf("Attept to remove self sa friend detected\n");
		return -1;
	}
	if (graph->nodelistzize <= friend1 || graph->nodelistzize <= friend2) {
		printf("Error Out of bounds access in possible corruption 'removefriend()'\n");
		return -1;
	}
	nodef1 = &(graph->nodelist[friend1]);
	nodef2 = &(graph->nodelist[friend2]);
	//check if friend
	f1exist = checkifalreadyfriend(nodef1, friend2);
	f2exist = checkifalreadyfriend(nodef2, friend1);
	if (f1exist || f2exist) {
		if (!(f1exist && f2exist)) {
			printf("possible corruption detected\n");
			return -1;
		}
	} else {
		printf("Not friend not possible to delete");
		return -1;
	}

	//remove friends
	if (swapbackarrayremove(nodef1->friends, nodef1->friendCount, friend2)) {
		printf("Friend removal failed\n");
		return -1;
	}
	nodef1->friendCount--;
	if (swapbackarrayremove(nodef2->friends, nodef2->friendCount, friend1)) {
		//rollback prevoius operation
		nodef1->friends[nodef1->friendCount] = friend2;
		nodef1->friendCount++;
		printf("Friend removal failed\n");
		return -1;
	}
	nodef2->friendCount--;

	//shrink to save memory if conditions meet
	if (attemptshrinking) {
		uint64_t newsize = nodef1->friendCapacity / memoryshrinkmultiplier;
		if ((basefriendssize < newsize) && (topercent(nodef1->friendCount, nodef1->friendCapacity) <= shrinkthreshold)) {
			friendlist = (uint64_t *)realloc(nodef1->friends, sizeof(uint64_t) * newsize);
			if (friendlist == NULL) {
				perror("Memory shrink operation failed 'removefriend()' ");
			} else {
				nodef1->friends = friendlist;
				nodef1->friendCapacity = newsize;
			}
		}

		newsize = nodef2->friendCapacity / memoryshrinkmultiplier;
		if ((basefriendssize < newsize) && (topercent(nodef2->friendCount, nodef2->friendCapacity) <= shrinkthreshold)) {
			friendlist = (uint64_t *)realloc(nodef2->friends, sizeof(uint64_t) * newsize);
			if (friendlist == NULL) {
				perror("Memory shrink operation failed 'removefriend()' ");
			} else {
				nodef2->friends = friendlist;
				nodef2->friendCapacity = newsize;
			}
		}
	}
	return 0;
}
int main(void)
{
	return 0;
}
