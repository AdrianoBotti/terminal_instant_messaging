#include "../headers/msg_server.h"

int main(int argc, char* argv[])
{
	int connection_sock, client_sock, ret; // optval = 1;
	char buffer_in[BUFFER_SIZE];
	struct sockaddr_in sv_addr, cl_addr;	
	socklen_t cl_len;	
	
	if(argc != 2){
		printf(" !!! Error: incorrect number of arguments. \nCorrect sintax : ./msg_server <listening_TCP_port>\n");
		return 1;	
	}
	printf(" - Server starting...\n");
	
	//initializing structures for the server socket
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(atoi(argv[1]));
	sv_addr.sin_addr.s_addr = INADDR_ANY;
	
	//initializing structures flock
	//read lock
	memset(&client_rdlock, 0, sizeof client_rdlock);
	client_rdlock.l_type = F_RDLCK;
	client_rdlock.l_whence = SEEK_SET;
	client_rdlock.l_start = 0;
	client_rdlock.l_len = 0; 
	//write lock
	memset(&client_wrlock, 0, sizeof client_wrlock);
	client_wrlock.l_type = F_WRLCK;
	client_wrlock.l_whence = SEEK_SET;
	client_wrlock.l_start = 0;
	client_wrlock.l_len = 0; 
	//unlock
	memset(&client_unlock, 0, sizeof client_unlock);
	client_unlock.l_type = F_UNLCK;
	client_unlock.l_whence = SEEK_SET;
	client_unlock.l_start = 0;
	client_unlock.l_len = 0; 


	printf(" - Creating TCP socket...\n");
	if(	(connection_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror(" !!! socket() ");
		return 1;	
	} 
	/* 	Not working in this case. From man socket(7) for SO_REUSEADDR:	*/
	/*
		[...]a socket may bind, except  when there  is an active 
		listening socket bound to the address. When the listening socket is bound 
		to INADDR_ANY with a specific port, then  it  is  not  possible  to  bind 
		to this port for any local address.
	*/
	/*
	if( (setsockopt(connection_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int))) == -1){
		perror(" !!! setsockopt()");
		close(connection_sock);
		return 1;	
	}
	*/
	printf(" - Binding socket to %s port on all local interfaces...\n", argv[1]);
	if( bind(connection_sock, (struct sockaddr*)&sv_addr, sizeof(sv_addr)) == -1 ){		
		perror(" !!! bind() ");
		close(connection_sock);
		return 1;		
	}

	if( listen(connection_sock, MAX_QUEUED_CONN) == -1 ){
		perror(" !!! listen() ");
		close(connection_sock);
		return 1;		
	} 
	printf(" - Server listening on socket\n");
	
	while(1)
	{
		// Preparing the structures for the new client
		cl_len = sizeof(cl_addr);
		memset(&cl_addr, 0, cl_len);
		
		printf("--- Listening process ready to accept a new connection request ---\n");
		if( (client_sock = accept(connection_sock, (struct sockaddr*)&cl_addr, 
														&cl_len)) == -1 ){
			perror(" !!! accept() ");
			break;
		}

		// A new request has arrived
		printf("-> A new client is connected!\n");
		pid = fork();
		if(pid == -1){
			perror(" !!! fork() ");
			close(client_sock);
			break;		
		}		
		if(pid == 0){ // child process
			close(connection_sock);			
			while(1)
			{
				pid = getpid();
				printf("%d:...Waiting command from client...\n", pid);
				// receiving new command from client
				ret = receiveTCP(buffer_in, client_sock);
				if(ret != 0 )
					break;

				// here the commands get recognized and executed 
				ret = executeCommand(buffer_in, client_sock);
				if(ret == 1 ){
					printf("%d: !!! Error: executeCommand() exit with status 1\n", pid);
					break;
				}
				if(ret == 2){
					printf("%d: --- Closing socket TCP and terminating server child process ---\n", pid);
					break;	
				}
			}
			close(client_sock);
			return 0;

		}else{// parent process
			close(client_sock);
		}
	}
	close(connection_sock);
	return 0;
}
