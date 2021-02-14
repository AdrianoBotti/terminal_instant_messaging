#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

#define BUFFER_SIZE 2000
#define MAX_COMM_LENGTH 50

char username[BUFFER_SIZE];

void showMenu();
int requestCommand(char* command, int TCP_sock, int UDP_sock, char* IP_address, char* UDP_port);
int requestRegister(char* command, int TCP_sock, char* IP_address, char* UDP_port);
int requestDeregister(char* command, int TCP_sock);
int requestWho(char* command, int TCP_sock);
int requestSend(char* command, int TCP_sock, int UDP_sock);
int requestQuit(char* command, int TCP_sock);
int sendTCP(char* buffer_out, int sock);
int receiveTCP(char* buffer_out, int sock);
int sendUDP(char* buffer_out, int sock, struct sockaddr_in* cl_addr);
int receiveUDP(char* buffer_in, int sock, struct sockaddr_in* cl_addr);
int offlineMode(int TCP_sock);
int onlineMode(int TCP_sock, int UDP_sock);
