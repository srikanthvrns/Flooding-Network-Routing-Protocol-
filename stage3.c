#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread.h>

# define N 10



typedef struct MSG {
	int sender;
	int TTL;
	int msg;
	struct MSG *next;
} node, *node_ptr;

node_ptr Q_head[N + 1], Q_tail[N + 1];

mutex_t lock[N + 2];

/*void sendMSG(int sender, int receiver, int msg, int TTL);
void enterQ(node newnode, int id);
void grabMSG(int id);*/

void enterQ(node_ptr newnode, int id) {
	mutex_lock(&lock[id]);
	if (Q_head[id] == NULL) {
		Q_head[id] = newnode;
		Q_tail[id] = newnode;
	} else {
		Q_tail[id]->next = newnode;
		Q_tail[id] = newnode;
	}
	mutex_unlock(&lock[id]);
}

void sendMSG(int sender, int receiver, int msg, int TTL) {
	node_ptr newNode = malloc(sizeof(node));
	newNode->msg = msg;
	newNode->TTL = TTL;
	newNode->sender = sender;
	newNode->next = NULL;
	enterQ(newNode, receiver);
}


node_ptr grabMSG(int id) {
	mutex_lock(&lock[id]);
	node_ptr recvmsg = NULL;
	if (Q_head[id]!= NULL) {
		recvmsg = Q_head[id];
		Q_head[id] = Q_head[id]->next;
		/* printf("Message received in Queue id: %d ", id);
		 printf("Message:%d\t TTL:%d\t sender:%d\t\n",recvmsg->msg,recvmsg->TTL,recvmsg->sender);*/
	} else {
		/*printf("The queue is empty\n");*/
	}
	mutex_unlock(&lock[id]);
	return recvmsg;
}

int main(int argc, char *argv[]) {
	FILE *fin;
	char peercount[3];
	char *neighbor;
	char *neighborcount;
	char reststring[50];
	int NW[N + 2][N + 2];
	node_ptr recv = NULL;
	int i = 0;
	int j = 0;
	int k;
	int n;
	for (i = 0; i < N + 2; i++) {
		for (j = 0; j < N + 2; j++) {
			NW[i][j] = 0;
		}
	}

	for (n = 0; n < N + 2; n++) {
		mutex_init(&lock[n], USYNC_THREAD, NULL);
	}
	i = 1;

	if ((fin = fopen(argv[1], "r")) == NULL) {
		printf("cannot open file");
		exit(1);
	}
	fgets(reststring, 5, fin);
	sscanf(reststring, "%s", peercount);
	while (i <= (atoi(peercount))) {
		if (!feof(fin)) {
			fgets(reststring, 50, fin);
			neighborcount = strtok(reststring, " ");
			for (j = 1; j <= atoi(neighborcount); j++) {
				neighbor = strtok(NULL, " ");
				k = atoi(neighbor);
				NW[i][k] = NW[k][i] = 1;
			}
			i++;
		} else {
			break;
		}
	}
	for (i = 1; i <= atoi(peercount) + 1; i++) {
		for (j = 1; j <= atoi(peercount) + 1; j++) {
			printf("%d\t", NW[i][j]);
		}
		printf("\n");
	}
	sendMSG(1, 2, 1000, 1); //sender = 1, receiver=2, msg = 1000, TTL=1
	sendMSG(1, 2, 2000, 2);
	sendMSG(1, 4, 2000, 4);

	if((recv = grabMSG(2))!= NULL)
	{
		printf("Message received in Queue id: 2 ");
		printf("Message:%d\t TTL:%d\t sender:%d\t\n",recv->msg,recv->TTL,recv->sender);
	}
	else
	{
		printf("The queue is empty\n");
	}
	if((recv = grabMSG(3))!= NULL)
		{
			printf("Message received in Queue id: 3 ");
			printf("Message:%d\t TTL:%d\t sender:%d\t\n",recv->msg,recv->TTL,recv->sender);
		}
		else
		{
			printf("The queue 3 is empty\n");
		}
	if((recv = grabMSG(2))!= NULL)
		{
			printf("Message received in Queue id: 2 ");
			printf("Message:%d\t TTL:%d\t sender:%d\t\n",recv->msg,recv->TTL,recv->sender);
		}
		else
		{
			printf("The queue is empty\n");
		}

	for (n = 0; n < N + 2; n++) {
		if (Q_head[n] != NULL) {
			printf("Message left for thread %d\n", n);
		}
	}

	for (n = 0; n < N + 2; n++) {
		mutex_destroy(&lock[n]);
	}
	return 0;
}
