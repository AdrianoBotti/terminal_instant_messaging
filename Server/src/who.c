#include "../headers/msg_server.h"
	
int who(int client_sock)
{
	int ret, scan_ret, filedes;
	char buffer_out[BUFFER_SIZE];	
	char username[BUFFER_SIZE];
	char UDP_port[BUFFER_SIZE];
	char IP_address[BUFFER_SIZE];
	char status[BUFFER_SIZE];
	FILE* fd;	
	
	fd = fopen("./src/txt/clients.txt", "r");
	if(fd == NULL){
		fprintf(stderr, "%d:", pid);
		perror(" !!! who() --> fopen() ");	
		return 1;
	}
	filedes = fileno(fd);
	ret = fcntl(filedes, F_SETLKW, &client_rdlock);
	if(ret == -1){
		fprintf(stderr, "%d:", pid);
		perror(" !!! who() --> fcntl()");
		fclose(fd);
		return 1;
	}
	/*	Test code for readlock functionality	*/
	/*	First client issues !who command, 		*/
	/*	second issues !register					*/
	/*sleep(5);*/
	
	while(scan_ret != EOF)
	{
		scan_ret = fscanf(fd, " %s %s %s %s\n", username, IP_address, UDP_port, status);
		if( (scan_ret != 4) && (scan_ret != EOF) ){
			printf("%d: !!! Error: who() --> fscanf() ret = %d\n", pid, ret);
			fcntl(filedes, F_SETLKW, &client_unlock);
			fclose(fd);
			return 1;	
		}
		memset(buffer_out, 0, sizeof(buffer_out));	
		if(scan_ret == EOF){
			strcpy(buffer_out, "end");
		}else{
			strcat(username, " (");
			strcat(username, status);
			strcat(username, ")\n");
			strcpy(buffer_out, username);
		}
		ret = sendTCP(buffer_out, client_sock);
		if(ret != 0){
			printf("%d: !!! Error: who() --> sendTCP()\n", pid);
			fcntl(filedes, F_SETLKW, &client_unlock);
			fclose(fd);
			return 1;		
		}
	}
	
	fcntl(filedes, F_SETLKW, &client_unlock);
	fclose(fd);
	return 0;	
}
