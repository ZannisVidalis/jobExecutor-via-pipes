#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <limits.h>
#include "queue.h"

#define SERVER_FIFO "/tmp/2000022myfifo"
#define CLIENT_FIFO "/tmp/2000022myfifo2"

QUEUE q;
QUEUE waiting;

pid_t pid;
int N = 1;
char waitJob[PIPE_BUF], buf2[PIPE_BUF];
char *Args[100];         //For storing the command's arguments


//SIGCONT handler
void wakeUp(int signum) {
    //Wake up the process
}

//SIGCHLD handler
void childHandler(int signum) 
{
    //Handle SIGCHLD signal
    if(signum == SIGCHLD) 
    {
        printf("\n\nSomeone Died...\n");
        pid_t term;
        int status;

        //If you catch a terminated process remove it from the running queue
        while((term = waitpid(-1, &status, WNOHANG)) > 0){
            if(WIFEXITED(status)) {
                removeByPid(&q, term);
            }
        }

        //Get the next task from the waiting queue and execute it if space is available in the main queue
        while(!isEmpty(&waiting) && lessThanN(&q, N)) 
        {
            //Empty the array
            for(int j=0; j<100; j++){
                Args[j] = NULL;
            }

            pid = fork();

            //Fork failed
            if(pid < 0) 
            { 
                perror("Fork failed");
                exit(EXIT_FAILURE);
            }
            //Child process
            else if(pid == 0) 
            { 
                char* command;
                command = getFirstJob(&waiting);        //Get the first job of the waiting queue for execution
                strcpy(buf2, command);

                //Tokenize each argument
                command = strtok(buf2, " ");
                int handlerCount=0;
                while(command != NULL){
                    Args[handlerCount++] = command;
                    command = strtok(NULL, " ");
                }

                execvp(Args[0], Args);    //Execute the command
                
                //If execvp() fails
                perror("Execution failed");
                exit(EXIT_FAILURE);
            }
            //Parent process
            else 
            {
                char* tag;
                tag = getFirstJobID(&waiting);
                strcpy(waitJob, tag); 
                
                setPid(&waiting, waitJob, pid);                //Set the pid for job

                setRunning(&waiting);                          //Update the running status of the task in the queue
                move(&waiting, &q);                            //Move the job from the waiting queue into the main one
            }
        }
    }
}



int main(int argc, char* argv[])
{
    int fd, fd2, res, i=1;
    char buf[PIPE_BUF];             //For receiving from client
    char com[30];                   //For storing the command
    char tmp[PIPE_BUF], tmp2[PIPE_BUF], tmp3[PIPE_BUF];
    char *token, *a;

    char *args[100];    //For storing the command's arguments
    
    //Initialize the queues
    init(&q);
    init(&waiting);
 

    //The SIGCHLD handler
    signal(SIGCHLD, childHandler);

    //The SIGCONT handler
    signal(SIGCONT, wakeUp);


    //Open the .txt file and write
    pid_t p = getpid();

    int file = open("jobExecutorServer.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(file == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }

    int length = snprintf(tmp, sizeof(tmp), "%d\n", (int)p);

    write(file, tmp, length);

    close(file);

    /* ====================== */

    
    // SET UP THE NAMED PIPES
    // -READING- making fifo
    if((mkfifo (SERVER_FIFO, 0664) == -1) && (errno != EEXIST)) {
        perror ("mkfifo");
        exit(EXIT_FAILURE);
    }

    // -READING- opening fifo
    if((fd = open (SERVER_FIFO, O_RDONLY)) == -1){
        perror ("open");
    }

    // -WRITING- Check if the named pipe already exists
    if(access(CLIENT_FIFO, F_OK) == -1) {
        //Named pipe doesn't exist, create it
        if(mkfifo(CLIENT_FIFO, 0664) == -1) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    // -WRITING- Open the pipe for writing
    fd2 = open(CLIENT_FIFO, O_WRONLY);
    if(fd2 == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    /* ====================== */



    while(1)
    {
        pause();    //Pause the server till he receives a SIGCONT signal from a commander

        res = read(fd, buf, sizeof(buf));

        if(res>0)
        {
            token = strtok(buf, " ");           //Extract the command
            strcpy(com, token);                 //and store it in com array

            if(strcmp(com, "setConcurrency")==0)
            {
                int Nold = N;
                token = strtok(NULL, " ");      //Extract the value
                N = atoi(token);                //and store it

                //Non valid concurrency -> Set it to 1
                if(N<=0){
                    N = 1;
                }

                //Case where you gave bigger Concurrency than the previous one:
                //If there are queud jobs and jave space in main queue -> then start executing them
                if(Nold < N)
                {
                    //Same code as the one in the SIGCHLD handler
                    while(!isEmpty(&waiting) && lessThanN(&q, N)) 
                    {
                        //Empty the array
                        for(int j=0; j<100; j++){
                            Args[j] = NULL;
                        }

                        pid = fork();

                        //Fork failed
                        if(pid < 0) 
                        { 
                            perror("Fork failed");
                            exit(EXIT_FAILURE);
                        }
                        //Child process
                        else if(pid == 0) 
                        { 
                            char* command;
                            command = getFirstJob(&waiting);        //Get the first job of the waiting queue for execution
                            strcpy(buf2, command);

                            //Tokenize each argument
                            command = strtok(buf2, " ");
                            int sum=0;
                            while(command != NULL){
                                Args[sum++] = command;
                                command = strtok(NULL, " ");
                            }

                            execvp(Args[0], Args);    //Execute the command
                            
                            //If execlp() fails
                            perror("Execution failed");
                            exit(EXIT_FAILURE);
                        }
                        //Parent process
                        else 
                        {
                            char* tag;
                            tag = getFirstJobID(&waiting);
                            strcpy(waitJob, tag); 
                            
                            setPid(&waiting, waitJob, pid);                //Set the pid for job

                            setRunning(&waiting);                          //Update the running status of the task in the queue
                            move(&waiting, &q);                            //Move the job from the waiting queue into the main one
                        }
                    }
                }
                //Case where you gave smaller Concurrency than the previous one:
                else if(Nold > N)
                {
                    //Ιf the main queue isnt empty then keep the previus N
                    if(!isEmpty(&q)){
                        N = Nold;
                    }
                }

            }
            else if(strcmp(com, "issueJob")==0)
            {
                //Empty the array
                for(int j=0; j<100; j++){
                    args[j] = NULL;
                }

                token = strtok(NULL, "");       //Extract the unix instruction
                strcpy(tmp, token);             //Store it in a temp string

                //Tokenize each argument
                strcpy(tmp3, token);
                token = strtok(tmp3, " ");
                int count=0;
                while(token != NULL){
                    args[count++] = token;
                    token = strtok(NULL, " ");
                }
                
                sprintf(tmp2, "job_%d", i);     //Concatenate "job_" with the integer i to create job ID
                i++;

                //Add the job into the main queue for execution, for the next cases:
                //1. If the queue is empty  -> No jobs running
                //2. If we have enough space for running more jobs concurrently
                if(isEmpty(&q) || reachedElements(&q, N))
                {
                    add(&q, tmp2, tmp);             //Add the command to the queue
                    a = getInfo(&q);                //Get the triplet <jobID,job,queuePosition>
                    strcpy(tmp, a); 

                    write(fd2, tmp, sizeof(tmp));
                
                    p = fork();

                    //Fork failed
                    if(p < 0)
                    {
                        perror("Fork failed");
                        exit(EXIT_FAILURE);
                    }
                    //Child
                    else if(p == 0)
                    {
                        execvp(args[0], args);          //Execute the command
                        
                        //If execvp() fails
                        perror("Execution failed");
                        exit(EXIT_FAILURE);
                    }
                    //Parent
                    else
                    {
                        setPid(&q, tmp2, p);              //Set the PID for the specific job 
                        set(&q, tmp2);                    //Set isRunning = 1 for the specific job
                        continue;       
                    }
                }
                //The space for the executing jobs in the main queue is full, so put the new job in the waiting queue
                else{
                    add(&waiting, tmp2, tmp);
                    a = getInfo(&waiting);                //Get the triplet  <jobID,job,queuePosition>
                    strcpy(tmp, a);
                    write(fd2, tmp, sizeof(tmp));
                }


            }
            else if(strcmp(com, "stop")==0)
            {
                token = strtok(NULL, "");       //Extract the job_ID
                strcpy(tmp, token);             //Store it
                strcpy(tmp2, tmp);

                //SETTING UP THE MESSAGE FOR THE COMMANDER
                //Check if the specific job is at running state
                if(getIsRunning(&q, tmp)==1){
                    strcat(tmp2, " terminated.");      //Prepare the corresponding message
                    write(fd2, tmp2, sizeof(tmp2));    //And sent it
                }
                //or is it in waiting state
                else if(getIsRunning(&waiting, tmp)==0){
                    strcat(tmp2, " removed.");         //Prepare the corresponding message
                    write(fd2, tmp2, sizeof(tmp2));    //And sent it
                }
                else{
                    strcat(tmp2, " was not found in the queue.");   //Prepare the corresponding message
                    write(fd2, tmp2, sizeof(tmp2));                 //And sent it
                    continue;                          //Continue -> no need to try stoping something that isnt in any of the queue's
                }

                //WHEN TO APPLY THE STOP LOGIC
                //If the main queue isnt empty and the specific job is running then terminate her
                if(!isEmpty(&q) && getIsRunning(&q, tmp)){
                    stop(&q, tmp, p);           //Stop the desired job
                }

                //If the waiting queue isnt empty and the specific job is waiting then remove her
                if(!isEmpty(&waiting) && getIsRunning(&waiting, tmp)==0){
                    stopvol(&waiting, tmp);     //For removing non executing jobs
                }
                    
            }
            else if(strcmp(com, "poll")==0)
            {
                token = strtok(NULL, "");       //Extract the running/queued
                strcpy(tmp, token);             //Store it

                //Case where we want to print the running jobs
                if(strcmp(tmp, "running")==0){
                    a = getRunningJobs(&q);
                    strcpy(tmp2, a);
                    write(fd2, tmp2, sizeof(tmp2));
                }
                //Case where we want to print the queued jobs
                else{
                    a = getQueuedJobs(&waiting);
                    strcpy(tmp2, a);
                    write(fd2, tmp2, sizeof(tmp2));
                }

            }
            else if(strcmp(com, "exit")==0)
            {
                strcpy(tmp, "jobExecutorServer terminated.");
                write(fd2, tmp, sizeof(tmp));                   //Sent the related message

                //Delete the .txt file
                if(unlink("jobExecutorServer.txt") == -1){
                    perror("unlink");
                    exit(EXIT_FAILURE);
                }

                //Close the pipe descriptors
                close(fd);
                close(fd2);

                break;
            }

        }

    }

    return 0;
}