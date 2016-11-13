#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread.h>

# define N 10
thread_t tid, thr[N + 2]; //storage to save thread id of all threads
FILE *logfin;
struct timespec Tim = { 0, 10000 }; //0 sec, 10000 nanosec

typedef struct MSG {
	int sender;
	int TTL;
	int msg;
	struct MSG *next;
} node, *node_ptr;

node_ptr Q_head[N + 1], Q_tail[N + 1];
int num_entries_queues;
mutex_t lock[N + 2];
char peercount[3];
int NW[N + 2][N + 2];
void *peer(void *args);
/* void sendMSG(int sender, int receiver, int msg, int TTL);
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
	num_entries_queues=num_entries_queues+1;
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
	node_ptr recvmsg = malloc(sizeof(node));
	if (Q_head[id] != NULL) {
		recvmsg = Q_head[id];
		Q_head[id] = Q_head[id]->next;
		num_entries_queues=num_entries_queues-1;
		/* printf("Message received in Queue id: %d ", id);
		 printf("Message:%d\t TTL:%d\t sender:%d\t\n",recvmsg->msg,recvmsg->TTL,recvmsg->sender);*/
	} else {
		recvmsg = NULL;
		/*printf("The queue is empty\n");*/
	}
	mutex_unlock(&lock[id]);
	return recvmsg;
}

int main(int argc, char *argv[]) {
	int status;
	FILE *fin;
	char *neighbor;
	char *neighborcount;
	char reststring[50];
	int msgmain,TTLmain;
	int i = 0;
	int j = 0;
	int k;
	int n;
	for (i = 0; i < N + 2; i++) {
		for (j = 0; j < N + 2; j++) {
			NW[i][j] = 0;
		}
	}
	logfin = fopen("Stage7_log_input", "w");
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
	/*	for (i = 1; i <= atoi(peercount) + 1; i++) {
	 for (j = 1; j <= atoi(peercount) + 1; j++) {
	 printf("%d\t", NW[i][j]);
	 }
	 printf("\n");
	 } */

	for (n = 2; n < atoi(peercount) + 2; n++)
		thr_create(NULL, 0, peer, (void*) n, THR_BOUND, &thr[n]);


	while(!feof(fin))
				{
					fgets(reststring, 50, fin);
					sscanf(reststring, "%d%d",&msgmain,&TTLmain);
					sendMSG(1,2,msgmain, TTLmain);
					nanosleep(&Tim, NULL);


				}

	while (thr_join(0, &tid, (void**) &status) == 0)
	{
			printf("\nmain - status=%d, first_id=%d\n", status, tid);
	}


	for (n = 0; n < N + 2; n++) {
		if (Q_head[n] != NULL) {
			printf("\nMessage left for thread %d", n);
		}
	}

	for (n = 0; n < N + 2; n++) {
		mutex_destroy(&lock[n]);
	}

	fclose(fin);
	fclose(logfin);
	return 0;
}

void *peer(void *args) {
	int i = 0;
	int sum=0;
	int k = atoi(peercount);
	int num_EMPTY= 10;
	int numemptycount=0;
	int myID = thr_self();
	int Num_rounds_no_msg = 0;
	int sender,msg,TTL;
	int rounds[12]={0};
	int roundindex=0;
	node_ptr recv = malloc(sizeof(node));
	int myNeighbors[k + 1];
	for (i = 0; i < k + 2; i++) {
		myNeighbors[i] = NW[myID][i];
	}
	printf("\nMythread id : %d", myID);
	printf("\nPeers of thread %d are:", myID);
	for (i = 1; i <= atoi(peercount) + 1; i++) {

			printf("%d\t", myNeighbors[i]);
	}
	printf("\n");

	usleep(300);
while(Num_rounds_no_msg<=num_EMPTY)
{
	recv = grabMSG(myID);
	if (recv != NULL) {
		while(recv!=NULL)
		{
		msg = recv->msg;
		TTL = recv->TTL;
		sender = recv->sender;
		if (TTL > 0) {
			TTL--;
			for (i = 2; i < k + 2; i++) {
				if (i != sender && myNeighbors[i] == 1 && i != myID) {
					sendMSG(myID, i, msg, TTL);
					/*	printf(
					 "sender=%d,myID=%d,myNeighbors=%d,Message=%d,TTL=%d\n",
					 sender, myID, i, msg, TTL);*/
					if(Num_rounds_no_msg>0)
					{
					rounds[roundindex]= Num_rounds_no_msg;
					roundindex++;
					Num_rounds_no_msg = 0;
					}
					fprintf(logfin,"%d\t%d\t%d\t%d\t%d\n",sender, myID, i, msg, TTL);
				}
			}

		}
			recv = grabMSG(myID);
	}
	}
	else
	{
		Num_rounds_no_msg ++;
	}
	numemptycount++;
	free(recv);
	usleep(300);

}

for(i=0;i<roundindex;i++)
{
sum=sum+rounds[i];
printf("Thread id : %d\t num of consecutive rounds for time %d:%d\n",myID,i,rounds[i]);
}

printf("\nThe smallest no of num_EMPTY for thread %d in which all messages got propagated: %d\n",myID,sum);
//printf("Thread id : %d\t total rounds:%d\n",myID,numemptycount);
if(roundindex==0)
{
printf("\nThe average no of rounds for which the thread %d became empty before finding a message  is: %f\n",myID,(float)sum);
}
else
{
printf("\nThe average no of rounds for which the thread %d became empty before finding a message  is: %f\n",myID,((float)sum/(float)(roundindex)));
}
return NULL;
}
