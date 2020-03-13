#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#define QUEUE_NAME "queue"

int queue() {
  int pid = fork();

  if (pid > 0) {
  } else if (pid == 0) {

    if (setsid() < 0) {
      exit(EXIT_FAILURE);
    };

    umask(0);

    if (chdir("/") < 0) {
      exit(EXIT_FAILURE);
    };
    
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
      close(x);
    }
  
    mqd_t mq;
    struct mq_attr queue_attributes;
    char buffer[1024 + 1];
    int terminate = 0;

    queue_attributes.mq_flags = 1;
    queue_attributes.mq_maxmsg = 10;
    queue_attributes.mq_msgsize = 1024;
    queue_attributes.mq_curmsgs = 0;

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &queue_attributes);
    
    openlog("Change_tracker", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Started queue process");
    closelog(); 
    
    while(1) {
      ssize_t bytes_read;
      
      bytes_read = mq_receive(mq, buffer, 1024, NULL);
      
      buffer[bytes_read] = '\0';
      if (!strncmp(buffer, "exit", strlen("exit"))){
        terminate = 1;
      }
      else {
        openlog("Change_tracker", LOG_PID|LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "Recieved: %s\n", buffer);
        closelog(); 
      }
      sleep(1);
    } 
    

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
  }
  
  return 0;
}
