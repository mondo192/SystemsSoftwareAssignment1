#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "changes.h"
#include "routes.h"
#include "backup.h"
#include "transfer.h"
#include "queue.h"

//constructor is used to handle the time and when the backup should happen automaticlly
//signal handler waits for the user to issue a SIGUSR1 command.

void signal_handler(int sig_no);
void lock_dir();
void unlock_dir();


int main() {
  int pid = fork();

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  } else if (pid == 0) {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Started change tracker process");
    closelog();

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
  
    queue();
  
    time_t now;
    struct tm midnight;
    double seconds;
    time(&now);
    
    midnight = *localtime(&now);
    midnight.tm_hour = 0; 
    midnight.tm_min = 0; 
    midnight.tm_sec = 0;
    
    // add the signal handler
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
      openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
      syslog(LOG_INFO, "SIGINT catch error");
      closelog();
    }
    
    // adding sigusr1.
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
      openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
      syslog(LOG_INFO, "SIGUSR1 catch error");
      closelog();
    }
    
    while(1) {
      time(&now);
      seconds = difftime(now, mktime(&midnight));
      if (seconds == 0) {  
      //at midnight call the functions    
        lock_dir();
        backup();
        transfer();
        unlock_dir();
      } else {
        //otherwise run changes()
        changes();
      } 
      sleep(1);
    }
  }
  
  return 0;
}

    //call the functions when the user sends a SIGUSR signal
void signal_handler(int sig_no) {
  if (sig_no == SIGINT) {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "SIGINT interrupt recieved");
    closelog();
  } else if (sig_no == SIGUSR1) {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "SIGUSR1 interrupt recieved");
    closelog();
    //call functions
    lock_dir();
    backup();
    transfer();
    unlock_dir();
  }
}

//locks after update
void lock_dir() {
  char mode[4] = "0555";
  int i = strtol(mode, 0, 8);
  
  if (chmod(intranet_dir, i) == 0) {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Directory locked");
    closelog();
  } else {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Error locking directory");
    closelog();
  }
}
//unlocks directoy after update and backup is done.
void unlock_dir() {
  char mode[4] = "0777";
  int i = strtol(mode, 0, 8);
  if (chmod(intranet_dir, i) == 0) {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Directory unlocked");
    closelog();
  } else {
    openlog("Assignment1", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Error unlocking directory");
    closelog();
  }
}
