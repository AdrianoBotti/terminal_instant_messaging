#include "../headers/msg_server.h"

int registerClient(char* username, int client_sock)
{
	int ret, seek_result;
	char buffer_out[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	char IP_address[BUFFER_SIZE];
	
	seek_result = seekEntry(username); // we check if client is already registered
	if(seek_result == 1){
		printf("%d: !!! Error: registerClient() --> seekEntry()\n", pid);
		return 1;	
	}	
	
	// client already registered and online
	if( seek_result == 2)
		strcpy(buffer_out, "Client already online");

	// client offline
	if( seek_result == 3)
	{	
		printf("%d:-> Updating client's info...\n", pid);	
		ret = removeEntry(username, IP_address, UDP_port);	
		if(ret == 1){
			printf("%d: !!! Error: registerClient() --> removeEntry(3)\n", pid);
			return 1;		
		}	
		ret = getClientInfo(IP_address, UDP_port, client_sock);
		if(ret == 1){
			printf("%d: !!! Error: registerClient() --> getClientInfo(3)\n", pid);
			return 1;		
		}
	
		ret = insertNewEntry(username, IP_address, UDP_port);
		if(ret == 1){
			printf("%d: !!! Error: registerClient() --> insertNewEntry(3)\n", pid);
			return 1;		
		}
		
		printf("%d:...Client is now online\n", pid)	;
		strcpy(buffer_out, "You are now online!");
		// sending any offline message received	
		queuedMessages(username, buffer_out);
	}

	// new client
	if( seek_result == 0 )
	{
		printf("%d:...Registering new client...\n", pid);
		ret = getClientInfo(IP_address, UDP_port, client_sock);
		if(ret == 1){
			printf("%d: !!! Error: registerClient() --> getClientInfo(0)\n", pid);
			return 1;		
		}
		ret = insertNewEntry(username, IP_address, UDP_port);
		if(ret == 1){
			printf("%d: !!! Error: registerClient() --> insertNewEntry(0)\n", pid);
			return 1;		
		}
	
		printf("%d:-> Client %s has been registered!\n", pid, username);
		strcpy(buffer_out, "Registration successful!");
	}
	
	ret = sendTCP(buffer_out, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: registerClient() --> sendTCP()\n", pid);
		return 1;		
	}
	return 0;
}
	
int getClientInfo(char* IP_address_out, char* UDP_port_out, int client_sock)
{
	int ret;
	char buffer_out[BUFFER_SIZE];
	
	// first we need to tell the client that the command is valid
	// and that they need to proceed sending IP and Port
	
	strcpy(buffer_out, "Waiting for informations");		
	ret = sendTCP(buffer_out, client_sock);
	if(ret != 0){
		printf("%d: !!! Error: getClientInfo() --> sendTCP()\n", pid);
		return 1;		
	}
	// receiving IP	
	ret = receiveTCP(IP_address_out, client_sock);
	if( ret != 0){
		printf("%d: !!! Error: getClientInfo() --> receiveTCP(1)\n", pid);
		return 1;		
	}
	// receiving UDP port
	ret = receiveTCP(UDP_port_out, client_sock);
	if( ret != 0){
		printf("%d: !!! Error: getClientInfo() --> receiveTCP(2)\n", pid);
		return 1;		
	}

	return 0;
}

int insertNewEntry(char* username, char* IP_address, char* UDP_port)
{
		int ret, wr_len, filedes;
		char status[BUFFER_SIZE];		
		FILE* fd;
	
		//saving informations into file "clients.txt"

		fd = fopen("./src/txt/clients.txt", "a");
		if(fd == NULL){
			printf("%d: !!! Error: InsertNewEntry() --> fopen()\n", pid);
			return 1;
		}
		filedes = fileno(fd);
		ret = fcntl(filedes, F_SETLKW, &client_wrlock);
		if(ret == -1){
			fprintf(stderr, "%d:", pid);
			perror(" !!! insertNewEntry() ---> fcntl()");		
			fclose(fd);	
			return 1;
		}
		/* 	Test code for writelock functionality 	*/	
		/*	Both clients issue !register command	*/
		/*
		srand(pid);
		int x = rand() %10;		
		printf("%d sleeping now for %d sec\n",pid, x);
		sleep(x);
		printf("%d awake now\n", pid);
		*/
	
		strcpy(status, "online");	// initializing status online by default		
		wr_len = strlen(username) + strlen(IP_address) + strlen(UDP_port) + strlen(status);
		
		ret = fprintf(fd, "%s %s %s %s\n", username, IP_address, UDP_port, status);
		if( ret != (wr_len + 4) || ret < 0){
			printf("%d: !!! Error: insertNewEntry() --> fprintf() ret = %d \n", pid, ret);
			fcntl(filedes, F_SETLKW, &client_unlock);	
			fclose(fd);
			return 1;	
		}
		fcntl(filedes, F_SETLKW, &client_unlock);	
		fclose(fd);
		return 0;		
}
	
int queuedMessages(char* username, char buffer_out[])
{
	char path[BUFFER_SIZE];
	char* sender;
	char* body;
	char textline[BUFFER_SIZE];
	FILE* fd;
	
	strcpy(path, "./src/txt/");
	strcat(path, username);
	strcat(path, ".txt");
	fd = fopen(path, "r"); // no need to lock, concurrent access is unlikely to happen
	if(fd == NULL){
		fprintf(stderr, "%d:", pid);
		perror(" !!! queuedMessage() --> fopen()");
		strcat(buffer_out, "\nThere are no unread messages\n");
		printf("%d:There are no unread messages for %s\n", pid, username);
		return 0;
	}
	
	sender = fgets(textline, BUFFER_SIZE, fd);
	if(sender == NULL){
		return 0;	
	}else{		
		strcat(buffer_out, "\n !-- There are unread messages! --!\n\n");
		printf("%d:There are unread messages for %s\n", pid, username);				
		while(sender != NULL)
		{
			strtok(sender, ":");
			strcat(buffer_out, sender);
			strcat(buffer_out, " said (offline msg):\n");
			body = fgets(textline, BUFFER_SIZE, fd);
			while( strcmp(body, "§\n") != 0 )
			{
				strcat(buffer_out, " - ");				
				strcat(buffer_out, body);
				body = fgets(textline, BUFFER_SIZE, fd);
			}
			strcat(buffer_out, "\n");
			sender = fgets(textline, BUFFER_SIZE, fd);
		}	
		strcat(buffer_out, " !-- End of messages. --! \n");
	}
	fclose(fd);
	
	//deleting file with already delivered messages
	if( remove( path ) != 0 ){
		fprintf(stderr, "%d:", pid);
    	perror(" !!! Error: deleting username.txt" );
	}
  	else
    	printf("%d:...Username.txt successfully deleted\n", pid );	
	return 0;
}
