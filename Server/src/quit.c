#include "../headers/msg_server.h"

int quit(char* username, int client_sock)
{
	int ret;
	char buffer_out[BUFFER_SIZE];
	char buffer_in[BUFFER_SIZE];
	
	if(username != NULL){
		ret = setStatus(username, "offline");
		if(ret == 1){
			printf("%d: !!! Error: quit() --> setStatus()\n", pid);
			return 1;		
		}
	}
	// sending "waiting for closure"  notification
	strcpy(buffer_out, "ok");
	ret = sendTCP(buffer_out, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: quit() --> sendTCP()\n", pid);
		return 1;		
	}
	
	// waiting for close(TCP_sock), receiving 0
	ret = recv(client_sock, (void*)buffer_in, sizeof(int), 0);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! Command recv() ");
		return 1;
	}
	if(ret == 0){
		printf("%d:-> Received 0 from client, terminating connection\n", pid);
		return 2;	
	}
	return 0;		
}


