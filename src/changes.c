#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include "routes.h"
#include "changes.h"

int pid;
int pipefd1[2];
int pipefd2[2];

void get_changes();

void changes()
{
    int fd;

    int saved_stdout = dup(STDOUT_FILENO);

    FILE *file = fopen(changes_dir, "a");

    fd = fileno(file);

    dup2(fd, STDOUT_FILENO);
    close(fd);

    get_changes();

    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
}
//uses the functions find and awk to get the changes in the file,
//redirects the output to a file stored in the specified file
void get_changes()
{
    char data[4096];

    // pipe awk
    if (pipe(pipefd1) == -1)
    {
        perror("Error Init Pipe");
        exit(1);
    }

    // get find command
    if ((pid = fork()) == -1)
    {
        openlog("tracker", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes: Error find fork");
        closelog();

        perror("Error find fork");
        exit(1);
    }
    else if (pid == 0)
    {
        dup2(pipefd1[1], 1);

        close(pipefd1[0]);
        close(pipefd1[1]);

        // execute find
        execlp("find", "find", intranet_dir, "-cmin", "-0.017", "-type", "f", "-ls", NULL);

        // error check
        perror("Error with ls -al");
        exit(1);
    }

    // pipe awk and sort
    if (pipe(pipefd2) == -1)
    {
        perror("Error Init Pipe");
        exit(1);
    }

    // fork awk
    if ((pid = fork()) == -1)
    {
        openlog("tracker", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes: Error awk fork");
        closelog();

        perror("Error awk fork");
        exit(1);
    }
    else if (pid == 0)
    {
        // pipe 1
        dup2(pipefd1[0], 0);

        // pipe2
        dup2(pipefd2[1], 1);

        // close
        close(pipefd1[0]);
        close(pipefd1[1]);
        close(pipefd2[0]);
        close(pipefd2[1]);

        // execute awk
        execlp("awk", "awk", "{print $5,$8,$9,$10,$11}", NULL);

        // error check
        perror("bad exec grep root");
        exit(1);
    }

    // close fds
    close(pipefd1[0]);
    close(pipefd1[1]);

    // fork sort
    if ((pid = fork()) == -1)
    {
        openlog("tracker", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "changes: Error sort fork");
        closelog();

        perror("Error sort fork");
        exit(1);
    }
    else if (pid == 0)
    {
        // pipe 2
        dup2(pipefd2[0], 0);

        close(pipefd2[0]);
        close(pipefd2[1]);
        execlp("sort", "sort", "-u", NULL);

        // error check
        perror("Error with sort");
        exit(1);
    }
    else
    {
        close(pipefd2[1]);
        int nbytes = read(pipefd2[0], data, sizeof(data));
        printf("%.*s", nbytes, data);
        close(pipefd2[0]);
    }
}
