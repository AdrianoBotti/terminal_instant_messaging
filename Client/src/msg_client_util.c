#include "../headers/msg_client.h"

void showMenu()
{
	printf("The following commands are available to be executed: \n\n");	
	printf(" -- !help --> shows available commands.\n");
	printf(" -- !register <username>--> registers a new user or logs in as an existing one.\n");
	printf(" -- !deregister --> deregisters current user.\n");
	printf(" -- !who --> shows the list of registered users.\n");
	printf(" -- !send <username> --> sends a message to any registered user.\n 			If the recipient is offline, the message will be\n 			delivered on its next reconnection.\n");
	printf(" -- !quit --> disconnects from server.\n\n");

	return;
}

int requestCommand(char* command, int TCP_sock, int UDP_sock, char* IP_address, char* UDP_port)
{
	char parsedCommand[BUFFER_SIZE];	
	char* argument;
	int i, ret;	
	const char commands[5][MAX_COMM_LENGTH] = {	"!register",
											"!deregister",
											"!who",
											"!send",
											"!quit" };	

	//removing newline character at end of command
	strtok(command, "\n");

	strcpy(parsedCommand, command);

	//parsing string for command and argument	
	strtok(parsedCommand, " ");
	argument = strtok(NULL, " ");
	
	// searching for command index
	for(i = 0; i<5; i++){
		if( strcmp(parsedCommand, commands[i]) == 0 )
			break;	
	}
	switch(i){
		case 0: // registration
			if(argument == NULL){
				printf("Missing username\n");
				ret = 0;
				break;
			}
			if(strcmp(username, "") != 0){
				printf("You need to be unregistered to issue this command\n");
				ret = 0;
				break;			
			}
			ret = requestRegister(command, TCP_sock, IP_address, UDP_port);
			break;
		case 1: // deregistration
			if(strcmp(username, "") == 0){
				printf("You need to be registered to issue this command\n");
				ret = 0;
				break;			
			}
			//you can only deregister yourself
			printf("You can only deregister yourself.\nAny additional parameters to this command will be ignored.\n\n");
			ret = requestDeregister(parsedCommand, TCP_sock);
			break;
		case 2:	// registered clients list
			if(strcmp(username, "") == 0){
				printf("You need to be registered to issue this command\n");
				ret = 0;
				break;
			}
			ret = requestWho(command, TCP_sock);
			break;
		case 3: // send a message
			if(strcmp(username, "") == 0){
				printf("You need to be registered to issue this command\n");
				ret = 0;
				break;			
			}			
			ret = requestSend(command, TCP_sock, UDP_sock);
			break;
		case 4:	// quitting connection
				ret = requestQuit(parsedCommand, TCP_sock);
			break;
		default:
			printf("Unrecognized command\n");
			ret = 0;
			break;	
	}

	return ret;
}

int receiveTCP(char* buffer_in, int sock)
{
	uint16_t buff_size;	
	int ret;

	// Acknowledging client's command length 
	ret = recv(sock, (void*)&buff_size, sizeof(uint16_t), 0);
	if(ret == 0){
		printf("Server closed the connection. Closing client_sock.\n");
		return 1;	
	}		
	if(ret == -1){
		perror("Length recv()");
		return 1;
	}			
	if(ret != sizeof(uint16_t)){
		printf("Warning: received %i bytes, instead of %lu\n", ret,sizeof(uint16_t));
		return 1;
	}
	
	buff_size = ntohs(buff_size);
	// Receving actual command
	memset(buffer_in, 0, BUFFER_SIZE);
	ret = recv(sock, (void*)buffer_in, buff_size, 0);
	if(ret == -1){
		perror("Command recv()");
		return 1;	
	}			
	if(ret != buff_size){
		printf("Warning: received %i bytes, instead of %d\n", ret, buff_size);
		return 1;
	}
	return 0;
}

int sendTCP(char* buffer_out, int sock)
{
	uint16_t buff_size;	
	int ret;
	
	//sending message length
	buff_size = htons(strlen(buffer_out));
	ret = send(sock, (void*)&buff_size, sizeof(uint16_t), 0);
	if(ret == -1){
		perror("Length send()");
		return 1;	
	}			
	if(ret != sizeof(uint16_t)){
		printf("Warning: sent %i bytes, instead of %lu\n", ret, sizeof(uint16_t));
		return 1;
	}

	//sending actual command
	buff_size = ntohs(buff_size);
	ret = send(sock, (void*)buffer_out, buff_size, 0);
	if(ret == -1){
		perror("Command send()");
		return 1;	
	}			
	if(ret != buff_size){
		printf("Warning: sent %i bytes, instead of %d\n", ret, buff_size);
		return 1;
	}	
	return 0;
}

int receiveUDP(char* buffer_in, int sock, struct sockaddr_in* peer_addr)
{
	uint16_t buff_size;	
	int ret;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof(*peer_addr);
	// Acknowledging client's command length 
	ret = recvfrom(sock, (void*)&buff_size, sizeof(uint16_t), 0, 
									(struct sockaddr*)peer_addr, &peer_addr_len);
	if(ret == 0){
		printf("Received 0 bytes from peer.\n");
		return 1;	
	}		
	if(ret == -1){
		perror("Length recvfrom()");
		return 1;
	}			
	if(ret != sizeof(uint16_t)){
		printf("Warning: received %i bytes, instead of %lu\n", ret, sizeof(uint16_t));
		return 1;
	}
	
	buff_size = ntohs(buff_size);
	// Receving actual command
	memset(buffer_in, 0, BUFFER_SIZE);
	ret = recvfrom(sock, (void*)buffer_in, buff_size, 0,
									(struct sockaddr*)peer_addr, &peer_addr_len);
	if(ret == -1){
		perror("Command recvfrom() ");
		return 1;	
	}			
	if(ret != buff_size){
		printf("Warning: received %i bytes, instead of %d\n", ret, buff_size);
		return 1;
	}
	return 0;
}

int sendUDP(char* buffer_out, int sock, struct sockaddr_in* peer_addr)
{
	uint16_t buff_size;	
	int ret;
	socklen_t peer_addr_len;

	peer_addr_len = sizeof(*peer_addr);	
	//sending message length
	buff_size = htons(strlen(buffer_out));
	ret = sendto(sock, (void*)&buff_size, sizeof(uint16_t), 0,
								(struct sockaddr*)peer_addr, peer_addr_len);
	if(ret == -1){
		perror("Length sendto() ");
		return 1;	
	}			
	if(ret != sizeof(uint16_t)){
		printf("Warning: sent %i bytes, instead of %lu\n", ret, sizeof(uint16_t));
		return 1;
	}

	//sending actual command
	buff_size = ntohs(buff_size);
	ret = sendto(sock, (void*)buffer_out, buff_size, 0,
								(struct sockaddr*)peer_addr, peer_addr_len);
	if(ret == -1){
		perror("Command sendto() ");
		return 1;	
	}			
	if(ret != buff_size){
		printf("Warning: sent %i bytes, instead of %d\n", ret, buff_size);
		return 1;
	}	
	return 0;
}

