struct node{
    char *id;               //jobID
    char *job;              //job
    int pos;                //Queue position
    int isRunning;          //Flag for wether the job is at running state or queued state
    pid_t pid;              //Store the process id only when the job is getting executed
    struct node *next;      //Pointer to the next node of the queue
};

typedef struct node NODE;

struct queue{
	NODE *front;          //Pointer to the front of the queue
    NODE *end;            //Pointer to the rear of the queue
};

typedef struct queue QUEUE;

void init(QUEUE *q);
int isEmpty(QUEUE *q);
void add(QUEUE *q, char *id, char *job);
void stop(QUEUE *q, char *id, pid_t p);
void stopvol(QUEUE *q, char *id);
char* getInfo(QUEUE *q);
char* getQueuedJobs(QUEUE *q);
char* getRunningJobs(QUEUE *q);
int getIsRunning(QUEUE *q, char *id);
void setRunning(QUEUE *q);
void move(QUEUE *source, QUEUE *destination);
void set(QUEUE *q, char *jobID);
char* getFirstJob(QUEUE *q);
int reachedElements(QUEUE *q, int N);
void setPid(QUEUE *q, char *id, pid_t pid);
void removeByPid(QUEUE *q, pid_t pid);
int lessThanN(QUEUE *q, int N);
char* getFirstJobID(QUEUE *q);
void deleteNode(QUEUE *q, NODE *prev, NODE *cur);