OBJS	= jobCommander.o
OBJS_S  = jobExecutorServer.o queue.o
SOURCE	= jobCommander.c jobExecutorServer.c queue.c
OUT 	= jobCommander
OUT_S   = jobExecutorServer
CC	    = gcc
FLAGS   = -g -c


all: $(OUT_S) $(OUT)

$(OUT_S): $(OBJS_S)
	$(CC) -g $(OBJS_S) -o $(OUT_S)

$(OUT): $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

#separate compilation
jobCommander.o: jobCommander.c
	$(CC) $(FLAGS) jobCommander.c

jobExecutorServer.o: jobExecutorServer.c
	$(CC) $(FLAGS) jobExecutorServer.c

queue.o: queue.c
	$(CC) $(FLAGS) queue.c

clean:
	rm -f $(OUT_S) $(OBJS_S) $(OUT) $(OBJS)