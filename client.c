#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>

#define SERVER_KEY 1234
#define CLIENT_TYPE 1000

struct message
{
  long mtype;
  long sender_id;
  char mtext[256];
};

int main()
{
  int client_id;
  int server_queue;
  key_t server_key;
  struct message msg;
  server_key = ftok("server.c", SERVER_KEY);
  server_queue = msgget(server_key, IPC_CREAT | 0666);

  if (server_queue == -1)
  {
    perror("msgget");
    exit(1);
  }

  printf("Enter Client-ID: ");
  scanf("%d", &client_id);

  while (1)
  {
    int choice;
    printf("Client %d Menu:\n", client_id);
    printf("1. Enter 1 to contact the Ping Server\n");
    printf("2. Enter 2 to contact the File Search Server\n");
    printf("3. Enter 3 to contact the File Word Count Server\n");
    printf("4. Enter 4 to exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    memset(msg.mtext, 0, sizeof(msg.mtext));
    msg.mtype = CLIENT_TYPE;
    msg.sender_id = client_id;
    char buff[256];
    switch (choice)
    {
    case 1:
      strcpy(msg.mtext, "1");
      strcat(msg.mtext, "PING");
      break;
    case 2:
      printf("Enter filename for File Search: ");
      scanf("%s", buff);
      strcpy(msg.mtext, "2");
      strcat(msg.mtext, buff);
      break;
    case 3:
      printf("Enter filename for File Word Count: ");
      scanf("%s", buff);
      strcpy(msg.mtext, "3");
      strcat(msg.mtext, buff);
      break;
    case 4:
      exit(0);
    default:
      printf("Invalid choice. Try again.\n");
      continue;
    }
    printf("Message sending...!\n");
    if (msgsnd(server_queue, &msg, sizeof(msg.mtext), 0) == -1)
    {
      perror("msgsnd");
      exit(1);
    }
    printf("Message sent!\n");
    if (msgrcv(server_queue, &msg, sizeof(msg.mtext), client_id, 0) == -1)
    {
      perror("msgrcv");
      exit(1);
    }
    printf("Server reply for Client %d:\n%s\n", client_id, msg.mtext);
  }

  return 0;
}
