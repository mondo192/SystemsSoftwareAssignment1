CC = gcc 
CCFLAGS = -g -Wall
OBJS = main.o routes.o changes.o backup.o migrations.o queue.o
HDRS = main.c routes.h changes.h backup.h migrations.h queue.h

myprog : $(OBJS)
	$(CC) $(CLAGS) -o main $(OBJS) -lm -lrt

main.o : main.c $(HDRS)
	$(CC) $(CFLAGS) -c main.c -lrt
								
routes.o : routes.c
	$(CC) $(CFLAGS) -c routes.c -lm

changes.o : changes.c
	$(CC) $(CFLAGS) -c changes.c -lm
						
backup.o : backup.c
	$(CC) $(CFLAGS) -c backup.c -lm

migrations.o : migrations.c
	$(CC) $(CFLAGS) -c migrations.c -lm
				
queue.o : queue.c
	$(CC) $(CFLAGS) -c queue.c -lm

clean:
	rm  $(OBJS)
