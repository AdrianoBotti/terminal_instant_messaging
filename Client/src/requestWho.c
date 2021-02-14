#include "../headers/msg_client.h"

int requestWho(char* command, int TCP_sock)
{
	int ret;
	char buffer_in[BUFFER_SIZE];
	
	//sending command <!who>
	ret = sendTCP(command, TCP_sock);
	if(ret != 0){
		printf("Error requestWho --> sendTCP()\n");
		return 1;		
	}
	
	while(1)
	{
		memset(buffer_in, 0, sizeof(buffer_in));
		ret = receiveTCP(buffer_in, TCP_sock);
		if(ret != 0){
			printf("Error requestWho --> receiveTCP()\n");
			return 1;		
		}	
		if(strcmp(buffer_in, "end") == 0)
			break;
		printf("	%s", buffer_in);
	}
	return 0;
}
