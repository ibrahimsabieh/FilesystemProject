#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define RESOURCE_SERVER_PORT 1067 // Change this!
#define BUF_SIZE 256

// We make this a global so that we can refer to it in our signal handler
int serverSocket;
FILE * outputFile[8];
/*
 We need to make sure we close the connection on signal received, otherwise we have to wait
 for server to timeout.
 */
void closeConnection() {
    printf("\nClosing Connection with file descriptor: %d \n", serverSocket);
    close(serverSocket);
    exit(1);
}

char * readFilePartition(char *fileName, int partiton){
    FILE *mappingFile = fopen(fileName, "r");
    char path[100];
    char garbage[500];
    int flag = 0;
    char *content = malloc(500);
    char *wholeContent = malloc(3000);

    while(fgetc(mappingFile) != EOF){
        fseek(mappingFile, -1, SEEK_CUR);
        if(fgetc(mappingFile) == (partiton + '0')){
            fseek(mappingFile, 1, SEEK_CUR);
            fgets(path, 100, mappingFile);
            flag = 1;
            break;
        }
        fgets(garbage, 500, mappingFile);
    }

    fclose(mappingFile);

    if(flag == 0){
        strcpy(wholeContent, "The provided partition does not exist!\n");
        return wholeContent;
    }

    for(int i=0; i<strlen(path); i++){
        if(path[i] == '\n')
            path[i] = '\0';
    }

    FILE *partitionFile = fopen(path, "r");
    strcpy(wholeContent, "");
    strcpy(content, "");
    while(fgetc(partitionFile) != EOF)
    {
        fseek(partitionFile, -1, SEEK_CUR);
        fgets(content, 500, partitionFile);
        strcat(wholeContent, content);
    }

    fclose(partitionFile);
    free(content);

    return wholeContent;
}

char * readFile(char *fileName){
    FILE *mappingFile = fopen(fileName, "r");
    char path[8][100];
    char contentLine[500];
    char *strContent = malloc(300000);
    int numFiles = 0, i, j=0;

    while(fgetc(mappingFile) != EOF){
        fseek(mappingFile, 1, SEEK_CUR);
        fgets(path[j++], 100, mappingFile);
        numFiles++;
    }
    fclose(mappingFile);

    for(j=0; j<numFiles; j++){
        for(i=0; i<strlen(path[j]); i++){
            if(path[j][i] == '\n')
                path[j][i] = '\0';
        }
    }


    for(i=0; i<numFiles; i++){
        outputFile[i] = fopen(path[i], "r");
    }

    i=0;
    strcpy(strContent, "");
    while(1){
        if(fgetc(outputFile[i]) == EOF){
            for(i=0; i<numFiles; i++){
                fclose(outputFile[i]);
            }
            return strContent;
        }

        fseek(outputFile[i], -1, SEEK_CUR);
        fgets(contentLine, 500, outputFile[i]);
        strcat(strContent, contentLine);

        i++;
        if(i == numFiles)
            i=0;
    }

}

void removeFile(char *fileName){
    FILE *mappingFile = fopen(fileName, "r");
    char path[8][100];
    int numFiles = 0, i, j=0, checkStatus;

    while(fgetc(mappingFile) != EOF){
        fseek(mappingFile, 1, SEEK_CUR);
        fgets(path[j++], 100, mappingFile);
        numFiles++;
    }
    fclose(mappingFile);

    for(j=0; j<numFiles; j++){
        for(i=0; i<strlen(path[j]); i++){
            if(path[j][i] == '\n')
                path[j][i] = '\0';
        }
    }


    j=0;
    for(i=0; i<numFiles; i++){
        checkStatus = remove(path[i]);
        if (checkStatus == 0)
            j++;
        else
            printf("Failed to Delete File #%d \n", i);

        if (j == numFiles)
            printf("Partitions Deleted Successfully\n");
    }

    checkStatus = remove(fileName);

    if (checkStatus == 0)
        printf("Mapping file deleted\n");
    else
        printf("Failed to delete mapping file\n");

}

void create(int partitions, char* fileName, char *fileContent){
    FILE * mappingFile = fopen(fileName, "w");
    int i = 0, j, k;
    char str[30];
    char * directories[8] = {"diska", "diskb", "diskc", "diskd", "diske", "diskf", "diskg", "diskh"};
    char content[256];
    char cwd[256];
    char *token;

    while(i < partitions){
        fprintf(mappingFile, "%d:", i+1);
        getcwd(cwd, sizeof(cwd));
        strcat(cwd, "/");
        strcat(cwd, directories[i++]);
        strcat(cwd, "/output.txt");
        fputs(cwd, mappingFile);
        fputc('\n', mappingFile);
    }
    fclose(mappingFile);

    k=0;
    j=0;
    i=0;
    token = strtok(fileContent, "\n");
    while (token != NULL) {
        if(j==partitions)
            j=0;
        strcpy(content, token);
        strcat(content, "\n");
        strcpy(str, "./");
        strcat(str, directories[j++]);
        strcat(str, "/output.txt");
        outputFile[k] = fopen(str, "a");
        fputs(content, outputFile[k]);
        fclose(outputFile[k]);
        k++;
        i++;
        token = strtok(NULL, "\n");
    }

}
// Create a separate function to process thread request
void * processClientRequest(void * request) {
    int connectionToClient = *(int *)request;
    char receiveLine[BUF_SIZE];
    char sendLine[BUF_SIZE];
    char funccall[BUF_SIZE];
    char filename[BUF_SIZE];
    int partitions = 0;
    char contents[BUF_SIZE];
    char userInput[BUF_SIZE];
    char *token;
    char *toDisplay = {"true"};


    int bytesReadFromClient = 0;
    // Read the request from the client
    while ( (bytesReadFromClient = read(connectionToClient, receiveLine, BUF_SIZE)) > 0) {
        strcpy(userInput, receiveLine);

        token = strtok(userInput, "\n");
        sscanf(token, "%s %s %d", funccall, filename, &partitions);

        if(strcmp(funccall, "create") == 0)
        {
            token = strtok(NULL, "");
            strcpy(contents, token);
        }

        // Need to put a NULL string terminator at end
        receiveLine[bytesReadFromClient] = 0;

        // Show what client sent
        printf("Received: %s\n", receiveLine);

        if(strcmp(funccall, "create") == 0  && partitions >= 1 && partitions <= 8)
            create(partitions, filename, contents);

        else if(strcmp(funccall, "read") == 0 && partitions == 0)
            toDisplay = readFile(filename);

        else if(strcmp(funccall, "read") == 0 && partitions != 0)
            toDisplay = readFilePartition(filename, partitions);

        else if(strcmp(funccall, "delete") == 0 && partitions == 0)
            removeFile(filename);


        printf("Sending output, if any\n");
        write(connectionToClient, toDisplay, strlen(toDisplay));

        if(strcmp(funccall, "read") == 0)
            free(toDisplay);


        // Zero out the receive line so we do not get artifacts from before
        bzero(&receiveLine, sizeof(receiveLine));
        close(connectionToClient);
    }
}
int main(int argc, char *argv[]) {
    int connectionToClient, bytesReadFromClient;

    // Create a server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddress;
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family      = AF_INET;

    // INADDR_ANY means we will listen to any address
    // htonl and htons converts address/ports to network formats
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port        = htons(RESOURCE_SERVER_PORT);

    // Bind to port
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Unable to bind to port just yet, perhaps the connection has to be timed out\n");
        exit(-1);
    }

    // Before we listen, register for Ctrl+C being sent so we can close our connection
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = closeConnection;
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Listen and queue up to 10 connections
    listen(serverSocket, 10);

    while (1) {
        /*
         Accept connection (this is blocking)
         2nd parameter you can specify connection
         3rd parameter is for socket length
         */
        connectionToClient = accept(serverSocket, (struct sockaddr *) NULL, NULL);

        // Kick off a thread to process request
        pthread_t someThread;
        pthread_create(&someThread, NULL, processClientRequest, (void *)&connectionToClient);

    }
}