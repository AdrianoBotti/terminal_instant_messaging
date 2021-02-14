#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

#define MAX_QUEUED_CONN 10
#define BUFFER_SIZE 2000
#define MAX_CMD_LENGTH 50

int pid;
struct flock client_rdlock;
struct flock client_wrlock;
struct flock client_unlock;

int executeCommand(char* string, int client_sock);
int registerClient(char* username, int client_sock);
int deregisterClient(char* username, int client_sock);
int who(int client_sock);
int sendMessage(char* recipient, int client_sock);
int quit(char* username, int client_sock);
int receiveTCP(char* buffer_in, int sock);
int sendTCP(char* buffer_out, int sock);
int seekEntry(char* username);
int removeEntry(char* username, char* out_IP_address, char* out_UDP_port);
int setStatus(char* username, char* status);
int getClientInfo(char* IP_address, char* UDP_port, int client_sock);
int insertNewEntry(char* username, char* IP_address, char* UDP_port);
int sendMessageOffline(char* recipient, int client_sock);
int sendMessageOnline(char* recipient, int client_sock);
int queuedMessages(char* username, char* buffer_out);
