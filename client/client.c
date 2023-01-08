#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>

extern int errno;

int port;

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char msg[100];             // mesajul trimis

  if (argc != 3)
  {
    printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  port = atoi(argv[2]);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[client] Eroare la socket().\n");
    return errno;
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);

  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  int bytesRead;
  while (true)
  {
    memset(msg, 0, 100);
    printf("[Client] Welcome! Please log in or sign up to continue.\n\nYou have access to the following commands:\n--- login\n--- signup\n--- login as admin\n--- exit\n\nType a command...\n\n");
    fflush(stdout);

    if (-1 == (bytesRead = read(0, msg, 100)))
    {
      perror("[Client] Failed to 'read' from client input!..");
      exit(4);
    }

    msg[strlen(msg) - 1] = '\0';
    if (write(sd, msg, 100) <= 0)
    {
      perror("[client]Eroare la write() spre  69.\n");
      return errno;
    }

    if (strcmp(msg, "login") == 0)
    {
      char credentials[200], askUsername[100], askPasswd[100];
      memset(credentials, 0, 200);
      memset(askUsername, 0, 100);
      memset(askPasswd, 0, 100);

      // ask for username
      printf("Username:\n");
      read(0, askUsername, 100);
      askUsername[strlen(askUsername) - 1] = '\0';

      // add username to final message
      strcpy(credentials, askUsername);
      strcat(credentials, ":");

      // ask for password
      printf("Password:\n");
      read(0, askPasswd, 100);
      askPasswd[strlen(askPasswd) - 1] = '\0';

      // add password to final message
      strcat(credentials, askPasswd);
      printf("credentials:%s", credentials);

      if (write(sd, credentials, 200) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }

      // receive the login status

      char loginStatus[100];
      memset(loginStatus, 0, 100);
      if (read(sd, loginStatus, 100) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }
      printf("\n\n%s\n", loginStatus);
      if (strstr(loginStatus, "You are logged in!\n\nWelcome, "))
      {
        printf("\nYou have access to the following commands:\n--- clone\n--- add\n--- delete\n--- commit\n--- push\n--- revert\n");
        char accountCommand[100];
        memset(accountCommand, 0, 100);
        while (true)
        {
          printf("\nType a command...\n");
          read(0, accountCommand, 100);
          accountCommand[strlen(accountCommand) - 1] = '\0';
          // printf("account command: %s, STRLEN:%d\n", accountCommand, strlen(accountCommand));

          if (write(sd, accountCommand, 100) <= 0)
          {
            perror("[Server] Eroare la read() in server (1).\n");
          }
          if (strstr(accountCommand, "clone"))
          {
            char alreadyCloned[100];
            memset(alreadyCloned, 0, 100);
            if (read(sd, alreadyCloned, 100) < 0)
            {
              perror("[client]Eroare la read() in client.\n");
            }

            if (strstr(alreadyCloned, "You have already cloned this repository on your local machine!\n"))
            {
              printf("\n%s\n", alreadyCloned);
            }
            else
            {
              char repoLocation[200];
              memset(repoLocation, 0, 200);
              printf("Type local repository location:\n");
              read(0, repoLocation, 200);
              repoLocation[strlen(repoLocation) - 1] = '\0';
              if (write(sd, repoLocation, 200) <= 0)
              {
                perror("[Server] Eroare la read() in server (1).\n");
              }

              char repoName[200];
              memset(repoName, 0, 200);
              printf("Type local repository name:\n");
              read(0, repoName, 200);
              repoName[strlen(repoName) - 1] = '\0';
              if (write(sd, repoName, 200) <= 0)
              {
                perror("[Server] Eroare la read() in server (1).\n");
              }

              char cloneStatus[200];
              memset(cloneStatus, 0, 200);
              if (read(sd, cloneStatus, 200) <= 0)
              {
                perror("[Client] Eroare la read in client.\n");
              }
              printf("%s\n", cloneStatus);
            }
          }
          else if (strcmp(accountCommand, "list") == 0)
          {
            char receivedEntity[200];
            memset(receivedEntity, 0, 200);
            int bRead;
            while (bRead = read(sd, receivedEntity, sizeof(receivedEntity)) > 0)
            {
              // perror("[Server] Entity error\n");
              printf("%s\n", receivedEntity);
            }
            printf("am terminat");
          }
          else if (strstr(accountCommand, "add"))
          {
            char fileToAdd[200];
            memset(fileToAdd, 0, 200);
            printf("Your file absolute path:\n");
            read(0, fileToAdd, 200);
            fileToAdd[strlen(fileToAdd) - 1] = '\0';
            if (write(sd, fileToAdd, 200) <= 0)
            {
              perror("[Client] Eroare la write() catre server.\n");
            }
            // char localRepoName[200];
            // memset(localRepoName, 0, 200);
            // printf("Your local repo absolute path:\n");
            // read(0, localRepoName, 200);
            // if (write(sd, localRepoName, 200) <= 0)
            // {
            //   perror("[Client] Eroare la write() catre server.\n");
            // }

            char addStatus[100];
            memset(addStatus, 0, 100);
            if (read(sd, addStatus, 100) <= 0)
            {
              perror("[Client] Eroare la read in client.\n");
            }
            printf("\n\n%s\n", addStatus);
          }
          else if (strcmp(accountCommand, "delete") == 0)
          {
            char path_toDelete[1000];
            memset(path_toDelete, 0, 1000);
            printf("Name of the file/directory that you want to delete:\n");
            read(0, path_toDelete, 200);
            if (write(sd, path_toDelete, 200) <= 0)
            {
              perror("[Server] Eroare la read() in server.\n");
            }

            char deleteStatus[100];
            memset(deleteStatus, 0, 100);
            if (read(sd, deleteStatus, 100) <= 0)
            {
              perror("[Client] Eroare la read in client.\n");
            }
            printf("\n\n%s\n", deleteStatus);
          }
          else if (strstr(accountCommand, "exit"))
          {
            // char exitSignal[50];
            // memset(exitSignal, 0, 50);
            // strcpy(exitSignal, "semnal");
            // if (write(sd, exitSignal, strlen(exitSignal)) <= 0)
            // {
            //   perror("[Client] Eroare la write() catre server.\n");
            // }
            // char ack[50];
            // memset(ack, 0, 50);
            // if (read(sd, ack, 50) <= 0)
            // {
            //   perror("[Client] Eroare la read in client.\n");
            // }
            exit(1);
          }
          else if (strstr(accountCommand, "logout"))
          {
            printf("\nYou are logged out.\n");
            break;
          }
          else if (strstr(accountCommand, "login"))
          {
            printf("\nYou are already logged in!\n");
          }
          else if (strstr(accountCommand, "signup"))
          {
            printf("\nSignup function is available only while you are logged out.\n");
          }
          else if (strstr(accountCommand, "commit"))
          {
            char commitStatus[100];
            memset(commitStatus, 0, 100);
            int addedFiles, deletedFiles, changedFiles;
            if (read(sd, commitStatus, sizeof(commitStatus)) <= 0)
            {
              perror("[Client] Eroare la read().\n");
            }
            commitStatus[strlen(commitStatus)] = '\0';

            if (strstr(commitStatus, "Commited successfully!"))
            {
              printf("%s\n", commitStatus);
              if (read(sd, &addedFiles, sizeof(addedFiles)) <= 0)
              {
                perror("[Client] Eroare la read() - added.\n");
              }
              printf("(+) Added files:%d\n", addedFiles);
              if (read(sd, &deletedFiles, sizeof(deletedFiles)) <= 0)
              {
                perror("[Client] Eroare la read() - deleted.\n");
              }
              printf("(-) Deleted files:%d\n", deletedFiles);
              if (read(sd, &changedFiles, sizeof(changedFiles)) <= 0)
              {
                perror("[Client] Eroare la read() - changed.\n");
              }
              printf("(*) Changed files:%d\n", changedFiles);
            }
            else
            {
              printf("%s\n", commitStatus);
            }
          }
          else if (strstr(accountCommand, "check"))
          {
            printf("Ok check..\n");
          }
          else if (strstr(accountCommand, "push"))
          {
            char pushMessage[100];
            memset(pushMessage, 0, 100);
            if (read(sd, pushMessage, sizeof(pushMessage)) <= 0)
            {
              perror("[Client] failed to read push message!\n");
            }
            pushMessage[strlen(pushMessage)] = '\0';
            printf("\n\n%s\n", pushMessage);
          }
          else if (strstr(accountCommand, "revert"))
          {
            char revertMessage[100];
            memset(revertMessage, 0, 100);
            if (read(sd, revertMessage, sizeof(revertMessage)) <= 0)
            {
              perror("[Client] failed to read revert message!\n");
            }
            printf("\n%s\n", revertMessage);
          }
          else
          {
            printf("This command is not available!\n");
          }
        }
      }
    }
    else if (strcmp(msg, "signup") == 0)
    {
      char credentials[200], askUsername[100], askPasswd[100], askConfPasswd[100];
      memset(credentials, 0, 200);
      memset(askUsername, 0, 100);
      memset(askPasswd, 0, 100);
      memset(askConfPasswd, 0, 100);

      printf("Username:\n");
      read(0, askUsername, 100);
      askUsername[strlen(askUsername) - 1] = '\0';

      // add username to final message
      strcpy(credentials, askUsername);
      strcat(credentials, ":");

      // ask for password
      printf("Password:\n");
      read(0, askPasswd, 100);
      askPasswd[strlen(askPasswd) - 1] = '\0';

      // add password to final message
      strcat(credentials, askPasswd);
      strcat(credentials, ":");

      // ask for password confirmation
      printf("Confirm password:\n");
      read(0, askConfPasswd, 100);
      askConfPasswd[strlen(askConfPasswd) - 1] = '\0';

      // add password confirmation to final message
      strcat(credentials, askConfPasswd);

      if (write(sd, credentials, 200) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }

      char signupStatus[100];
      memset(signupStatus, 0, 100);
      if (read(sd, signupStatus, 100) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }
      printf("\n\n%s\n", signupStatus);
    }
    else if (strcmp(msg, "exit") == 0)
    {
      printf("\nGoodbye!\n\n");
      exit(2);
    }
    else if (strstr(msg, "login as admin"))
    {
      char credentials[200], askUsername[100], askPasswd[100];
      memset(credentials, 0, 200);
      memset(askUsername, 0, 100);
      memset(askPasswd, 0, 100);

      // ask for username
      printf("Username:\n");
      read(0, askUsername, 100);
      askUsername[strlen(askUsername) - 1] = '\0';

      // add username to final message
      strcpy(credentials, askUsername);
      strcat(credentials, ":");

      // ask for password
      printf("Password:\n");
      read(0, askPasswd, 100);
      askPasswd[strlen(askPasswd) - 1] = '\0';

      // add password to final message
      strcat(credentials, askPasswd);
      printf("credentials:%s", credentials);

      if (write(sd, credentials, 200) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }
      char loginStatus[100];
      memset(loginStatus, 0, 100);
      if (read(sd, loginStatus, 100) < 0)
      {
        perror("[client]Eroare la write() catre server.\n");
        return errno;
      }
      printf("\n\n%s\n", loginStatus);
      if (strstr(loginStatus, "Logged in succesfully!"))
      {
        printf("You have access to the following commands:\n--- Change permission\n--- logout\n\n");
        while (true)
        {
          printf("Type a command...\n\n");
          char adminCommand[100];
          memset(adminCommand, 0, 100);
          read(0, adminCommand, 100);
          adminCommand[strlen(adminCommand) - 1] = '\n';
          if (write(sd, adminCommand, strlen(adminCommand)) < 0)
          {
            perror("[client]Eroare la write() catre server.\n");
            return errno;
          }
          if (strstr(adminCommand, "change permission"))
          {
            char userToChange[100];
            memset(userToChange, 0, 100);
            printf("Username to change permission:\n");
            read(0, userToChange, 100);
            userToChange[strlen(userToChange) - 1] = '\n';
            if (write(sd, userToChange, strlen(userToChange)) < 0)
            {
              perror("[client]Eroare la write() catre server.\n");
              return errno;
            }
            char permStatusMessage[100];
            memset(permStatusMessage, 0, 100);
            if (read(sd, permStatusMessage, 100) < 0)
            {
              perror("[client]Eroare la read() in client.\n");
            }
            permStatusMessage[strlen(permStatusMessage)] = '\0';
            printf("\n%s\n\n", permStatusMessage);
          }
          else if (strstr(adminCommand, "logout"))
          {
            printf("\n\nAdmin logged out.\n\n");
            break;
          }
        }
      }
      else
      {
        printf("Check username/password and try again!\n");
      }
    }
    else
    {
      printf("This command is not available.\n\n");
    }
  }
  close(sd);
}
