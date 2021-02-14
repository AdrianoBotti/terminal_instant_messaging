#include "../headers/msg_server.h"

int deregisterClient(char* username, int client_sock)
{
	int ret, seek_result;
	char IP_address[BUFFER_SIZE]; 
	char UDP_port[BUFFER_SIZE];		
	char buffer_out[BUFFER_SIZE];
	
	seek_result = seekEntry(username); // we check the status of the client
	if(seek_result == 1){
		printf(" !!! Error: deregisterClient() --> seekEntry()\n");
		return 1;	
	}	
	// the client is always already registered and online, 
	// due to the checks we have done client-side
	// 				seek_result = 2
	ret = removeEntry(username, IP_address, UDP_port);
	if(ret == 1 ){
		printf(" !!! Error: deregisterClient() --> removeEntry()\n");
		return 1;	
	}
	strcpy(buffer_out,"Client deregistered!");
	printf("%d:...%s\n", pid, buffer_out);
	ret = sendTCP(buffer_out, client_sock);
		if(ret != 0){
			printf(" !!! Error registerClient() --> sendTCP()\n");
			return 1;		
		}
	return 0;
}
