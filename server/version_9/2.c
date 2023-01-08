#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/socket.h>

// simplificat de forma (a + b) + (c + d) + ..
int main(int argc, char** argv) {
    
    pid_t pid;
    int skt[2];

    char expr[] = "(1 + 5) + (6 * 4)";
    // scanf("%s", expr);
    // printf("Expression is %s\n", expr);


    pid = fork();
    int globalResult = 0;

    for (int i = 0; i < strlen(expr); i++) {
            // printf("%s %d\n", "Here", i);

        if (expr[i] == '(') {
            int j = i + 1;

            while (j != ')' && j < strlen(expr)) { j++; )
    
            int result;
            socketpair(AF_UNIX, SOCK_STREAM, 0, skt);

            pid = fork();
            if (pid == 0) {
                close(skt[0]);
                
                int l, r;

                read(skt[1], &l, 4);
                read(skt[1], &r, 4);

                int a = expr[l + 1] - '0';
                int b = expr[l + 5] - '0';
                int type = (expr[l + 3] == '*');// 0 sum, 1 prod
                
                if (type == 1) {
                    a *= b;
                } else {
                    a += b;
                }

                write(skt[1], &a, 4);

                close(skt[1]);
                exit(1);
            } else {
                close(skt[1]);

                int res;

                write(skt[0], &i, 4);
                write(skt[0], &j, 4);
                
                read(skt[0], &res, 4);

                globalResult += res;
                close(skt[0]);
            }

        }

    }

    printf("%d \n", globalResult);

    return 0;
}




                switch(math_exp[j+1])
                    {
                        case '+':
                            finalRes += fromChildRes;
                            break;
                        case '-':
                            finalRes -= fromChildRes;
                            break;
                        case '*':
                            finalRes *= fromChildRes;
                            break;
                        case '/':
                            finalRes /= fromChildRes;
                            break;
                    }
                }