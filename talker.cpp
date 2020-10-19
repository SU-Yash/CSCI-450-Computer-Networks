/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT "30255"	// the port users will be connecting to

int get_socket_fd(int * sockfd, char * port, struct addrinfo **p){

	struct addrinfo hints, *servinfo;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("localhost", port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
		if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype,
				(*p)->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (*p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}


	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes, error;
	struct addrinfo *p;

	if ((error = get_socket_fd(&sockfd, "30255", &p)) != 0) {
		return error;
	}

	if ((numbytes = sendto(sockfd, "Hello", 4, 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		printf("%s", p->ai_addr->sa_data);
		perror("talker: sendto");
		exit(1);
	}

	printf("talker: sent %d bytes to 30255\n", numbytes);
	close(sockfd);

	return 0;
}