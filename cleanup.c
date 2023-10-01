#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

struct message
{
    long mtype;
    long sender_id;
    char mtext[256];
};

int main()
{
    key_t server_key;
    int server_queue;
    server_key = ftok("server.c", 1234);
    server_queue = msgget(server_key, 0666);

    if (server_queue == -1)
    {
        perror("msgget");
        exit(1);
    }

    while (1)
    {
        char choice;
        printf("Do you want the server to terminate? Press Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y')
        {
            struct message msg;
            msg.mtype = 1000;
            msg.sender_id = -1;
            strcpy(msg.mtext, "4");
            if (msgsnd(server_queue, &msg, sizeof(msg.mtext), 0) == -1)
            {
                perror("msgsnd");
                exit(1);
            }
            printf("Termination request sent to the server. Exiting cleanup process.\n");
            exit(0);
        }
        else if (choice == 'N' || choice == 'n')
        {
            continue;
        }
        else
        {
            printf("Invalid choice. Please enter Y or N.\n");
        }
    }

    return 0;
}
