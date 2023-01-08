#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MSG1 "Comunicam prin sockets!"
#define MSG2 "Sockets-urile sunt o generalizare a pipe-urilor!"

int main()
{
    int sockp[2], child;
    char msg[1024];

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0)
    {
        printf("Err...socketpair");
        exit(1);
    }

    if(-1 == (child = fork()))
    {
        printf("Fork failed");
        exit(2);
    }
    else{
        if(child) //parinte
        {
            close(sockp[0]);
            if(read(sockp[1], msg, 1024) < 0){ printf("[Parent]Err...read"); exit(4); }
            printf("[Parinte] %s\n", msg);
            if(write(sockp[1], MSG2, sizeof(MSG2)) < 0) { printf("[Parent]Err...write"); exit(5); }
            close(sockp[1]);
        }
        else
        {
            close(sockp[1]);
            if(write(sockp[0], MSG1, sizeof(MSG1)) < 0){ printf("[Child]Err...write"); exit(6);}
            if(read(sockp[0], msg, 1024) < 0) { printf("[Child]Err...read"); exit(7); }
            printf("[Child] %s\n", msg);
            close(sockp[0]);
        }
    }
}