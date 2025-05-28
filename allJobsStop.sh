#!/bin/bash

#Function for stoping a job by its ID
stop(){
    ./jobCommander stop "$1"
}

#Get the queued jobs from the poll command
queued=$(./jobCommander poll queued)

#Extract job IDs from queued jobs output and stop them
IFS=$'\n'
for job in $queued; do
    job_id=$(echo "$job" | cut -d ',' -f 1)
    stop "$job_id"
done

##Get the running jobs from the poll command
running=$(./jobCommander poll running)

#Extract job IDs from running jobs output and stop them
IFS=$'\n'
for job in $running; do
    job_id=$(echo "$job" | cut -d ',' -f 1)
    stop "$job_id"
done
