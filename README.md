# jobExecutor-via-pipes

## Overview
This project is an implementation of a job scheduling and execution system using named pipes and signals. It consists of two main components:
* jobExecutorServer: Manages, schedules and executes jobs
* jobCommander: Sends user commands to the server
Jobs are submitted by the user via commands and executed according to the concurrency level, using named pipes and signals for communication.
