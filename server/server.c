#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sqlite3.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

#define PORT 2024

typedef struct thData
{
    int idThread;
    int cl;
} thData;

static void *treat(void *);
void raspunde(void *);

int deleteFile(char *filePath)
{
    int result = remove(filePath);
    char *fileName = strrchr(filePath, '/');
    if (result == 0)
    {
        printf("[Server] File '%s' was deleted successfully!\n", fileName + 1);
        return 0;
    }
    else
    {
        printf("[Server] Error deleting file: '%s'.\n", filePath);
        return -1;
    }
}

int deleteDirectory(char *dirPath)
{
    DIR *dir = opendir(dirPath);
    if (dir == NULL)
    {
        printf("[Server] Failed to open directory %s\n", dirPath);
        return -1;
    }
    char *entityName = strrchr(dirPath, '/');
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            char subpath[1024];
            strcpy(subpath, dirPath);
            strcat(subpath, "/");
            strcat(subpath, entry->d_name);
            if (entry->d_type == DT_DIR)
            {
                int result = deleteDirectory(subpath);
                if (result != 0)
                {
                    closedir(dir);
                    return result;
                }
            }
            else
            {
                // remove returneaza 0 pt succes si -1 pt eroare
                int result = remove(subpath);
                if (result != 0)
                {
                    printf("[Server] Failed to delete file %s.\n", subpath);
                    closedir(dir);
                    return result;
                }
            }
        }
    }

    closedir(dir);
    int result = rmdir(dirPath);
    if (result != 0)
    {
        printf("[Server] Failed to delete directory %s.\n", dirPath);
        return result;
    }
    return 0;
}

int deleteEntity(char *path)
{
    // folosesc stat aici in loc de dirent pentru ca trebuie sa pot deschide si path-ul unui fisier
    // d_type din dirent poate fi folosit abia dupa ce am deschis un fisier..prin urmare folosesc stat
    struct stat st;
    if (stat(path, &st) != 0)
    {
        printf("Error checking path '%s'.\n", path);
        return -1;
    }
    if (S_ISREG(st.st_mode))
    {
        return deleteFile(path);
    }
    else if (S_ISDIR(st.st_mode))
    {
        return deleteDirectory(path);
    }
    else
    {
        printf("[Server] Nu e nici file nici fisier, probabil e ceva corupt/inexistent. '%s'.\n", path);
        return -1;
    }
}

int copyFile(char *sourcePath, const char *destinationPath)
{
    FILE *sourceFile;
    FILE *destinationFile;
    char *fileName = strrchr(sourcePath, '/');
    int c;
    sourceFile = fopen(sourcePath, "r");
    if (sourceFile == NULL)
    {
        printf("[Server] Error opening source file (in copyFile function):%s\n.", sourcePath);
        return 1;
    }
    destinationFile = fopen(destinationPath, "w");
    if (destinationFile == NULL)
    {
        printf("[Server] Error opening destination file (in copyFile function):%s\n.", destinationPath);
        fclose(sourceFile);
        return 1;
    }
    // citesc caracter cu caracter
    while ((c = fgetc(sourceFile)) != EOF)
    {
        fputc(c, destinationFile);
    }
    fclose(sourceFile);
    fclose(destinationFile);
    return 0;
}

int copyDirectory(char *sourcePath, const char *destinationPath)
{
    DIR *sourceDirectory;
    struct dirent *sourceEntity;
    char entry_sourcePath[2000];
    char entry_destinationPath[2000];
    char *dirName = strrchr(sourcePath, '/');

    sourceDirectory = opendir(sourcePath);
    if (sourceDirectory == NULL)
    {
        printf("[Server] Error opening source directory (in 'copyDirectory' function):%s.\n", sourcePath);
        return 1;
    }

    if (mkdir(destinationPath, 0777) != 0)
    {
        printf("[Server] Destination directory could not be created (in 'copyDirectory' function): %s.\n", destinationPath);
        closedir(sourceDirectory);
        return 1;
    }

    while ((sourceEntity = readdir(sourceDirectory)) != NULL)
    {
        // nu copiez si directoarele "." si ".."
        if (strcmp(sourceEntity->d_name, ".") != 0 && strcmp(sourceEntity->d_name, "..") != 0)
        {
            memset(entry_sourcePath, 0, 1000);
            strcpy(entry_sourcePath, sourcePath);
            strcat(entry_sourcePath, "/");
            strcat(entry_sourcePath, sourceEntity->d_name);

            memset(entry_destinationPath, 0, 1000);
            strcpy(entry_destinationPath, destinationPath);
            strcat(entry_destinationPath, "/");
            strcat(entry_destinationPath, sourceEntity->d_name);

            if (sourceEntity->d_type == DT_REG) // e fisier obisnuit
            {
                if (copyFile(entry_sourcePath, entry_destinationPath) != 0)
                {
                    perror("[Server] Error copying file");
                    closedir(sourceDirectory);
                    return 1;
                }
            }
            else if (sourceEntity->d_type == DT_DIR) // e director
            {
                if (copyDirectory(entry_sourcePath, entry_destinationPath) != 0)
                {
                    perror("Error copying directory");
                    closedir(sourceDirectory);
                    return 1;
                }
            }
        }
    }
    closedir(sourceDirectory);
    return 0;
}

int compareFiles(const char *filePath_1, const char *filePath_2)
{
    int areEqual = 1;
    FILE *file_1, *file_2;
    if ((file_1 = fopen(filePath_1, "r")) == NULL)
    {
        perror("[Server] First file failed to open.\n");
        return 0;
    }
    else if ((file_2 = fopen(filePath_2, "r")) == NULL)
    {
        perror("[Server] Second file failed to open.\n");
        return 0;
    }

    char tempArray1[1000], tempArray2[1000];
    memset(tempArray1, 0, 1000);
    memset(tempArray2, 0, 1000);
    while (1)
    {
        char *line_1, *line_2;
        line_1 = fgets(tempArray1, 1000, file_1);
        line_2 = fgets(tempArray2, 1000, file_2);
        if (line_1 == NULL || line_2 == NULL)
        {
            printf("S-a ajuns la eof\n");
            break;
        }
        if (strcmp(line_1, line_2) != 0)
        {
            FILE *logs;
            if ((logs = fopen("logs.txt", "a+")) == NULL)
            {
                perror("[Server] Couldn't open logs file.\n");
            }
            else
            {
                fprintf(logs, "MODIFICARE:\nOld:%s\nUpdated:%s.\n", line_1, line_2);
            }
            fclose(logs);
            printf("Liniile nu sunt la fel\n");
            areEqual = 0;
            break;
        }
    }
    fclose(file_1);
    fclose(file_2);
    return areEqual;
}

int getChanges(const char *dirPath_1, const char *dirPath_2, char *myUser)
{
    int changesNr = 0;
    DIR *dir_1, *dir_2;
    struct dirent *entry_1, *entry_2;
    struct stat fileStat;
    if ((dir_1 = opendir(dirPath_1)) == NULL)
    {
        perror("[Server] First directory failed to open.\n");
        return 0;
    }
    else if ((dir_2 = opendir(dirPath_2)) == NULL)
    {
        perror("[Server] Second directory failed to open.\n");
        return 0;
    }
    // iau toatele entry-urile din primul director si le compar pe rand cu toate entry-urile din al doilea
    // pentru a gasi fisiere/directoare cu acelasi nume si apoi sa vad care linii sunt diferite
    // acele linii sunt printate in fisierul de log-uri
    while ((entry_1 = readdir(dir_1)) != NULL)
    {
        if (strcmp(entry_1->d_name, ".") == 0 || strcmp(entry_1->d_name, "..") == 0)
        {
            continue;
        }
        char entryPath_1[1000];
        memset(entryPath_1, 0, 1000);
        strcpy(entryPath_1, dirPath_1);
        strcat(entryPath_1, "/");
        strcat(entryPath_1, entry_1->d_name);
        if (stat(entryPath_1, &fileStat) == 0) // succes, putem vedea informatii despre fisierul/directorul din calea data
        {
            if (S_ISREG(fileStat.st_mode))
            {
                while ((entry_2 = readdir(dir_2)) != NULL)
                {
                    // citesc al doilea director si compar continutul, nu ma intereseaza "." si ".."
                    if (strcmp(entry_2->d_name, ".") == 0 || strcmp(entry_2->d_name, "..") == 0)
                        continue;
                    if (strcmp(entry_1->d_name, entry_2->d_name) == 0)
                    {
                        char entryPath_2[1000];
                         memset(entryPath_2, 0, 1000);
                        strcpy(entryPath_2, dirPath_2);
                        strcat(entryPath_2, "/");
                        strcat(entryPath_2, entry_2->d_name);
                        if (stat(entryPath_2, &fileStat) == 0 && S_ISREG(fileStat.st_mode))
                        {
                            int result = compareFiles(entryPath_1, entryPath_2);
                            if (result == 0)
                            {
                                changesNr++;
                                printf("Files %s and %s are different\n", entryPath_1, entryPath_2);
                                FILE *logs;
                                if ((logs = fopen("logs.txt", "a+")) == NULL)
                                {
                                    perror("[Server] Couldn't open logs file.\n");
                                }
                                else
                                {
                                    fprintf(logs, "(*) Fisierul '%s' a fost MODIFICAT de %s.\n", entry_1->d_name, myUser);
                                }
                                fclose(logs);
                            }
                        }
                    }
                }
                rewinddir(dir_2);
            }
            else if (S_ISDIR(fileStat.st_mode))
            {
                // citim din al doilea director
                while ((entry_2 = readdir(dir_2)) != NULL)
                {
                    if (strcmp(entry_2->d_name, ".") == 0 || strcmp(entry_2->d_name, "..") == 0)
                    {
                        continue;
                    }
                    // compar fisierele care au acelasi nume
                    if (strcmp(entry_1->d_name, entry_2->d_name) == 0)
                    {
                        char entryPath_2[1000];
                        memset(entryPath_2, 0, 1000);
                        strcpy(entryPath_2, dirPath_2);
                        strcat(entryPath_2, "/");
                        strcat(entryPath_2, entry_2->d_name);
                        if (stat(entryPath_2, &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
                        {
                            // compar recursiv directoarele
                            int result = getChanges(entryPath_1, entryPath_2, myUser);
                            if (result > 0)
                            {
                                changesNr += result;
                                printf("Directory %s and %s have %d different files\n", entryPath_1, entryPath_2, result);
                            }
                        }
                    }
                }
                // ii zicem directorului 2 sa o ia de la capat, pt ca vreau sa compar cate un fisier din primul director cu cate unul din al doilea
                rewinddir(dir_2);
            }
        }
    }
    closedir(dir_1);
    closedir(dir_2);
    return changesNr;
}

int copyRepository(const char *serverRepoName, const char *localRepoName)
{
    DIR *dir = opendir(serverRepoName);
    if (dir == NULL)
    {
        perror("Error opening source directory");
        return 1;
    }

    // Create the local repository
    int result;
    if (-1 == (result = mkdir(localRepoName, 0700)))
    {
        perror("[Server] mkdir error!\n");
        return 1;
    }

    struct dirent *entity;
    while ((entity = readdir(dir)) != NULL)
    {
        // Skip the "." and ".." entries
        if (strcmp(entity->d_name, ".") == 0 || strcmp(entity->d_name, "..") == 0)
        {
            continue;
        }

        char sourcePath[1000], destinationPath[1000];
        memset(sourcePath, 0, 1000);
        memset(destinationPath, 0, 1000);

        strcpy(sourcePath, serverRepoName);
        strcat(sourcePath, "/");
        strcat(sourcePath, entity->d_name);

        strcpy(destinationPath, localRepoName);
        strcat(destinationPath, "/");
        strcat(destinationPath, entity->d_name);

        struct stat s;
        if (stat(sourcePath, &s) == 0) // 0 -> succes => fisierul exista
        {
            if (S_ISDIR(s.st_mode))
            {
                if (copyRepository(sourcePath, destinationPath) == 1)
                    return 1; // recursiv
            }
            else
            {
                if (copyFile(sourcePath, destinationPath) == 1)
                    return 1;
            }
        }
        else
        {
            return 1;
        }
    }
    return 0;
    closedir(dir);
}

int addEntity(char *fileToCopy_path, char *localRepository_path)
{
    struct stat entityStat;
    char updatedPath[1000];
    memset(updatedPath, 0, 1000);
    if (stat(fileToCopy_path, &entityStat) != 0)
    {
        perror("[Server] Stat error checking file or directory (addEntity).\n");
        return 1; // 1 = eroare
    }

    localRepository_path[strlen(localRepository_path)] = '\0';

    char isolatedFile[100];
    memset(isolatedFile, 0, 100);
    strcpy(isolatedFile, strrchr(fileToCopy_path, '/') + 1); // de la ultimul slash incolo se afla numele file-ului
    strcpy(updatedPath, localRepository_path);
    strcat(updatedPath, "/");
    strcat(updatedPath, isolatedFile);

    printf("updated Path:%s\n", updatedPath);

    if (S_ISREG(entityStat.st_mode))
    {
        if (copyFile(fileToCopy_path, updatedPath) != 0)
        {
            perror("[Server] File copy error!(addEntity)\n");
            return 1; // eroare
        }
    }
    else if (S_ISDIR(entityStat.st_mode))
    {
        if (copyDirectory(fileToCopy_path, updatedPath) != 0)
        {
            perror("[Server] Directory copy error!(addEntity)\n");
            return 1; // eroare
        }
    }
    else
    {
        fprintf(stderr, "[Server] Not a regular file or directory!(addEntity)\n");
        return 1; // eroare
    }

    return 0; // succes
}

int getLatestVersion()
{
    DIR *dir;
    struct dirent *entry;
    int maxNumber = 0;
    if ((dir = opendir(".")) == NULL)
    {
        perror("Error opening directory");
        return 0;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, "version_"))
        {
            int number = atoi(entry->d_name + strlen("version_"));
            if (number > maxNumber)
                maxNumber = number;
        }
    }
    closedir(dir);
    return maxNumber;
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;
    int nr;
    int sd;
    int pid;
    pthread_t th[100];
    int i = 0;

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    while (1)
    {
        int client;
        thData *td; // parametru functia executata de thread
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, (socklen_t *)&length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } // while
};
static void *treat(void *arg) // argumentul este de tip void pentru ca noi putem apela functia cu orice tip
{
    struct thData tdL;
    tdL = *((struct thData *)arg); // continuare: dar apoi trebuie castat
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self()); // dupa ce un thread si-a terminat treaba, il detasam, adica el se termina si ne da inapoi memoria pe care a alocat-o
    raspunde((struct thData *)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg)
{
    int versionNumber = getLatestVersion();
    int revertsAvailable;
    printf("versionNumber:%d\n", versionNumber);
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char msg[100], inputUser[100];
    char msgrasp[100] = " ";
    memset(msg, 0, 100);
    int bytesRead;
    do
    {
        printf("[server]Asteptam mesajul...\n");
        fflush(stdout);

        if ((bytesRead = read(tdL.cl, msg, 100)) <= 0)
        {
            printf("[Server] Client %d - deconectat sau nu e nimic de citit.\n", tdL.idThread);
        }

        char processCommand[100];
        memset(processCommand, 0, 100);
        strcpy(processCommand, msg);
        printf("[Server] Command:%s\n", processCommand);
        if (strcmp(processCommand, "login") == 0)
        {
            char recvCredentials[200];
            memset(recvCredentials, 0, 200);
            if (read(tdL.cl, recvCredentials, 200) <= 0)
            {
                perror("[Server] Eroare la read() in server (1000).\n");
            }
            printf("recvCredentials:%s\n", recvCredentials);
            fflush(stdout);

            // process credentials

            char inputPasswd[100];
            memset(inputUser, 0, 100);
            memset(inputPasswd, 0, 100);

            char *tok = strtok(recvCredentials, ":");
            strcpy(inputUser, tok);
            printf("inputUser:%s\n", inputUser);
            fflush(stdout);
            tok = strtok(NULL, " ");
            strcpy(inputPasswd, tok);
            printf("inputPasswd:%s\n", inputPasswd);
            fflush(stdout);
            sqlite3 *db;
            sqlite3_stmt *stmt;
            if (sqlite3_open("usersDB", &db) != SQLITE_OK)
            {
                printf("Database failed to open.\n");
                fflush(stdout);
            }
            else
            {
                printf("Connected to database.\n");
                fflush(stdout);
            }

            int sqlAction = sqlite3_prepare_v2(db, "SELECT * FROM users WHERE username=? AND password=?", -1, &stmt, NULL);
            if (sqlAction != SQLITE_OK)
                printf("Error:%s\n", sqlite3_errmsg(db));

            sqlAction = sqlite3_bind_text(stmt, 1, inputUser, -1, SQLITE_STATIC);
            sqlAction = sqlite3_bind_text(stmt, 2, inputPasswd, -1, SQLITE_STATIC);
            sqlAction = sqlite3_step(stmt);

            // send login status to client

            char loginAnswer[100];
            memset(loginAnswer, 0, 100);
            if (sqlAction != SQLITE_ROW)
            {
                strcpy(loginAnswer, "Invalid username/password\n");
                if (write(tdL.cl, loginAnswer, 100) <= 0)
                {
                    perror("[Server] Eroare la read() in server (1).\n");
                }
                printf("loginStatus:%s", loginAnswer);
                fflush(stdout);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
            else
            {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                int deletedFiles = 0;
                int addedFiles = 0;
                int changesNr;
                bool isCommited = false;

                strcpy(loginAnswer, "You are logged in!\n\nWelcome, ");
                strcat(loginAnswer, inputUser);

                if (write(tdL.cl, loginAnswer, 100) <= 0)
                {
                    perror("[Server] Eroare la read() in server (1).\n");
                }

                printf("loginStatus:%s\n", loginAnswer);
                fflush(stdout);

                char accountCommand[100];
                memset(accountCommand, 0, 100);
                int bytesReadAccount;
                do
                {
                    if (-1 == (bytesReadAccount = read(tdL.cl, accountCommand, 100)))
                    {
                        perror("[Server] Eroare la read() in server (1).\n");
                    }
                    accountCommand[strlen(accountCommand)] = '\0';
                    printf("acc command:%s\n", accountCommand);
                    fflush(stdout);
                    if (strstr(accountCommand, "clone"))
                    {
                        char repoLocation[200], repoName[100], checkCloned[100];
                        memset(repoLocation, 0, 100);
                        memset(repoName, 0, 100);
                        memset(checkCloned, 0, 100);

                        char line[1024];
                        bool found = 0;
                        FILE *checkRepo = fopen("repo_paths.txt", "r");
                        if (checkRepo == NULL)
                        {
                            perror("Error opening file");
                        }
                        while (fgets(line, sizeof(line), checkRepo))
                        {
                            if (strstr(line, inputUser) != NULL)
                            {
                                found = 1;
                                printf("Found '%s' in line: %s", inputUser, line);
                            }
                        }
                        fclose(checkRepo);

                        if (found == 1)
                        {
                            strcpy(checkCloned, "You have already cloned this repository on your local machine!\n");
                            if (write(tdL.cl, checkCloned, 100) <= 0)
                            {
                                perror("[Server] Eroare la read() in server (1).\n");
                            }
                        }
                        else
                        {
                            strcpy(checkCloned, "Something else\n");

                            // strcpy(repoLocation, "../client/");

                            if (write(tdL.cl, checkCloned, 100) <= 0)
                            {
                                perror("[Server] Eroare la read() in server (1).\n");
                            }

                            if (read(tdL.cl, repoLocation, 200) <= 0)
                            {
                                perror("[Server] Eroare la read() in server (1).\n");
                            }

                            printf("%s\n", repoLocation);
                            if (read(tdL.cl, repoName, 200) <= 0)
                            {
                                perror("[Server] Eroare la read() in server (1).\n");
                            }

                            printf("repo name:%s", repoName);

                            strcat(repoLocation, repoName);
                            char latestVersion[100];
                            memset(latestVersion, 0, 100);
                            strcpy(latestVersion, "version_");
                            sprintf(latestVersion + strlen(latestVersion), "%d", versionNumber);
                            printf("latest version: %s\n", latestVersion);
                            char cloneStatus[200];
                            memset(cloneStatus, 0, 200);
                            if (copyRepository(latestVersion, repoLocation) == 0)
                            {
                                strcpy(cloneStatus, "\n\nRepository cloned succesfully!\n");
                                printf("%s\n", cloneStatus);
                                fflush(stdout);
                                if (write(tdL.cl, cloneStatus, strlen(cloneStatus)) <= 0)
                                {
                                    perror("[Server] Eroare la read() in server (1).\n");
                                }
                            }
                            else
                            {
                                strcpy(cloneStatus, "Repository clone error!\n");
                                printf("%s\n", cloneStatus);
                                fflush(stdout);
                                if (write(tdL.cl, cloneStatus, strlen(cloneStatus)) <= 0)
                                {
                                    perror("[Server] Eroare la read() in server (1).\n");
                                }
                            }

                            char fileLine[200];
                            memset(fileLine, 0, 200);
                            strcpy(fileLine, inputUser);
                            strcat(fileLine, ":");
                            strcat(fileLine, repoLocation);
                            FILE *repoPaths;
                            if ((repoPaths = fopen("repo_paths.txt", "a+")) == NULL)
                            {
                                perror("[Server] Couldn't open logs file for deleting directory.\n");
                            }
                            else
                            {
                                fprintf(repoPaths, "%s\n", fileLine);
                            }
                            fclose(repoPaths);
                        }
                    }
                    else if (strstr(accountCommand, "add"))
                    {
                        char fileToAdd[200];
                        memset(fileToAdd, 0, 200);
                        if (read(tdL.cl, fileToAdd, 200) <= 0)
                        {
                            perror("[Server] Eroare la read() in server (1).\n");
                        }

                        printf("file to add:%s\n", fileToAdd);

                        char *fileName;
                        fileName = strrchr(fileToAdd, '/');

                        char addStatus[100], currentLocation[200];
                        memset(currentLocation, 0, 200);

                        char line[1024];
                        bool foundUser = false;
                        FILE *findLocation = fopen("repo_paths.txt", "r");

                        if (findLocation == NULL)
                        {
                            perror("Error opening file");
                        }

                        while (fgets(line, sizeof(line), findLocation))
                        {
                            if (strstr(line, inputUser) != NULL)
                            {
                                foundUser = true;
                                printf("Found '%s' in line: %s", inputUser, line);
                                char *pathTok = strtok(line, ":");
                                pathTok = strtok(NULL, " ");
                                strcpy(currentLocation, pathTok);
                            }
                        }

                        fclose(findLocation);
                        currentLocation[strlen(currentLocation) - 1] = '\0';
                        printf("current LOCATION: %s\n", currentLocation);
                        memset(addStatus, 0, 100);
                        if (addEntity(fileToAdd, currentLocation) == 0)
                        {
                            addedFiles++;
                            bool isDir = false;
                            FILE *logs;
                            if ((logs = fopen("logs.txt", "a+")) == NULL)
                            {
                                perror("[Server] Couldn't open logs file for deleting directory.\n");
                            }
                            else
                            {
                                if (strstr(fileName, "."))
                                    fprintf(logs, "(+) Fisierul '%s' a fost ADAUGAT de %s.\n", fileName + 1, inputUser);
                                else
                                {
                                    isDir = true;
                                    fprintf(logs, "(+) Directorul '%s' a fost ADAUGAT de %s.\n", fileName + 1, inputUser);
                                }
                            }
                            fclose(logs);
                            if (isDir)
                            {
                                strcpy(addStatus, "Directory added succesfully!\n");
                            }
                            else
                            {
                                strcpy(addStatus, "File added succesfully!\n");
                            }
                        }
                        else
                        {
                            strcpy(addStatus, "[Err] Failed to add file. Check the name of the paths and try again.\n");
                        }
                        if (write(tdL.cl, addStatus, strlen(addStatus)) <= 0)
                        {
                            perror("[Server] Add write error.\n");
                        }
                    }
                    else if (strstr(accountCommand, "delete"))
                    {

                        char path_toDelete[1000];
                        memset(path_toDelete, 0, 1000);
                        if (read(tdL.cl, path_toDelete, 200) <= 0)
                        {
                            perror("[Server] Eroare la read() in server (1).\n");
                        }
                        path_toDelete[strlen(path_toDelete) - 1] = '\0';
                        printf("path_toDelete:%s\n", path_toDelete);
                        char *fileName = strrchr(path_toDelete, '/');

                        char currentLocation[200];
                        memset(currentLocation, 0, 200);
                        char line[1024];
                        bool foundUser = false;
                        FILE *findLocation = fopen("repo_paths.txt", "r");

                        if (findLocation == NULL)
                        {
                            perror("Error opening file");
                        }

                        while (fgets(line, sizeof(line), findLocation))
                        {
                            if (strstr(line, inputUser) != NULL)
                            {
                                foundUser = true;
                                printf("Found '%s' in line: %s", inputUser, line);
                                char *pathTok = strtok(line, ":");
                                pathTok = strtok(NULL, " ");
                                strcpy(currentLocation, pathTok);
                            }
                        }

                        fclose(findLocation);
                        currentLocation[strlen(currentLocation) - 1] = '\0';
                        printf("current LOCATION: %s\n", currentLocation);
                        strcat(currentLocation, path_toDelete);

                        bool isDir = false;
                        char deleteStatus[100];
                        memset(deleteStatus, 0, 100);
                        if (deleteEntity(currentLocation) == 0)
                        {
                            deletedFiles++;
                            FILE *logs;
                            if ((logs = fopen("logs.txt", "a+")) == NULL)
                            {
                                perror("[Server] Couldn't open logs file for deleting directory.\n");
                            }
                            else
                            {
                                if (strstr(fileName, "."))
                                {
                                    fprintf(logs, "(-) Fisierul '%s' a fost STERS de %s.\n", fileName + 1, inputUser);
                                }
                                else
                                {
                                    isDir = true;
                                    fprintf(logs, "(-) Directorul '%s' a fost STERS de %s.\n", fileName + 1, inputUser);
                                }
                            }
                            fclose(logs);
                            if (isDir)
                                strcpy(deleteStatus, "Directory deleted succesfully!\n");
                            else
                                strcpy(deleteStatus, "File deleted succesfully!\n");
                        }
                        else
                        {
                            strcpy(deleteStatus, "[Err] Failed to delete file. Check the name of the path and try again.\n");
                        }
                        if (write(tdL.cl, deleteStatus, strlen(deleteStatus)) <= 0)
                        {
                            perror("[Server] Delete write error.\n");
                        }
                    }
                    else if (strstr(accountCommand, "commit"))
                    {
                        char line1[1024];
                        bool foundUser1 = false;
                        char perm[100];
                        memset(perm, 0, 100);
                        FILE *permissions = fopen("permissions.txt", "r");
                        if (permissions == NULL)
                        {
                            perror("[Server] Error opening file!.\n");
                        }
                        while (fgets(line1, sizeof(line1), permissions))
                        {
                            if (strstr(line1, inputUser) != NULL)
                            {
                                foundUser1 = true;
                                printf("Found '%s' in line: %s", inputUser, line1);
                                char *permTok = strtok(line1, ":");
                                permTok = strtok(NULL, " ");
                                strcpy(perm, permTok);
                            }
                        }
                        fclose(permissions);
                        perm[strlen(perm)] = '\0';
                        printf("permission: %s\n", perm);

                        char commitStatus[100];
                        memset(commitStatus, 0, 100);

                        if (strstr(perm, "denied"))
                        {
                            strcpy(commitStatus, "\n\nYou don't have the permission to perform this action.\n");
                            if (write(tdL.cl, commitStatus, sizeof(commitStatus)) <= 0)
                            {
                                perror("[Server] Eroare la write() in server.\n");
                            }
                        }
                        else if (strstr(perm, "granted"))
                        {
                            char added[100], deleted[100];
                            memset(deleted, 0, 100);
                            memset(added, 0, 100);

                            char line[1024];
                            bool foundUser = false;
                            FILE *findLocation = fopen("repo_paths.txt", "r");
                            char currentLocation[200];
                            memset(currentLocation, 0, 200);
                            if (findLocation == NULL)
                            {
                                perror("Error opening file");
                            }

                            while (fgets(line, sizeof(line), findLocation))
                            {
                                if (strstr(line, inputUser) != NULL)
                                {
                                    foundUser = true;
                                    printf("Found '%s' in line: %s\n", inputUser, line);
                                    char *pathTok = strtok(line, ":");
                                    pathTok = strtok(NULL, " ");
                                    strcpy(currentLocation, pathTok);
                                }
                            }

                            fclose(findLocation);
                            currentLocation[strlen(currentLocation) - 1] = '\0';
                            printf("current LOCATION: %s\n", currentLocation);

                            isCommited = true;
                            
                            char latestLocation[200];
                            memset(latestLocation, 0, 200);
                            strcpy(latestLocation, "version_");
                            sprintf(latestLocation + strlen(latestLocation), "%d", versionNumber);
                            changesNr = getChanges(latestLocation, currentLocation, inputUser);
                            printf("number of changes:%d\n", changesNr);

                            if (addedFiles == 0 && deletedFiles == 0 && changesNr == 0)
                            {
                                strcpy(commitStatus, "\n[!] Nothing to commit.");
                                if (write(tdL.cl, commitStatus, sizeof(commitStatus)) <= 0)
                                {
                                    perror("[Server] Eroare la write() in server.\n");
                                }
                            }
                            else
                            {
                                strcpy(commitStatus, "\nCommited successfully!");
                                char versionMsg[100];
                                memset(versionMsg, 0, 100);
                                strcpy(versionMsg, "^ Modificarile versiunii ");
                                sprintf(versionMsg + strlen(versionMsg), "%d", versionNumber);
                                strcat(versionMsg, "\n\n");
                                FILE *logs;
                                if ((logs = fopen("logs.txt", "a+")) == NULL)
                                {
                                    perror("[Server] Couldn't open logs file for deleting directory.\n");
                                }
                                else
                                {
                                    fprintf(logs, "%s\n", versionMsg);
                                }
                                fclose(logs);

                                if (write(tdL.cl, commitStatus, sizeof(commitStatus)) <= 0)
                                {
                                    perror("[Server] Eroare la write() in server.\n");
                                }
                                if (write(tdL.cl, &addedFiles, sizeof(addedFiles)) <= 0)
                                {
                                    perror("[Server] Eroare la write() in server (1).\n");
                                }
                                if (write(tdL.cl, &deletedFiles, sizeof(addedFiles)) <= 0)
                                {
                                    perror("[Server] Eroare la write() in server (1).\n");
                                }

                                if (write(tdL.cl, &changesNr, sizeof(changesNr)) <= 0)
                                {
                                    perror("[Server] Eroare la write() in server (1).\n");
                                }
                                addedFiles = 0;
                                deletedFiles = 0;
                                changesNr = 0;
                            }
                        }
                    }
                    else if (strstr(accountCommand, "push"))
                    {
                        char line[1024];
                        bool foundUser = false;
                        versionNumber = getLatestVersion();
                        FILE *findLocation = fopen("repo_paths.txt", "r");
                        char currentLocation[200];
                        memset(currentLocation, 0, 200);
                        if (findLocation == NULL)
                        {
                            perror("Error opening file");
                        }

                        while (fgets(line, sizeof(line), findLocation))
                        {
                            if (strstr(line, inputUser) != NULL)
                            {
                                foundUser = true;
                                printf("Found '%s' in line: %s", inputUser, line);
                                char *pathTok = strtok(line, ":");
                                pathTok = strtok(NULL, " ");
                                strcpy(currentLocation, pathTok);
                            }
                        }

                        fclose(findLocation);
                        currentLocation[strlen(currentLocation) - 1] = '\0';
                        printf("current LOCATION: %s\n", currentLocation);

                        char pushMessage[100];
                        memset(pushMessage, 0, 100);
                        if (isCommited == true)
                        {
                            char repoLocation[200], repoName[100];
                            memset(repoLocation, 0, 100);
                            memset(repoName, 0, 100);
                            strcpy(repoLocation, "version_");
                            versionNumber++;
                            sprintf(repoLocation + strlen(repoLocation), "%d", versionNumber);
                            copyRepository(currentLocation, repoLocation);
                            isCommited = false;
                            strcpy(pushMessage, "Pushed succesfully!\n");
                            printf("[Server] %s\n", pushMessage);

                            char line[1024];
                            FILE *findLocation = fopen("repo_paths.txt", "r");
                            char replaceRepo[200];
                            memset(replaceRepo, 0, 200);
                            if (findLocation == NULL)
                            {
                                perror("Error opening file");
                            }

                            while (fgets(line, sizeof(line), findLocation))
                            {
                                char *pathTok = strtok(line, ":");
                                pathTok = strtok(NULL, " ");
                                strcpy(replaceRepo, pathTok);
                                replaceRepo[strlen(replaceRepo) - 1] = '\0';
                                printf("current LOCATION: %s\n", replaceRepo);
                                deleteDirectory(replaceRepo);
                                copyRepository(repoLocation, replaceRepo);
                                memset(replaceRepo, 0, 200);
                            }
                            fclose(findLocation);
                            revertsAvailable++;
                        }
                        else
                        {
                            strcpy(pushMessage, "You have to commit before pushing!\n");
                            printf("[Server] %s\n", pushMessage);
                        }
                        if (write(tdL.cl, pushMessage, strlen(pushMessage)) <= 0)
                        {
                            perror("[Server] Failed to write push message!\n");
                        }
                    }
                    else if (strstr(accountCommand, "logout"))
                    {
                        printf("[Server] User '%s' has logged out.\n", inputUser);
                        break;
                    }
                    else if (strstr(accountCommand, "revert"))
                    {
                        char revertMessage[100];
                        memset(revertMessage, 0, 100);
                        if (revertsAvailable > 0)
                        {
                            char lastVersion[1000];
                            memset(lastVersion, 0, 1000);
                            strcpy(lastVersion, "version_");
                            sprintf(lastVersion + strlen(lastVersion), "%d", versionNumber);
                            printf("lastVersion:%s\n", lastVersion);
                            revertsAvailable--;
                            deleteDirectory(lastVersion);
                            versionNumber--;
                            strcpy(revertMessage, "Reverted successfully!\n");
                            addedFiles = 0;
                            deletedFiles = 0;
                            changesNr = 0;
                            memset(lastVersion, 0, 1000);
                            strcpy(lastVersion, "version_");
                            sprintf(lastVersion + strlen(lastVersion), "%d", versionNumber);

                            char line[1024];
                            FILE *findLocation = fopen("repo_paths.txt", "r");
                            char replaceRepo[200];
                            memset(replaceRepo, 0, 200);
                            if (findLocation == NULL)
                            {
                                perror("Error opening file");
                            }

                            while (fgets(line, sizeof(line), findLocation))
                            {
                                char *pathTok = strtok(line, ":");
                                pathTok = strtok(NULL, " ");
                                strcpy(replaceRepo, pathTok);
                                replaceRepo[strlen(replaceRepo) - 1] = '\0';
                                printf("current LOCATION: %s\n", replaceRepo);
                                deleteDirectory(replaceRepo);
                                copyRepository(lastVersion, replaceRepo);
                                memset(replaceRepo, 0, 200);
                            }
                            fclose(findLocation);
                        }
                        else
                        {
                            strcpy(revertMessage, "You can't revert anymore!\n");
                        }
                        if (write(tdL.cl, revertMessage, strlen(revertMessage)) <= 0)
                        {
                            perror("[Server] Failed to write revert message!\n");
                        }
                    }
                    else if (strstr(accountCommand, "login"))
                    {
                        printf("\nUser already logged in.\n");
                    }
                    else if (strstr(accountCommand, "signup"))
                    {
                        printf("\nLogout to signup.\n");
                    }
                } while (bytesReadAccount > 0);
            }
        }
        else if (strcmp(processCommand, "signup") == 0)
        {

            char inputUser[100], inputPasswd[100], inputConfPasswd[100];
            memset(inputUser, 0, 100);
            memset(inputPasswd, 0, 100);
            memset(inputConfPasswd, 0, 100);

            char recvCredentials[200];
            memset(recvCredentials, 0, 200);
            if (read(tdL.cl, recvCredentials, 200) <= 0)
            {
                perror("[Server] Eroare la read() in server (1).\n");
            }
            printf("recvCredentials:%s\n", recvCredentials);
            fflush(stdout);

            char *tok = strtok(recvCredentials, ":");
            strcpy(inputUser, tok);
            printf("inputUser:%s\n", inputUser);
            fflush(stdout);
            tok = strtok(NULL, ":");
            strcpy(inputPasswd, tok);
            printf("inputPasswd:%s\n", inputPasswd);
            tok = strtok(NULL, " ");
            strcpy(inputConfPasswd, tok);
            printf("inputConfPasswd:%s\n", inputConfPasswd);
            fflush(stdout);

            // signup status

            char signupStatus[100];
            memset(signupStatus, 0, 100);
            sqlite3 *db;
            sqlite3_stmt *stmt;
            if (sqlite3_open("usersDB", &db) != SQLITE_OK)
            {
                printf("Database failed to open.\n");
                fflush(stdout);
            }
            else
            {
                printf("Connected to database.\n");
                fflush(stdout);
            }
            if (strcmp(inputPasswd, inputConfPasswd) == 0)
            {
                int sqlAction = sqlite3_prepare_v2(db, "INSERT INTO users (username, password) VALUES(?, ?)", -1, &stmt, 0);

                sqlite3_bind_text(stmt, 1, inputUser, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, inputPasswd, -1, SQLITE_STATIC);

                if (sqlAction == SQLITE_OK)
                {
                    int stepResult = sqlite3_step(stmt);
                    if (stepResult == SQLITE_DONE)
                    {
                        printf("User added to database.\n");
                        strcat(signupStatus, "User added to database.\n");

                        char addUserPerm[100];
                        memset(addUserPerm, 0, 100);
                        strcpy(addUserPerm, inputUser);
                        strcat(addUserPerm, ":");
                        strcat(addUserPerm, "granted\n");
                        FILE *permissions = fopen("permissions.txt", "a+");
                        if (permissions == NULL)
                        {
                            perror("[Server] Error opening file!.\n");
                        }
                        fprintf(permissions, "%s\n", addUserPerm);
                        fclose(permissions);
                    }
                    else if (stepResult == SQLITE_CONSTRAINT)
                    {
                        printf("Username already exists.\n");
                        strcat(signupStatus, "Username already exists.\n");
                    }
                    else
                    {
                        printf("Insert error:%s\n", sqlite3_errmsg(db));
                    }
                    sqlite3_finalize(stmt);
                }
                else
                {
                    printf("[ERR] SQL query failed!\n");
                }
                sqlite3_close(db);
            }
            else
            {
                printf("Passwords don't match.\n");
                strcat(signupStatus, "Passwords don't match.\n");
                sqlite3_close(db);
            }
            if (write(tdL.cl, signupStatus, 100) <= 0)
            {
                perror("[Server] Eroare la write() catre client.\n");
            }
        }
        else if (strstr(processCommand, "login as admin"))
        {
            char recvCredentials[200], a_loginStatus[200];
            memset(recvCredentials, 0, 200);
            memset(a_loginStatus, 0, 200);
            if (read(tdL.cl, recvCredentials, 200) <= 0)
            {
                perror("[Server] Eroare la read() in server (12362).\n");
            }
            printf("recvCredentials:%s\n", recvCredentials);
            fflush(stdout);

            // process credentials

            char a_inputUser[100], a_inputPasswd[100];
            memset(a_inputUser, 0, 100);
            memset(a_inputPasswd, 0, 100);

            char *tok = strtok(recvCredentials, ":");
            strcpy(a_inputUser, tok);
            printf("inputUser:%s\n", inputUser);
            fflush(stdout);
            tok = strtok(NULL, " ");
            strcpy(a_inputPasswd, tok);
            printf("inputPasswd:%s\n", a_inputPasswd);
            fflush(stdout);
            if (strcmp(a_inputUser, "admin") == 0 && strcmp(a_inputPasswd, "admin") == 0)
            {
                strcpy(a_loginStatus, "Logged in succesfully!\n");
                if (write(tdL.cl, a_loginStatus, strlen(a_loginStatus)) <= 0)
                {
                    perror("[Server] Eroare la write() catre client (1).\n");
                }
                char adminCommand[100];
                memset(adminCommand, 0, 100);
                int bytesReadAdmin;
                do
                {
                    if ((bytesReadAdmin = read(tdL.cl, adminCommand, 100)) <= 0)
                    {
                        perror("[Server] Eroare la read() in server (12362).\n");
                    }
                    printf("admin command:%s\n", adminCommand);
                    if (strstr(adminCommand, "change permission"))
                    {
                        char userToChange[100];
                        memset(userToChange, 0, 100);
                        if (read(tdL.cl, userToChange, 100) <= 0)
                        {
                            perror("[Server] Eroare la read() in server (12362).\n");
                        }
                        userToChange[strlen(userToChange) - 1] = '\0';
                        printf("user to change permission:%s\n", userToChange);
                        char line1[1024];
                        bool foundUser = false;
                        char perm[100];
                        memset(perm, 0, 100);
                        FILE *permissions = fopen("permissions.txt", "r+");
                        FILE *temp = fopen("temp.txt", "r+");
                        if (permissions == NULL)
                        {
                            perror("[Server] Error opening file!.\n");
                        }
                        if (temp == NULL)
                        {
                            perror("[Server] Error opening file!.\n");
                        }
                        char *pos;
                        while (fgets(line1, sizeof(line1), permissions))
                        {
                            if (strstr(line1, userToChange) == 0)
                            {
                                fputs(line1, temp);
                            }
                            else
                            {
                                foundUser = true;
                                printf("Found '%s' in line: %s", userToChange, line1);
                                char *permTok = strtok(line1, ":");
                                permTok = strtok(NULL, " ");
                                strcpy(perm, permTok);
                            }
                        }
                        perm[strlen(perm) - 1] = '\0';
                        printf("permission: %s\n", perm);
                        char newPerm[50], permStatusMessage[100];
                        memset(newPerm, 0, 50);
                        memset(permStatusMessage, 0, 100);
                        strcpy(newPerm, userToChange);
                        strcat(newPerm, ":");
                        if (strstr(perm, "granted"))
                        {
                            strcat(newPerm, "denied");
                            strcpy(permStatusMessage, "(-) Denied permission to user ");
                        }
                        else if (strstr(perm, "denied"))
                        {
                            strcat(newPerm, "granted");
                            strcpy(permStatusMessage, "(+) Granted permission to user ");
                        }
                        fprintf(temp, "\n%s", newPerm);
                        fclose(permissions);
                        fclose(temp);
                        strcat(permStatusMessage, userToChange);
                        if ((permissions = fopen("permissions.txt", "r+")) == NULL)
                        {
                            printf("file open error!\n");
                        }
                        if ((temp = fopen("temp.txt", "r+")) == NULL)
                        {
                            printf("file open error!\n");
                        }
                        int bytesRead;
                        char buffer[4096];
                        while ((bytesRead = fread(buffer, 1, 4096, temp)) > 0)
                        {
                            if (fwrite(buffer, 1, bytesRead, permissions) != bytesRead)
                            {
                                perror("[Server] Write to destination file error!\n");
                            }
                        }
                        fclose(permissions);
                        fclose(temp);
                        if (write(tdL.cl, permStatusMessage, strlen(permStatusMessage)) <= 0)
                        {
                            perror("[Server] Eroare la write() catre client.\n");
                        }
                    }
                    else if (strstr(adminCommand, "logout"))
                    {
                        printf("[Server] Admin has logged out.\n");
                        break;
                    }
                } while (bytesReadAdmin > 0);
            }
            else
            {
                strcpy(a_loginStatus, "Invalid username/password.\n");
                if (write(tdL.cl, a_loginStatus, strlen(a_loginStatus)) <= 0)
                {
                    perror("[Server] Eroare la write() catre client (1).\n");
                }
            }
        }
        else if (strcmp(processCommand, "exit") == 0)
        {
            printf("[Server] Goodbye user!\n");
        }
    } while (bytesRead > 0);
}
