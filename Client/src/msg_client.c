#include "../headers/msg_client.h"
	
int main(int argc, char *argv[])
{
	int TCP_sock, UDP_sock, ret;
	char* input_str;
	char buffer_out[BUFFER_SIZE];
	char buffer_in[BUFFER_SIZE];
	struct sockaddr_in sv_addr, my_addr, peer_addr;
	fd_set master, readfd;
	
	
	if(argc != 5){
		printf("Error: incorrect number of arguments. \nCorrect sintax : ./msg_client <client_IP> <client_UDP_port> <server_IP> <server_TCP_port>\n");
		return 1;	
	}	

	//inizialing structures for TCP connection with the server
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(atoi(argv[4]));
	inet_pton(AF_INET, argv[3], &sv_addr.sin_addr);
	
	if( (TCP_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("TCP socket() ");
		return 1;	
	} 
	
	if( (ret = connect(TCP_sock, (struct sockaddr*)&sv_addr, sizeof(sv_addr))) == -1 ){
		perror("connect() ");
		close(TCP_sock);
		return 1;
	}

	// initializing structures for UDP connections
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &my_addr.sin_addr);
	
	if( (UDP_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("UDP socket() ");
		return 1;	
	} 
	if( (ret = bind(UDP_sock, (struct sockaddr*)&my_addr, sizeof(my_addr) )) == -1 ){
		perror("UDP bind() ");
		return 1;	
	} 
	
	// connection successful, showing menu options
	printf("\nConnection to server on IP %s , port %s successful!\nElected port %s for incoming instant messages\n\n", argv[3], argv[4], argv[2]);
	showMenu();
	
	// setting synchronous I/O on UDP socket and user input
	FD_ZERO(&master);
    FD_ZERO(&readfd);
	FD_SET(UDP_sock, &readfd);
	FD_SET(0, &readfd);
	master = readfd;

	setbuf(stdout, NULL); // unbuffering the stdout
	while(1)
	{
		while(1) // getting a command or a new istant message
		{
			printf("%s > ", username);
			ret = select(UDP_sock+1, &readfd, NULL, NULL, NULL);
			if (ret == -1) {
            	perror("select()");
           		return 1;
       		}
			if ( FD_ISSET(0, &readfd) ){
				readfd = master;
				memset(buffer_out, 0, BUFFER_SIZE);
				input_str = fgets(buffer_out, BUFFER_SIZE, stdin);
				if(input_str == NULL){
					printf("Error main --> fgets()\n ");	
					continue;
				}
				break;
			}
			if (FD_ISSET(UDP_sock, &readfd) ){
				readfd = master;	
				ret = receiveUDP(buffer_in, UDP_sock, &peer_addr);	
				printf("%s\n", buffer_in);			
			}
		}

		// removing heading blank spaces
		while(*input_str == ' ')
			input_str++;
				
		strcpy(buffer_out, input_str);

		//if command is "!help", we serve it client side	
		if(strcmp("!help\n", buffer_out) == 0){
			showMenu();		
			continue;
		}
		// recognize command and send it
		ret = requestCommand(buffer_out, TCP_sock, UDP_sock, argv[1], argv[2]);
		if(ret == 1){
			printf("Error requestCommand()\n");
			break;		
		}
		if(ret == 2){
			printf("Closing socket TCP and terminating client process...\n");
			break;	
		}
	}
	close(TCP_sock);
	printf("See you next time! \n");
	return 0;
}
