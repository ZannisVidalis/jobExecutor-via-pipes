#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <limits.h>
#include "queue.h"

//Initialize the queue
void init(QUEUE *q)
{
    q->front = NULL;
    q->end = NULL;
}


//Return 1/0 if the queue is/isn't empty
int isEmpty(QUEUE *q)
{
    return (q->front == NULL);
}


//Add the job to the queue
void add(QUEUE *q, char *id, char *job)
{
    NODE *n;
    
    n = (NODE *)malloc(sizeof(NODE));
    if(!n) {printf("Memory allocation failed.\n");}

    n->id = strdup(id);
    n->job = strdup(job);
    n->isRunning = 0;                   //The job is not running at the start
    n->next = NULL;
    n->pid = -1;

    //If queue empty -> set front and end to the n node
    if(isEmpty(q)){
        q->front = n;
        q->end = n;
        n->pos = 1;                     //First position
    }   
    //Else queue has already a node so -> Add the n node to the end of the queue
    else{
        q->end->next = n;
        q->end = n;

        //Update the positioning of the queue: Increment positions of the nodes in queue
        NODE *cur = q->front;
        int pos = 1;
        while(cur!=NULL){
            cur->pos = pos;
            cur = cur->next;
            pos++;
        }
        n->pos = pos-1;                 //Assign position to the new node
    }
}


//For terminating running processes via the job_id & pid
void stop(QUEUE *q, char *id, pid_t p) 
{
    NODE *prev = NULL;
    NODE *cur = q->front;

    //Traverse the queue to find the node with the given ID
    while(cur != NULL && strcmp(cur->id, id) != 0){
        prev = cur;
        cur = cur->next;
    }

    //If the node with the given ID is found
    if(cur != NULL) 
    {
        cur->isRunning = 0;         //Set the isRunning variable to 0 -> i.e the job isnt running

        kill(cur->pid, SIGTERM);    //Kill the process

        deleteNode(q, prev, cur);   //Delete the node
        
        //Update the positioning of the queue after deletion
        NODE *cur = q->front;
        int pos = 1;
        while(cur != NULL){
            cur->pos = pos;
            cur = cur->next;
            pos++;
        }
    } 
}


//For removing queued jobs (Same with the stop())
void stopvol(QUEUE *q, char *id) 
{
    NODE *prev = NULL;
    NODE *cur = q->front;

    //Traverse the queue to find the node with the given ID
    while(cur != NULL && strcmp(cur->id, id) != 0){
        prev = cur;
        cur = cur->next;
    }

    //If the node with the given ID is found
    if(cur != NULL) 
    {
        cur->isRunning = 0;         //Set the isRunning variable to 0 -> i.e the job isnt running

        deleteNode(q, prev, cur);   //Delete the node
        
        //Update the positioning of the queue after deletion
        NODE *cur = q->front;
        int pos = 1;
        while(cur != NULL){
            cur->pos = pos;
            cur = cur->next;
            pos++;
        }
    } 
}


//Returns the triplet for jobCommander
char* getInfo(QUEUE *q) 
{
    //Check if the queue is empty
    if(!isEmpty(q)) 
    {
        //Allocate memory
        char *info = (char *)malloc(1024 * sizeof(char));
        if(!info){ printf("Memory allocation failed.\n");}

        //Traverse the queue to find the last node
        NODE *cur = q->front;
        while(cur->next != NULL){
            cur = cur->next;
        }

        //Construct the triplet
        sprintf(info, "%s,%s,%d", cur->id, cur->job, cur->pos);

        return info;
    }
}


//Returns the queued jobs for the "poll queued" instruction
char* getQueuedJobs(QUEUE *q) 
{
    //Allocate memory
    char *info = (char *)malloc(PIPE_BUF * sizeof(char));
    if(!info){ printf("Memory allocation failed.\n");}

    //Traverse the queue to find nodes with isRunning = 0
    NODE *cur = q->front;
    while(cur != NULL) 
    {
        if(cur->isRunning == 0) 
        {
            //Allocate memory
            char *jobInfo = (char *)malloc(1024 * sizeof(char));
            if(!jobInfo){ printf("Memory allocation failed.\n");}

            //Concatenate the triplet
            sprintf(jobInfo, "%s,%s,%d\n", cur->id, cur->job, cur->pos);
            strcat(info, jobInfo);

            //free the mem
            free(jobInfo);
        }
        cur = cur->next;
    }

    return info;
}


//Returns the running jobs for the "poll running" instruction
char* getRunningJobs(QUEUE *q) 
{
    //Allocate memory
    char *info = (char *)malloc(PIPE_BUF * sizeof(char));
    if(!info){ printf("Memory allocation failed.\n");}

    //Traverse the queue to find nodes with isRunning = 1
    NODE *cur = q->front;
    while(cur != NULL) 
    {
        if(cur->isRunning == 1) 
        {
            //Allocate memory
            char *jobInfo = (char *)malloc(1024 * sizeof(char));
            if(!jobInfo){ printf("Memory allocation failed.\n");}

            //Concatenate the triplet
            sprintf(jobInfo, "%s,%s,%d\n", cur->id, cur->job, cur->pos);
            strcat(info, jobInfo);

            //free the mem
            free(jobInfo);
        }
        cur = cur->next;
    }

    return info;
}


//Returns 1/0 whether the specific job is/isn't running   
int getIsRunning(QUEUE *q, char *id) 
{
    //Traverse the queue
    NODE *cur = q->front;
    while(cur != NULL){
        //Found the job
        if(strcmp(cur->id, id) == 0){
            return cur->isRunning;
        }
        cur = cur->next;
    }
    
    return -1;      //Didn't find the job
}


//Set the isRunning = 1 for the first node of the queue
void setRunning(QUEUE *q) 
{
    //If the queue isn't empty
    if(!isEmpty(q)){
        q->front->isRunning = 1;
    }
}


//It is used for moving the jobs from the waiting queue into the main one + updates the positions
void move(QUEUE *source, QUEUE *destination) 
{
    //If the source queue isn't empty
    if(!isEmpty(source)) 
    {
        //Allocate memory for the new node
        NODE *n = (NODE *)malloc(sizeof(NODE));
        if(!n){printf("Memory allocation failed.\n");}

        //Copy the data from the first node of the source queue
        n->id = strdup(source->front->id);
        n->job = strdup(source->front->job);
        n->isRunning = source->front->isRunning;
        //If destination queue empty then pos = 1, else the position 
        //is ( 1 + the position of the end node of destination's queue )
        n->pos = (isEmpty(destination)) ? 1 : (destination->end->pos + 1);
        n->next = NULL;
        n->pid = source->front->pid;

        //The first node gets moved so -> Update the source queue
        NODE *temp = source->front;
        source->front = source->front->next;
        if(source->front == NULL){
            source->end = NULL;                 //Update end if source queue becomes empty
        }

        //Free the mem
        free(temp->id);
        free(temp->job);
        free(temp);

        //Update the positioning of the source queue
        temp = source->front;
        while(temp != NULL){
            temp->pos--;                        //Decrement the position of each node
            temp = temp->next;
        }

        //Update the destination queue
        if(isEmpty(destination)){
            destination->front = n;
            destination->end = n;
        }else{
            destination->end->next = n;
            destination->end = n;
        }
    }
}


//Find the specific job id and set isRunning=1
void set(QUEUE *q, char *jobID)
{
    //Traverse the queue
    NODE *cur = q->front;
    while(cur != NULL){
        if(strcmp(cur->id, jobID) == 0){
            cur->isRunning = 1;
            break;
        }
        cur = cur->next;
    }
}


//Get the first job of the queue
char* getFirstJob(QUEUE *q) 
{
    //If the queue isn't empty
    if(!isEmpty(q)){
        return q->front->job;
    }
}


//Returns 0/1 if we have/haven't N nodes -> It is used to check 
//if we have enough space for running more jobs concurrently
int reachedElements(QUEUE *q, int N) 
{
    int count = 0;

    //Traverse the queue and count the nodes
    NODE *cur = q->front;
    while(cur != NULL){
        count++;
        cur = cur->next;
    }

    //If we have N nodes
    if(count == N){
        return 0;           //0 if you have N nodes
    } 
    
    return 1;                  
}


//Set the pid for the specific job id in the queue
void setPid(QUEUE *q, char *id, pid_t pid) 
{
    //Traverse the queue
    NODE *cur = q->front;
    while (cur != NULL){
        if(strcmp(cur->id, id) ==0 ){
            cur->pid = pid;
            break;
        }
        cur = cur->next;
    }
}


//Removes from the running queue the job with a specific pid that just got terminated
void removeByPid(QUEUE *q, pid_t pid) 
{
    NODE *prev = NULL;
    NODE *cur = q->front;

    //Traverse the queue to find the node with the given ID
    while(cur != NULL && cur->pid != pid){
        prev = cur;
        cur = cur->next;
    }

    //If the node with the given ID is found
    if(cur != NULL)
    {
        deleteNode(q, prev, cur);   //Delete the node

        //Update the positioning of the queue after deletion
        cur = q->front;
        int pos = 1;
        while (cur != NULL) {
            cur->pos = pos;
            cur = cur->next;
            pos++;
        }
    }
}


//Returns 1/0 if there are less/more than N jobs
int lessThanN(QUEUE *q, int N)
{
    int count = 0;

    //Traverse the queue and count the nodes
    NODE *cur = q->front;
    while (cur != NULL) {
        count++;
        cur = cur->next;
    }

    if(count < N){
        return 1;       //1 if there are less than N jobs
    }

    return 0;           //0 if there are N or more jobs
}


//Returns the job id of the first node in the queue 
char* getFirstJobID(QUEUE *q) 
{
    //If the queue isn't empty
    if(!isEmpty(q)){
        return q->front->id;
    }
}


//Helpfull function for deletion -> Used by the stop(), stopvol() & removeByPid()
void deleteNode(QUEUE *q, NODE *prev, NODE *cur)
{
    //If the node to be deleted is the first node
    if(prev == NULL){
        q->front = cur->next;
    }
    else{
        prev->next = cur->next;
    }

    //If the node to be deleted is also the last node
    if(cur == q->end){
        q->end = prev;
    }

    //Free memory allocated for the node
    free(cur->id);
    free(cur->job);
    free(cur);
}