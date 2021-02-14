#include "../headers/msg_server.h"

int sendMessage(char* recipient, int client_sock)
{
	int ret, seek_result;	
	char buffer_out[BUFFER_SIZE];
	
	seek_result = seekEntry(recipient); // we check if client is registered
	if(seek_result == 1){
		printf("%d: !!! Error: sendMessage() --> seekEntry()\n", pid);
		return 1;	
	}
	
	// recipient does not exist	
	if(seek_result == 0){
		strcpy(buffer_out, " !!! Error: username not found");
		printf("%d:%s\n", pid, buffer_out);
		ret = sendTCP(buffer_out, client_sock);
		if(ret != 0){
			printf("%d: !!! Error: sendMessage() --> sendTCP()\n", pid);
			return 1;		
		}
		return 0;
	}
	// recipient exists and is online
	if(seek_result == 2){
		printf("%d:...sending message in online mode\n", pid);
		ret = sendMessageOnline(recipient, client_sock);	
	}
	// recipient exists, but is offline
	if(seek_result == 3){
		printf("%d:...sending message in offline mode\n", pid);
		ret = sendMessageOffline(recipient, client_sock);
	}	
	return 0;
}
	
int sendMessageOffline(char* recipient, int client_sock)
{
	int ret, wr_len, filedes;
	char buffer_in[BUFFER_SIZE];
	char buffer_out[BUFFER_SIZE];
	char path[BUFFER_SIZE];
	FILE* fd;
	
	// asking client if message has to be sent as an offline message
	strcpy(buffer_out, "-- Recipient offline, do you want to send an offline message?[y/n]");	
	ret = sendTCP(buffer_out, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: sendMessageOffline() --> sendTCP()\n", pid);
		return 1;		
	}
	// receiving answer from client
	ret = receiveTCP(buffer_in, client_sock);
	if( ret != 0){
		printf("%d: !!! Error: sendMessageOffline() --> receiveTCP(1)\n", pid);
		return 1;		
	}

	// if no
	if(strcmp(buffer_in, "n\n") == 0){
		printf("%d:-> Client refused to send an offline message\n", pid);
		return 0;	
	}
	// if yes
	if(strcmp(buffer_in, "y\n") == 0){
		printf("%d:-> Client accepted offline mode\n", pid);
		memset(buffer_in, 0, BUFFER_SIZE);
		ret = receiveTCP(buffer_in, client_sock);
		if( ret != 0){
			printf("%d: !!! Error: sendMessageOffline() --> receiveTCP(2)\n", pid);
			return 1;		
		}
		
		// saving the message in recipient.txt
		strcpy(path, "./src/txt/");
		strcat(path, recipient);
		strcat(path, ".txt");	
		
		fd = fopen(path, "a");
		if(fd == NULL){
			printf("%d: !!! Error: sendMessageOffline() --> fopen()\n", pid);
			return 1;
		}
		filedes = fileno(fd);
		ret = fcntl(filedes, F_SETLKW, &client_wrlock);
		if(ret == 1){
			fprintf(stderr, "%d:", pid);
			perror("!!! sendMessageOnline() --> fcntl()");
			fclose(fd);
			return 1;		
		}		
		
		wr_len = strlen(buffer_in);	
		ret = fprintf(fd, "%s§\n", buffer_in);
		if( ret != (wr_len+3) || ret < 0){ // 3 = '§'+'\n'+'\0'
			printf("%d: !!! Error: sendMessageOffline() --> fprintf() ret = %d\n", pid, ret);
			fclose(fd);
			return 1;	
		}
	fcntl(filedes, F_SETLKW, &client_unlock);
	fclose(fd);
	}
	return 0;
}

int sendMessageOnline(char* recipient, int client_sock)
{
	int ret, filedes;
	char username[BUFFER_SIZE];
	char IP_address[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	char status[BUFFER_SIZE];
	char buffer_out[BUFFER_SIZE];
	FILE* fd;
	
	// we tell the client that recipient's infos are going to be sent soon
	printf("%d:...Sending recipient's infos...\n",pid );
	strcpy(buffer_out, "-- Sending recipient's infos...");	
	ret = sendTCP(buffer_out, client_sock);
	if(ret != 0){ 
		printf("%d: !!! Error: sendMessageOnline() --> sendTCP(1)\n", pid);
		return 1;		
	}	
	// we obtain client's info from file
	
	fd = fopen("./src/txt/clients.txt", "r");
	if(fd == NULL){
		printf("%d: !!! Error: sendMessageOnline() --> fopen()\n", pid);
		return 1;
	}
	filedes = fileno(fd);
	ret = fcntl(filedes, F_SETLKW, &client_rdlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! sendMessageOnline() --> fcntl()");
		fclose(fd);	
		return 1;
	}	

	while(1)
	{
		ret = fscanf(fd, " %s %s %s %s\n", username, IP_address, UDP_port, status);
		if(strcmp(username, recipient) == 0)
			break;
	}
	fcntl(filedes, F_SETLKW, &client_unlock);
	fclose(fd);

	printf("%d: %s %s %s %s\n", pid, username, IP_address, UDP_port, status);
	// send ip 
	ret = sendTCP(IP_address, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: sendMessageOnline() --> sendTCP(2)\n", pid);
		return 1;		
	}
	// send udp port 
	ret = sendTCP(UDP_port, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: sendMessageOnline() --> sendTCP(3)\n", pid);
		return 1;		
	}
	
	printf("%d:...Informations sent to requesting client!\n",pid );
	return 0;
}
