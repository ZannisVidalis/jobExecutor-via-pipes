#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

#define SERVER_FIFO "/tmp/2000022myfifo"
#define CLIENT_FIFO "/tmp/2000022myfifo2"


//SIGCONT handler
void wakeUp(int signum) {
    //Wake up the process
}


int main(int argc, char* argv[])
{
    int fd, fd2, value;
    char buf[PIPE_BUF];                      //buf -> For sending the command 
    char buf2[PIPE_BUF];                     //buf2 -> For receiving the answer (if needed) from server

    char* instruction = argv[1];         //Get the command that is given by the user


    //Checking if the command is valid
    if(strcmp(instruction, "setConcurrency")==0 || strcmp(instruction, "issueJob")==0 || strcmp(instruction, "stop")==0 || strcmp(instruction, "poll")==0 || strcmp(instruction, "exit")==0)
    {
        //The command is valid
    }
    else{
        fprintf(stderr, "You have to use: \n\t1) %s issueJob <executable>\n\t2) %s setConcurrency <Number>\n\t3) %s stop <job ID>\n\t4) %s poll <running/queued>\n\t5) %s exit\n", argv[0], argv[0], argv[0], argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }


    //CHECKING IF THE SERVER IS AVAILABLE
    int check = open("jobExecutorServer.txt", O_RDONLY);
    //The server does not exist since the .txt isnt created yet
    if(check == -1)
    {
        pid_t task = fork();
        //Fork failed
        if(task < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        //Child
        else if(task == 0)
        {
            //Redirect standard output and standard error to /dev/null
            int devNull = open("/dev/null", O_WRONLY);
            if(devNull == -1){
                perror("Failed to open /dev/null");
                exit(EXIT_FAILURE);
            }
            
            //Redirect output
            if(dup2(devNull, STDOUT_FILENO) == -1 || dup2(devNull, STDERR_FILENO) == -1){
                perror("Failed to redirect output to /dev/null");
                close(devNull);
                exit(EXIT_FAILURE);
            }

            close(devNull);


            execlp("./jobExecutorServer", "jobExecutorServer", NULL);
                               
            //If execlp() fails
            perror("Execution failed");
            exit(EXIT_FAILURE);
        }
        //Parent
        else
        {
            sleep(1);   //Give a second for the server to create and write in the .txt file first
        }
    }
    /* ============================== */
    

    //The SIGCONT handler
    signal(SIGCONT, wakeUp);


    // SET UP THE NAMED PIPES
    // -WRITING- Check if the named pipe already exists
    if(access(SERVER_FIFO, F_OK) == -1) {
        //Named pipe doesn't exist, create it
        if(mkfifo(SERVER_FIFO, 0664) == -1) {
            perror("mkfifo");
            return 1;
        }
    }

    // -WRITING- Open the pipe for writing
    fd = open(SERVER_FIFO, O_WRONLY);
    if(fd == -1){
        perror("open");
        return 1;
    }

    // -READING- making fifo
    if((mkfifo (CLIENT_FIFO, 0664) == -1) && (errno != EEXIST)) {
        perror ("mkfifo");
        exit(1);
    }

    // -READING- opening fifo
    if((fd2 = open (CLIENT_FIFO, O_RDONLY)) == -1){
        perror ("open");
    }
    /* ====================== */


    pid_t serverPid;   //For storing the servers pid which is read from the .txt file
    
    //RETRIEVING THE SERVER PID
    //Open the .txt to get the server's Pid
    int file;
    char buffer[100];

    file = open("jobExecutorServer.txt", O_RDONLY);
    if(file == -1){
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    //Read the first line from the file
    int res = read(file, buffer, sizeof(buffer));
    if(res == -1){
        perror("read");
        close(file);
        exit(EXIT_FAILURE);
    }

    close(file);                       //Close the file

    serverPid = atoi(buffer);          //Store the sever's pid
    /* ======================= */

    kill(serverPid, SIGCONT);          //Send a SIGCONT to the server for waking him up, in order to start reading the commanders commands
    memset(buf, 0, PIPE_BUF);          //Empty the array


    
    //Set up the message that will be sent to the server for each command
    if(strcmp(instruction, "setConcurrency")==0)
    {
        snprintf(buf, sizeof(buf), "%s %s", argv[1], argv[2]);      //Combine the command and value (N)
        write(fd, buf, sizeof(buf));
    }
    else if(strcmp(instruction, "issueJob")==0)
    {
        //Combine the command and unix command
        for(int i=1; i<argc-1; i++)
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s ", argv[i]);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s", argv[argc-1]);
        
        write(fd, buf, sizeof(buf));
        read(fd2, buf2, sizeof(buf2));
        printf("%s\n", buf2);
    }
    else if(strcmp(instruction, "stop")==0)
    {
        snprintf(buf, sizeof(buf), "%s %s", argv[1], argv[2]);      //Combine the command and job_ID
        write(fd, buf, sizeof(buf));
        read(fd2, buf2, sizeof(buf2));
        printf("%s\n", buf2);
    }
    else if(strcmp(instruction, "poll")==0)
    {
        snprintf(buf, sizeof(buf), "%s %s", argv[1], argv[2]);      //Combine the command and running/queued
        write(fd, buf, sizeof(buf));
        read(fd2, buf2, sizeof(buf2));
        printf("%s\n", buf2);
    }
    else if(strcmp(instruction, "exit")==0)
    {
        strcpy(buf, argv[1]);
        write(fd, buf, sizeof(buf));
        read(fd2, buf2, sizeof(buf2));
        printf("%s\n", buf2);
    }

    close(fd);

    printf("\n");

    return 0;
}