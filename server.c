#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define SERVER_KEY 1234
#define CLIENT_TYPE 1000

struct message
{
    long mtype;
    long sender_id;
    char mtext[256];
};

void handle_client_request(int client_id, int client_queue, const char *request)
{
    struct message msg;
    char response[256];
    int sendMsg = 1;

    if (request[0] == '1')
    {
        snprintf(response, sizeof(response), "\nhello from server child %d\n", client_id);
    }
    else if (request[0] == '2')
    {
        printf("In File Search\n");
        FILE *fp;
        char filename[256];
        int found = 0;

        strcpy(filename, request + 1);

        if ((fp = fopen(filename, "r")) != NULL)
        {
            found = 1;
            fclose(fp);
        }

        if (found)
        {
            
            snprintf(response, sizeof(response), "\nFile %s found.\n", filename);
        }
        else
        {
            snprintf(response, sizeof(response), "\nFile %s not found.\n", filename);
        }
    }
    else if (request[0] == '3')
    {
        char temp[30];
        strcpy(temp, request + 1);
        const char *temp1 = temp;
        pid_t child_pid = fork();
        if (child_pid == 0) {
            const char *output_file = "output.txt";
            sendMsg = 0;
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1)
            {
                perror("open");
                return;
            }
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("dup2");
                return;
            }

            close(fd);
            execlp("wc", "wc", "-w", temp1, NULL);
        } else {
            int status = 0;
            int wpid = wait(&status);
            FILE *outfile = fopen("output.txt", "r");
            fgets(response, 100, outfile);
        }
    }
    else if (request[0] == '4')
    {
        msgctl(client_queue, IPC_RMID, NULL);
        printf("Client %d exiting.\n", client_id);
        printf("Cleanup completed\n");
        exit(0);
    }
    else
    {
        snprintf(response, sizeof(response), "Invalid request.");
    }
    if (sendMsg) {
        printf("Sending message to client id %d", client_id);
        msg.mtype = client_id;
        msg.sender_id = CLIENT_TYPE;
        strcpy(msg.mtext, response);

        if (msgsnd(client_queue, &msg, sizeof(msg.mtext), 0) == -1)
        {
            perror("msgsnd");
            exit(1);
        }
        printf("Message sent from server\n");
    }
    exit(0);
}

int main()
{
    int server_queue;
    int cl_queue;
    key_t server_key;
    server_key = ftok("server.c", SERVER_KEY);
    server_queue = msgget(server_key, IPC_CREAT | 0666);
    if (server_queue == -1)
    {
        perror("msgget");
        exit(1);
    }

    while (1)
    {
        pid_t child_pid;
        struct message msg;
        printf("waiting for msg  ...\n");
        if (msgrcv(server_queue, &msg, sizeof(msg.mtext), CLIENT_TYPE, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("Got msg %s %ld\n", msg.mtext, msg.mtype);

        child_pid = fork();
        if (child_pid == -1)
        {
            perror("fork");
            exit(1);
        }

        if (child_pid == 0)
        {
            handle_client_request(msg.sender_id, server_queue, msg.mtext);
        }
        else
        {
            continue;
        }
    }
    msgctl(server_queue, IPC_RMID, NULL);
    printf("Cleanup completed\n");
    exit(0);
    return 0;
}
