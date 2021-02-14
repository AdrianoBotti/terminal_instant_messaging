#include "../headers/msg_server.h"

int executeCommand(char* command, int client_sock)
{
	char* argument = NULL;
	int i, ret;	
	const char commands[5][MAX_CMD_LENGTH] = {	"!register",
											"!deregister",
											"!who",
											"!send",
											"!quit" };

	strtok(command, " ");	// parsing string for command and argument	
	argument = strtok(NULL, " ");
	
	// searching for command's index
	for(i = 0; i<5; i++){
		if( strcmp(command, commands[i]) == 0 )
			break;	
	}
	printf("%d:-> Command issued: %s %s\n", pid, command, argument);
	switch(i){
		case 0:
			ret = registerClient(argument, client_sock);
			break;
		case 1:
			ret = deregisterClient(argument, client_sock);
			break;
		case 2:
			ret = who(client_sock);
			break;
		case 3:
			ret = sendMessage(argument, client_sock);
			break;
		case 4:
			ret = quit(argument, client_sock);
			break;
		default:
			printf("%d: !!! Error: unrecognized command\n", pid);
			ret = 1;
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
		printf("%d:-> Client closed the connection. Closing TCP_sock.\n", pid);
		return 1;	
	}		
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! Length recv(): ");
		return 1;
	}			
	if(ret != sizeof(uint16_t)){
		printf("%d: !!! Warning: received %i bytes, instead of %lu\n", pid, ret, sizeof(uint16_t));
		return 1;
	}
	
	buff_size = ntohs(buff_size);
	// Receving actual command
	memset(buffer_in, 0, BUFFER_SIZE);
	ret = recv(sock, (void*)buffer_in, buff_size, 0);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! Command recv() ");
		return 1;	
	}			
	if(ret != buff_size){
		printf("%d: !!! Warning: received %i bytes, instead of %d\n", pid, ret, buff_size);
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
		fprintf(stderr, "%d:", pid);
		perror(" !!! Length send() ");
		return 1;	
	}			
	if(ret != sizeof(uint16_t)){
		printf("%d: !!! Warning: sent %i bytes, instead of %lu\n", pid, ret, sizeof(uint16_t));
		return 1;
	}

	//sending actual command
	buff_size = ntohs(buff_size);
	ret = send(sock, (void*)buffer_out, buff_size, 0);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! Command send() ");
		return 1;	
	}			
	if(ret != buff_size){
		printf("%d: !!! Warning: sent %i bytes, instead of %d\n", pid, ret, buff_size);
		return 1;
	}	
	return 0;
}

int seekEntry(char* in_user)
{
	int ret, filedes;
	char username[BUFFER_SIZE];
	char IP_address[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	char status[BUFFER_SIZE];
	FILE* fd;
	
	fd = fopen("./src/txt/clients.txt", "r");
	if(fd == NULL){
		printf("%d: !!! Error: registerClient --> fopen()\n", pid);
		return 1;
	}
	//acquiring the lock
	filedes = fileno(fd);
	ret = fcntl(filedes, F_SETLKW, &client_rdlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! seekEntry() --> fcntl()");
		fclose(fd);
		return 1;	
	}
	while(1)
	{
		ret = fscanf(fd, " %s %s %s %s", username, IP_address, UDP_port, status );
		if(ret == EOF){
			//new client
			printf("%d:...Client %s not found\n", pid, in_user);
			fcntl(filedes, F_SETLKW, &client_unlock);			
			fclose(fd);
			return 0;		
		}
		if(ret != 4){
			printf("%d: !!! Error: seekEntry --> fscanf ret = %d\n", pid, ret);	
			fcntl(filedes, F_SETLKW, &client_unlock);
			fclose(fd);
			return 1;		
		}
		if(strcmp(username, in_user) == 0){
			//client already registered
			if(strcmp(status, "online") == 0){
				//client already online
				printf("%d:...Client %s online\n", pid, in_user);
				fcntl(filedes, F_SETLKW, &client_unlock);
				fclose(fd);
				return 2;
			}else{
				printf("%d:...Client %s offline\n", pid, in_user);
				fcntl(filedes, F_SETLKW, &client_unlock);
				fclose(fd);
				return 3;			
			}
		}
	}
	return 0;
}

int removeEntry(char* in_username, char* IP_address_out, char* UDP_port_out)
{
	int ret, wr_len, filedes, filedes1;
	char username[BUFFER_SIZE];
	char status[BUFFER_SIZE];
	char IP_address[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	FILE *fd, *fd1;
	
	//old file, to be eliminated after copy
	fd = fopen("./src/txt/clients.txt", "r");
	if(fd == NULL){
		fprintf(stderr, "%d:", pid);
		perror(" !!! removeEntry() --> fopen(fd)");
		return 1;
	}
	filedes = fileno(fd);
	ret = fcntl(filedes, F_SETLKW, &client_rdlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! removeEntry() --> fcntl(filedes2)");
		fclose(fd);
		return 1;
	}

	//new file
	fd1 = fopen("./src/txt/clients1.txt", "w");
	if(fd1 == NULL){
		fprintf(stderr, "%d:", pid);
		perror(" !!! removeEntry() --> fopen(fd1)");
		fcntl(filedes, F_SETLKW, &client_unlock);		
		fclose(fd);
		return 1;
	}
	filedes1 = fileno(fd1);	
	ret = fcntl(filedes1, F_SETLKW, &client_wrlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! removeEntry() --> fcntl(fildes1)");	
		fcntl(filedes, F_SETLKW, &client_unlock);		
		fclose(fd);	
		fclose(fd1);
		return 1;
	}
	while(1)
	{
		ret = fscanf(fd, " %s %s %s %s\n", username, IP_address, UDP_port, status);
		if(ret == EOF){
			break;	
		}		
		if(ret != 4){
			fcntl(filedes, F_SETLKW, &client_unlock);
			fcntl(filedes1, F_SETLKW, &client_unlock);
			fclose(fd);
			fclose(fd1);
			return 1;		
		}
		if(strcmp(in_username, username) == 0){
			strcpy(IP_address_out, IP_address);
			strcpy(UDP_port_out, UDP_port);
			continue;		
		}
		
		wr_len = strlen(username) + strlen(IP_address) + strlen(UDP_port) + strlen(status);
		ret = fprintf(fd1, "%s %s %s %s\n", username, IP_address, UDP_port, status);
		if( ret != (wr_len + 4) || ret < 0){
			printf("%d: !!! Error: removeEntry --> fprintf(): ret = %d \n",pid, ret);
			fcntl(filedes, F_SETLKW, &client_unlock);
			fcntl(filedes1, F_SETLKW, &client_unlock);
			fclose(fd);
			fclose(fd1);
			return 1;	
		}	
	}

	//removing old file
	if( remove( "./src/txt/clients.txt" ) != 0 ){
		fprintf(stderr, "%d:", pid);
    	perror( " !!! Error deleting old clients.txt" );
	}
  	else
    	printf("%d:...Old clients.txt successfully deleted\n", pid );
	
	//renaming new file as old file
 	if ( rename ("./src/txt/clients1.txt", "./src/txt/clients.txt") != 0 ){
		fprintf(stderr, "%d:", pid);
   		perror( " !!! Error renaming clients1.txt" );
	}
	else
		printf("%d:...Clients1.txt successfully renamed\n", pid);

	fcntl(filedes, F_SETLKW, &client_unlock);
	fcntl(filedes1, F_SETLKW, &client_unlock);
	fclose(fd);
	fclose(fd1);
  	return 0;
}

int setStatus(char* username, char* status)
{
	//first we remove the client completely, then we add it again with 
	//different status
	int ret, wr_len, filedes;
	char IP_address[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	FILE *fd;
	
	ret = removeEntry(username, IP_address, UDP_port);
	if(ret == 1 ){
		printf("%d: !!! Error: setStatus() --> removeEntry()\n", pid);
		return 1;	
	}

	fd = fopen("./src/txt/clients.txt", "a");
	if(fd == NULL){
		printf("%d: !!! Error: setStatus() --> fopen()\n", pid);
		return 1;
	}
	filedes = fileno(fd);
	ret = fcntl(filedes, F_SETLKW, &client_wrlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! setStatus() --> fcntl()");	
		fclose(fd);	
		return 1;
	}	
	
	wr_len = strlen(username) + strlen(IP_address) + strlen(UDP_port) + strlen(status);
	ret = fprintf(fd, "%s %s %s %s\n", username, IP_address, UDP_port, status);
	if( ret != (wr_len + 4) || ret < 0){
		printf("%d:  !!! Error: setStatus() --> fprintf() ret = %d \n", pid, ret);
		fclose(fd);
		fcntl(filedes, F_SETLKW, &client_unlock);	
		return 1;	
	}
	printf("%d:...Client %s is now %s\n", pid, username, status);
	fcntl(filedes, F_SETLKW, &client_unlock);
	fclose(fd);
	return 0;
}
