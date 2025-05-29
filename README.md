# jobExecutor-via-pipes

## Overview
This project is an implementation of a job scheduling and execution system using named pipes and signals. It consists of two main components:
* jobExecutorServer: Manages, schedules and executes jobs
* jobCommander: Sends user commands to the server

Jobs are submitted by the user via commands and executed according to the concurrency level, using named pipes and signals for communication.

# Compilation & Execution
To compile the project run: make
### Mode 1 (Single Terminal):
Run jobCommander directly with a command (e.g ./jobCommander issueJob ls -l). If the server is not running, it will automatically start in the background. The server is launched via exec, and command output will not be displayed.

### Mode 2 (Two Terminals):
In Terminal 1, start the server manually: ./jobExecutorServer. In terminal 2 send commands using jobCommander. This mode allows real-time output to appear in the terminal running the server.

# jobExecutorServer
The jobExecutorServer writes its process ID (PID) to a .txt file upon startup, allowing other processes (like jobCommander) to detect whether the server is active.

Its core logic is based on an infinite loop that remains in a paused state (pause()), resuming execution only when it receives a SIGCONT signal from a jobCommander. Upon receiving the signal, it reads the incoming command from the pipe and processes it accordingly.

Additionally, when the server receives a SIGCHLD signal—indicating that a previously launched job has terminated—it removes that job from the running queue using its PID. It then checks whether there is available space in the running queue and, if so, promotes the next job from the waiting queue to execution.

# jobCommander
When executed, jobCommander first checks whether the server (jobExecutorServer) is running. If the server is not active, jobCommander launches it using exec.

It then reads the .txt file created by the server to obtain its process ID (PID).

Before sending a command through the pipe, jobCommander sends a SIGCONT signal to the server. This signal notifies the server that a new command is about to be transmitted.

### Commands
1. issueJob <command>: Submits a job. It will be placed in either the running queue (if slots are available) or the waiting queue. Each job receives a unique jobID (e.g., job_1).
2. setConcurrency <N>: Sets the maximum number of concurrent jobs. If increased, jobs are moved from the waiting queue to execution. If decreased, the change is applied only if the running queue is empty.
3. stop <jobID>: Stops a running job or removes it from the waiting queue
4. poll running / poll queued: Lists all currently running or queued jobs with their jobID, original command, and queue position.
5. exit: Shuts down the server, deletes the jobExecutorServer.txt file, and closes named pipes.
