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
