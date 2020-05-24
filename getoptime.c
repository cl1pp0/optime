#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SOCK_PATH "/tmp/optime_socket"

int main ()
{
    int timecount = -1;
    int fd_sock;
    struct sockaddr_un addr;
    socklen_t addrlen;
    int rc = EXIT_SUCCESS;
    
    if ((fd_sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket()");
        rc = EXIT_FAILURE;
        goto exit;
    }

    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);
    addrlen = sizeof(struct sockaddr_un);

    if (connect(fd_sock, (const struct sockaddr *)&addr, addrlen) < 0)
    {
        perror("Cannot connect() to daemon");
        rc = EXIT_FAILURE;
        goto exit;
    }

    if (read(fd_sock, (void *)&timecount, sizeof(timecount)) < 0)
    {
        perror("Cannot read() from socket");
        rc = EXIT_FAILURE;
        goto exit;
    }

    printf("%d:%02d\n", timecount/60, timecount%60);

exit:
    close(fd_sock);
    return rc;
}
