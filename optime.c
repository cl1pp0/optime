#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#define INTERVAL  1
#define SOCK_PATH "/tmp/optime_socket"

static int terminate = 0;
static int timecount = 0;
static int fd_sock, fd_conn;

void sig_handler(int sig)
{
    if (sig == SIGTERM)
    {
        syslog(LOG_NOTICE, "Received SIGTERM.");
        close(fd_sock);
        terminate = 1;
    }
    return;
}

static void daemonize()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, sig_handler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("optime", LOG_PID, LOG_DAEMON);
}

void thread_handler()
{
    timecount += INTERVAL;
    //syslog(LOG_NOTICE, "Timer event handled, timecount = %d", timecount);
}

void setup_timer(timer_t *timer_id)
{
    struct sigevent sigev;
    struct itimerspec timspec;

    memset(&sigev, 0, sizeof(struct sigevent));
    memset(&timspec, 0, sizeof(struct itimerspec));

    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_notify_function = &thread_handler;
    timer_create(CLOCK_REALTIME, &sigev, timer_id);

    timspec.it_interval.tv_sec = INTERVAL;
    timspec.it_value.tv_sec = INTERVAL;
    timer_settime(*timer_id, 0, &timspec, NULL);
}

int main()
{
    timer_t timer_id;
    struct sockaddr_un addr;
    socklen_t addrlen;
    int rc = EXIT_SUCCESS;

    daemonize();
    syslog (LOG_NOTICE, "Optime started.");

    setup_timer(&timer_id);
    syslog (LOG_NOTICE, "Timer started (interval %d s).", INTERVAL);

    if ((fd_sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        rc = EXIT_FAILURE;
        syslog (LOG_ERR, "Cannot create socket.");
        goto exit;
    }
    
    unlink(SOCK_PATH);
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);
    addrlen = sizeof(struct sockaddr_un);

    if (bind(fd_sock, (const struct sockaddr*)&addr, addrlen) < 0)
    {
        rc = EXIT_FAILURE;
        syslog (LOG_ERR, "Cannot bind socket.");
        goto exit;
    }

    if (listen(fd_sock, 3) < 0)
    {
        rc = EXIT_FAILURE;
        syslog (LOG_ERR, "Cannot listen on socket.");
        goto exit;
    }

    while (((fd_conn = accept(fd_sock, (struct sockaddr *)&addr, &addrlen)) >= 0) && !terminate)
    {
        write(fd_conn, (const void *)&timecount, sizeof(timecount));
        close(fd_conn);
    }

exit:
    close(fd_sock);
    unlink(SOCK_PATH);
    timer_delete(timer_id);
    syslog (LOG_NOTICE, "Optime terminated.");
    closelog();

    return rc;
}
