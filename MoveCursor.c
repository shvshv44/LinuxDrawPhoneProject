#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>
#define PORT 8080
int main(int argc, char const *argv[])
{
	printf("IM ALIVE!!!");
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";
	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
	
	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
					(socklen_t*)&addrlen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}

	while(1) {
		valread = read( new_socket , buffer, 1024);
		printf("%s\n",buffer);
		
		//find values!!!
		int i = 0;
    		char *p = strtok (buffer, " ");
		char *cursurVals[12];
		while (p != NULL)
		{
			cursurVals[i++] = p;
			p = strtok (NULL, " ");
		}
		
		float deltaX = atof(cursurVals[0]);
		float deltaY = atof(cursurVals[1]);
		int clicked = atoi(cursurVals[2]);
		
		if(deltaX != 0 && deltaY != 0) {
			float sensitivity = 4000;
			int moveX = (int) (deltaX*sensitivity);
			int moveY = (int) (deltaY*sensitivity);
			//printf("%d %d %d\n",moveX,moveY,clicked);
			//printf("%s\n","hello");

			char * command = (char *) malloc(1024 * sizeof(char));
			sprintf(command, "xdotool mousemove_relative -- %d %d", moveX, moveY);
			printf("%s\n", command);
			int sys = system(command);
		}
		
		
		memset(buffer, 0, sizeof(buffer));
		//buffer[0] = "\0";
	}

	
	return 0;
}

