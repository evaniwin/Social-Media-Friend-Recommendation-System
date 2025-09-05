#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#define minimumlistsize 8
#define basefriendssize 8
#define baseintrestsize 8

#define memorygrowthmultiplier 2
#define memoryshrinkmultiplier 2
#define growthreshold 100
#define shrinkthreshold 25

#define topercent(nominator, denominatior) ((nominator * 100) / denominatior)
//types
typedef struct node {
	char *name;
	char **intrests;
	char *recentactivity;
	uint64_t *friends;
	uint64_t friendCount;
	uint64_t friendCapacity;
	uint64_t searchhitcount;
	uint64_t intrestcount;
	uint64_t intrestcapacity;
} node_t;
typedef struct {
	node_t *nodelist;
	uint64_t nodelistcapacity;
	uint64_t nodelistsize;
} graph_t;
typedef struct {
	uint64_t friend;
	uint64_t level;
} Bfs_t;
typedef struct {
	Bfs_t *memory;
	uint64_t front;
	uint64_t size;
	uint64_t capacity;
} queue_t;
typedef struct {
	uint64_t friend;
	uint64_t mutualconnections;
} friends_t;
//function prototypes
int initnodelist(uint64_t basesize, graph_t **graph);
int deinitnodelist(graph_t **graph);
int grownodelist(graph_t *graph);
int checkshrinkfit(uint64_t count, uint64_t capacity, uint64_t minsize);
uint64_t max(uint64_t numa, uint64_t numb);
int shrinknodelist(graph_t *graph);
int addnode(graph_t *graph, uint64_t *noderef);
int removenode(graph_t *graph, uint64_t noderef);
int checkifalreadyfriend(node_t *node, uint64_t refid);
int swapbackarrayremove(uint64_t *array, uint64_t arraysize, uint64_t target);
int addfriend(graph_t *graph, uint64_t friend1, uint64_t friend2);
int removefriend(graph_t *graph, uint64_t friend1, uint64_t friend2, int attemptshrinking);
int addintrest(graph_t *graph, uint64_t refno, char *intrest);
int removeintrest(graph_t *graph, uint64_t refno, uint64_t intrestno);
int createqueue(queue_t **queue, uint64_t size);
int destroyqueue(queue_t **queue);
int enqueue(queue_t *queue, uint64_t value, uint64_t level);
int dequeue(queue_t *queue, Bfs_t *value);
int peek(queue_t *queue, Bfs_t *value, uint64_t index);
int resetsearchhitcount(graph_t *graph);
int BFS_mutualfriend(graph_t *graph, uint64_t refno, friends_t **recommendationlist, uint64_t *size);

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
	(*graph)->nodelistcapacity = basesize;
	(*graph)->nodelistsize = 0;
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
	for (uint64_t i = 0; i < (*graph)->nodelistsize; i++) {
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
	free((*graph)->nodelist);
	free(*graph);
	*graph = NULL;
	return 0;
}
int grownodelist(graph_t *graph)
{
	uint64_t newnodelistcapacity = graph->nodelistcapacity * memorygrowthmultiplier;
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

	graph->nodelistcapacity = newnodelistcapacity;
	return 0;
}
int checkshrinkfit(uint64_t count, uint64_t capacity, uint64_t minsize)
{
	int inshrinkthreshold = topercent(count, capacity) < shrinkthreshold;
	int isfit = (capacity / memoryshrinkmultiplier) > count;
	int isabovemin = capacity > minsize;
	return inshrinkthreshold && isfit && isabovemin;
}
uint64_t max(uint64_t numa, uint64_t numb)
{
	if (numa > numb) {
		return numa;
	}
	return numb;
}
int shrinknodelist(graph_t *graph)
{
	uint64_t newnodelistcapacity = graph->nodelistcapacity / memoryshrinkmultiplier;
	node_t *newnodelist = NULL;

	if (newnodelistcapacity < graph->nodelistsize) {
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
	return 0;
}
int addnode(graph_t *graph, uint64_t *noderef)
{
	node_t *newnode = NULL;
	if (graph->nodelistcapacity == graph->nodelistsize) {
		if (grownodelist(graph)) {
			return -1;
		}
	}
	newnode = &(graph->nodelist[graph->nodelistsize]);
	newnode->name = NULL;
	newnode->recentactivity = NULL;
	newnode->friendCount = 0;
	newnode->intrestcount = 0;
	newnode->searchhitcount = 0;
	newnode->friends = (uint64_t *)malloc(sizeof(uint64_t) * basefriendssize);
	if (newnode->friends == NULL) {
		perror("Failed to Allocate Memory 'addnode()' ");
		return -1;
	}
	newnode->friendCapacity = basefriendssize;

	newnode->intrests = (char **)malloc(sizeof(char *) * baseintrestsize);
	if (newnode->intrests == NULL) {
		perror("Failed to Allocate Memory 'addnode()' ");
		free(newnode->friends);
		return -1;
	}
	newnode->intrestcapacity = baseintrestsize;
	*noderef = graph->nodelistsize;
	graph->nodelistsize++;
	return 0;
}
int removenode(graph_t *graph, uint64_t noderef)
{
	node_t *node = &(graph->nodelist[noderef]);
	uint64_t size = 0;
	uint64_t refrence = 0;
	//logic to remove node
	if (graph->nodelistsize <= noderef) {
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
	graph->nodelistsize--;
	graph->nodelist[noderef] = graph->nodelist[graph->nodelistsize];
	//update old refrences
	size = graph->nodelist[noderef].friendCount;
	for (uint64_t i = 0; i < size; i++) {
		refrence = graph->nodelist[noderef].friends[i];
		for (uint64_t j = 0; j < graph->nodelist[refrence].friendCount; j++) {
			if (graph->nodelist[refrence].friends[j] == graph->nodelistsize) {
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
	if (graph->nodelistsize <= friend1 || graph->nodelistsize <= friend2) {
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
		if (arraysize > 1) {
			array[foundind] = array[arraysize - 1];
		}
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
	if (graph->nodelistsize <= friend1 || graph->nodelistsize <= friend2) {
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
		uint64_t newsize = 0;
		if (checkshrinkfit(nodef1->friendCount, nodef1->friendCapacity, basefriendssize)) {
			newsize = max(nodef1->friendCapacity / memoryshrinkmultiplier, basefriendssize);
			friendlist = (uint64_t *)realloc(nodef1->friends, sizeof(uint64_t) * newsize);
			if (friendlist == NULL) {
				perror("Memory shrink operation failed 'removefriend()' ");
			} else {
				nodef1->friends = friendlist;
				nodef1->friendCapacity = newsize;
			}
		}

		if (checkshrinkfit(nodef2->friendCount, nodef2->friendCapacity, basefriendssize)) {
			newsize = max(nodef2->friendCapacity / memoryshrinkmultiplier, basefriendssize);
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
int addintrest(graph_t *graph, uint64_t refno, char *intrest)
{
	char **tempintrest = NULL;
	char *newintrest = NULL;
	node_t *node = NULL;
	if (graph == NULL) {
		printf("Error null passed insted of pointer to graph in 'addintrest()'\n");
		return -1;
	}
	if (intrest == NULL) {
		printf("Null passed insted of intrest string in 'addintrest()'\n");
		return -1;
	}
	if (refno >= graph->nodelistsize) {
		printf("Error invalid node refrence in 'addintrest()'\n");
		return -1;
	}
	newintrest = (char *)malloc(sizeof(char) * (strlen(intrest) + 1));
	if (newintrest == NULL) {
		perror("Memory allocation failed 'addintrest()' ");
		return -1;
	}
	strcpy(newintrest, intrest);
	node = &(graph->nodelist[refno]);
	if (node->intrestcount >= node->intrestcapacity) {
		uint64_t newsize = node->intrestcapacity * memorygrowthmultiplier;
		if (newsize < node->intrestcapacity) {
			printf("Possible intiger overflow or misconfiguration detected\n");
			return -1;
		}
		tempintrest = (char **)realloc((void *)node->intrests, sizeof(char *) * newsize);
		if (tempintrest == NULL) {
			perror("Memory allocation failed 'addintrest()' ");
			free(newintrest);
			return -1;
		}
		node->intrestcapacity = newsize;
		node->intrests = tempintrest;
	}
	node->intrests[node->intrestcount] = newintrest;
	node->intrestcount++;
	return 0;
}
int removeintrest(graph_t *graph, uint64_t refno, uint64_t intrestno)
{
	char **tempintrest = NULL;
	node_t *node = NULL;

	if (graph == NULL) {
		printf("Error null passed insted of pointer to graph in 'removeintrest()'\n");
		return -1;
	}
	if (refno >= graph->nodelistsize) {
		printf("Error invalid node refrence in 'removeintrest()'\n");
		return -1;
	}
	if (intrestno >= graph->nodelist[refno].intrestcount) {
		printf("Error invalid intrest refrence in 'removeintrest()'\n");
		return -1;
	}
	node = &(graph->nodelist[refno]);
	//free the memory and swapback last element to fill hole
	free(node->intrests[intrestno]);
	node->intrestcount--;
	if (node->intrestcount != 0) {
		node->intrests[intrestno] = node->intrests[node->intrestcount];
	}
	if (checkshrinkfit(node->intrestcount, node->intrestcapacity, baseintrestsize)) {
		uint64_t newsize = max(node->intrestcapacity / memoryshrinkmultiplier, baseintrestsize);
		tempintrest = (char **)realloc((void *)node->intrests, sizeof(char *) * newsize);
		if (tempintrest == NULL) {
			perror("Memory allocation failed 'removeintrest()' ");
			return -1;
		}
		node->intrests = tempintrest;
		node->intrestcapacity = newsize;
	}
	return 0;
}
int createqueue(queue_t **queue, uint64_t size)
{
	if (queue == NULL) {
		printf("Error null check failed, NULL given as arguement when expected pointer in 'createqueue()'\n");
		return -1;
	}
	if (*queue != NULL) {
		printf("Error null check failed queue not initilized to NULL in 'createqueue()'\n");
		return -1;
	}
	if (size == 0) {
		printf("Error cannot create queue of zero size in 'createqueue()'\n");
		return -1;
	}
	*queue = (queue_t *)malloc(sizeof(queue_t));
	if (*queue == NULL) {
		perror("Memory allocation failed 'createqueue()' ");
		return -1;
	}
	(*queue)->memory = (Bfs_t *)malloc(sizeof(Bfs_t) * size);
	if ((*queue)->memory == NULL) {
		perror("Memory allocation failed 'createqueue()' ");
		free(*queue);
		*queue = NULL;
		return -1;
	}
	(*queue)->front = 0;
	(*queue)->size = 0;
	(*queue)->capacity = size;
	return 0;
}
int destroyqueue(queue_t **queue)
{
	if (queue == NULL) {
		printf("Error null check failed, NULL given as arguement when expected pointer in 'destroyqueue()'\n");
		return -1;
	}
	if (*queue == NULL) {
		printf("Error null check failed queue not exist in 'destroyqueue()'\n");
		return -1;
	}
	free((*queue)->memory);
	free(*queue);
	*queue = NULL;
	return 0;
}
int enqueue(queue_t *queue, uint64_t value, uint64_t level)
{
	uint64_t next = 0;
	if (queue == NULL) {
		printf("Error null check failed, NULL given as arguement when expected pointer in 'enqueue()'\n");
		return -1;
	}
	if (queue->size == queue->capacity) {
		printf("Error queue is full unable to enqueue\n");
		return -1;
	}
	next = (queue->front + queue->size) % queue->capacity;
	queue->memory[next].friend = value;
	queue->memory[next].level = level;
	queue->size++;
	return 0;
}
int dequeue(queue_t *queue, Bfs_t *value)
{
	if (queue == NULL || value == NULL) {
		printf("Error null check failed, NULL given as arguement when expected pointer in 'dequeue()'\n");
		return -1;
	}
	if (queue->size == 0) {
		printf("Error queue is empty unable to dequeue\n");
		return -1;
	}
	*value = queue->memory[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size--;
	return 0;
}
int peek(queue_t *queue, Bfs_t *value, uint64_t index)
{
	uint64_t relativeindex = 0;
	if (queue == NULL || value == NULL) {
		printf("Error null check failed, NULL given as arguement when expected pointer in 'peek()'\n");
		return -1;
	}
	if (index >= queue->size) {
		printf("Out of bounds\n");
		return -1;
	}
	relativeindex = (queue->front + index) % queue->capacity;
	*value = queue->memory[relativeindex];
	return 0;
}
int resetsearchhitcount(graph_t *graph)
{
	for (uint64_t i = 0; i < graph->nodelistsize; i++) {
		graph->nodelist[i].searchhitcount = 0;
	}
	return 0;
}
int BFS_mutualfriend(graph_t *graph, uint64_t refno, friends_t **recommendationlist, uint64_t *size)
{
	queue_t *queue = NULL;
	Bfs_t value;
	uint64_t friend = 0;
	if (graph == NULL) {
		printf("Error null passed insted of pointer to graph in 'BFS_mutualfriend()'\n");
		return -1;
	}
	if (refno >= graph->nodelistsize) {
		printf("Invalid refno in 'BFS_mutualfriend()'\n");
		return -1;
	}
	resetsearchhitcount(graph);
	createqueue(&queue, graph->nodelistsize);
	enqueue(queue, refno, 0);
	while (1) {
		if (peek(queue, &value, 0)) {
			break;
		}
		if (value.level >= 2) {
			break;
		}
		dequeue(queue, &value);
		for (uint64_t j = 0; j < graph->nodelist[value.friend].friendCount; j++) {
			friend = graph->nodelist[value.friend].friends[j];
			if (graph->nodelist[friend].searchhitcount == 0) {
				enqueue(queue, friend, value.level + 1);
			}
			graph->nodelist[friend].searchhitcount++;
		}
	}
	//TODO Create sorted friend recommendation list based on mutualconnections
	*recommendationlist = (friends_t *)malloc(sizeof(friends_t) * queue->size);
	if (*recommendationlist == NULL) {
		perror("Memory allocation failed 'BFS_mutualfriend()' ");
		destroyqueue(&queue);
		return -1;
	}
	*size = queue->size;
	for (uint64_t i = 0; i < queue->size; i++) {
		uint64_t bigger = 0;
		friends_t friendtmp;
		Bfs_t val1;
		peek(queue, &val1, i);
		for (uint64_t j = 0; j < queue->size; j++) {
			Bfs_t val2;
			peek(queue, &val2, j);
			if (graph->nodelist[val1.friend].searchhitcount < graph->nodelist[val2.friend].searchhitcount) {
				bigger++;
			}
			if (graph->nodelist[val1.friend].searchhitcount == graph->nodelist[val2.friend].searchhitcount) {
				if (i < j) {
					bigger++;
				}
			}
		}
		friendtmp.friend = val1.friend;
		friendtmp.mutualconnections = graph->nodelist[val1.friend].searchhitcount;
		(*recommendationlist)[bigger] = friendtmp;
	}
	destroyqueue(&queue);
	return 0;
}
int main(void)
{
	return 0;
}
