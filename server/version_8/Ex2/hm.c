#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

int main()
{
    int finalRes = 0, sockp[2], child;
    bool isFirst = true;
    char math_exp[] = "(8 + 2)+(6 * 3)+(9 / 3)+(4 - 2)";

    for(int i = 0; i < strlen(math_exp); i++)
    {
        if(math_exp[i] == '(')
        {
            int j = i + 1;
            while(math_exp[j] != ')' && j < strlen(math_exp))
                j++;
            
            if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) { perror("[Socket] Err...socketpair"); }
            if(-1 == (child = fork())) { perror("[Fork] Err...fork"); }

            if(child) //parent
            {
                
                int fromChildRes;
                close(sockp[1]);
                if(write(sockp[0], &i, sizeof(i)) < 0) { perror("[Parent] Err...i writing"); }
                if(write(sockp[0], &j, sizeof(j)) < 0) { perror("[Parent] Err...j writing"); }
                if(read(sockp[0], &fromChildRes, sizeof(fromChildRes)) < 0) { perror("[Child] Err...first read"); }
                close(sockp[0]);
                
                if(isFirst)
                {
                    finalRes += fromChildRes;
                    isFirst = false;
                     printf("Current result is: %d and j is: %d\n", finalRes, j);
                }
                else
                {
                    if(math_exp[j+1] == '+') finalRes += fromChildRes;
                    else if(math_exp[j+1] == '-') finalRes -= fromChildRes;
                    else if(math_exp[j+1] == '*') finalRes *= fromChildRes;
                    else if(math_exp[j+1] == '/') finalRes /= fromChildRes;
                    printf("Current result is: %d and j is: %d\n", finalRes, j);

                }
                // printf("Now finalRes is: %d\n", finalRes);
            }
            else //child
            {
                int left, right, localRes;

                close(sockp[0]);
                if(read(sockp[1], &left, sizeof(left)) < 0) { perror("[Child] Err...first read"); }
                if(read(sockp[1], &right, sizeof(right)) < 0) { perror("[Child] Err...second read"); }

                // printf("left is %d and right is %d\n", left, right);

                int a = math_exp[left + 1] - '0';
                int b = math_exp[right - 1] - '0';

                // printf("a is %d and b is %d\n", a, b);

                switch(math_exp[left+3])
                {
                    case '+':
                        localRes = a + b;
                        break;
                    case '-':
                        localRes = a - b;
                        break;
                    case '*':
                        localRes = a * b;
                        break;
                    case '/':
                        localRes = a / b;
                        break;
                }
                printf("localRes is %d\n", localRes);

                if(write(sockp[1], &localRes, sizeof(localRes)) < 0) { perror("[Child] Err...localRes write"); }
                close(sockp[1]);
                exit(1);
            }

        }

    }
    printf("Your result is: %d\n", finalRes);

    return 0;
}
