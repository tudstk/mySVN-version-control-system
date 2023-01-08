#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define SIR1 "Sunt primul sir, "
#define SIR2 "iar eu sunt sirul 2 si m-am concatenat!"

int main(){

    int sockp[2], child;
    char msg[1024], msg1[1024], msg2[1024];

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) { perror("Socketpair error"); }
    if(-1 == (child == fork())) { perror("Fork error"); }
    else
    {
        if(child) //parinte
        {
            
            write(sockp[0], SIR1, sizeof(SIR1));
            write(sockp[1], SIR2, sizeof(SIR2));
            close(sockp[0]);
            close(sockp[1]);
        }
        else // copil
        {
            read(sockp[0], msg, 1024);
            strcpy(msg1,msg);
            read(sockp[1], msg, 1024);
            strcpy(msg2, msg);
            printf("Sirul final este:%s\n", strcat(msg1, msg2));
            close(sockp[1]);
            close(sockp[0]);
        }
    }
    return 0;
}