/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100

#define MAXBUFLEN 100

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_socket_fd(int * sockfd, char * port){
	struct addrinfo hints, *servinfo, *p;
	int rv, yes=1;
	struct sigaction sa;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype, // returns a socker descriptor 
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {  // associate socket with a port 
			close(*sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(*sockfd, BACKLOG) == -1) {  // server is listening on the port that was bound to this socket descriptor
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	return 0;

}

int get_socket_fd_talker(int * sockfd, char * port, struct addrinfo **p){

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

int get_socket_fd_listener(int * sockfd, char * port){
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(*sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...: %s\n", port);

	return 0;
}


char* receiveFrom(int sockfd, sockaddr_storage * their_addr, socklen_t addr_len){
	int numbytes;
	char s[INET6_ADDRSTRLEN], buf[MAXBUFLEN];

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr->ss_family,
			get_in_addr((struct sockaddr *)their_addr),
			s, sizeof s));

	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);	

	return buf;
}

int main(void)
{
	int sockfd, new_fd, numbytes, error;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char buf[MAXDATASIZE], s[INET6_ADDRSTRLEN];

	// Get socket file descriptor for endpoint facing client
	if ((error = get_socket_fd(&sockfd, "33255")) != 0) {
		return error;
	}

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			
			close(sockfd); // child doesn't need the listener

			// **** Start conversing with the client ****
			
			
			// Receive data from client
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
	    		perror("recv");
	    		exit(1);
			}


			printf("server: received : %s from client\n \n", buf);


			// Talker

			int sockfd_t;
			struct addrinfo * p_t;

			if ((error = get_socket_fd_talker(&sockfd_t, "30255", &p_t)) != 0) {
				return error;
			}

			if ((numbytes = sendto(sockfd_t, buf, strlen(buf), 0,
					 p_t->ai_addr, p_t->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			printf("talker: sent %d bytes to 30255\n", numbytes);
			close(sockfd_t);



			// Listener

			char buf_l[MAXBUFLEN];

			int sockfd_l, error;
			struct sockaddr_storage their_addr_l;
			socklen_t addr_len;

			// Get socket file descriptor for port we are listening on
			if ((error = get_socket_fd_listener(&sockfd_l, "32255")) != 0) {
				return error;
			}

			//receiveFrom(sockfd_l, &their_addr_l, sizeof their_addr_l);

			char s[INET6_ADDRSTRLEN];

			addr_len = sizeof their_addr_l;

			if ((numbytes = recvfrom(sockfd_l, buf_l, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr_l, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}

			printf("listener: got packet from %s\n",
				inet_ntop(their_addr_l.ss_family,
					get_in_addr((struct sockaddr *)&their_addr_l),
					s, sizeof s));

			printf("listener: packet is %d bytes long\n", numbytes);
			buf_l[numbytes] = '\0';
			printf("listener: packet contains \"%s\"\n", buf_l);	

			close(sockfd_l);

			// Send data to client
			if (send(new_fd, buf_l, sizeof buf_l , 0) == -1)
				perror("send");


			
			// **** End conversing with the client ****

			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

