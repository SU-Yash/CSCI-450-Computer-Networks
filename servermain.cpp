// This is servermain.cpp

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

#define PORTC "33255"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

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

int main(void)
{
	/*
	Structure to contain information about address of a service provider. 
	struct addrinfo
	{
  		int ai_flags;			// Input flags. 
  		int ai_family;		// Protocol family for socket.  
 	 	int ai_socktype;		// Socket type.  
  		int ai_protocol;		// Protocol for socket.  
  		socklen_t ai_addrlen;		// Length of socket address.  
  		struct sockaddr *ai_addr;	// Socket address for socket.  
  		char *ai_canonname;		// Canonical name for service location.  
  		struct addrinfo *ai_next;	// Pointer to next in list.  
	};
	*/

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORTC, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, // returns a socker descriptor 
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {  // associate socket with a port 
			close(sockfd);
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

	if (listen(sockfd, BACKLOG) == -1) {  // server is listening on the port that was bound to this socket descriptor
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

	printf("server: waiting for connections...: %s\n", PORTC);

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

            //Contact backend servers



            int sockfd2;
	        struct addrinfo hints2, *servinfo2, *p2;
	        int rv2;
	        int numbytes2;


	        memset(&hints2, 0, sizeof hints2);
	        hints2.ai_family = AF_UNSPEC;
	        hints2.ai_socktype = SOCK_DGRAM;

            if ((rv2 = getaddrinfo("localhost", "30255", &hints2, &servinfo2)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
                return 1;
            }

            // loop through all the results and make a socket
            for(p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
                if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype,
                        p2->ai_protocol)) == -1) {
                    perror("talker: socket");
                    continue;
                }

                break;
            }

            if (p2 == NULL) {
                fprintf(stderr, "talker: failed to create socket\n");
                return 2;
            }

            char message[] = "Lame";

            printf("Sending message to: 30255");

            if ((numbytes2 = sendto(sockfd2, message, strlen(message), 0,
                    p2->ai_addr, p2->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }

            freeaddrinfo(servinfo2);

            printf("talker: sent %d bytes to\n", numbytes2);
           

            //


            // Listen 

            int sockfd3;
			struct addrinfo hints3, *servinfo3, *p3;
			int rv3;
			int numbytes3;
			struct sockaddr_storage their_addr3;
			char buf3[MAXBUFLEN];
			socklen_t addr_len3;
			char s3[INET6_ADDRSTRLEN];

			memset(&hints3, 0, sizeof hints3);
			hints3.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
			hints3.ai_socktype = SOCK_DGRAM;
			hints3.ai_flags = AI_PASSIVE; // use my IP

			if ((rv3 = getaddrinfo(NULL, "32255", &hints3, &servinfo3)) != 0) {
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv3));
				return 1;
			}

			// loop through all the results and bind to the first we can
			for(p3 = servinfo3; p3 != NULL; p3 = p3->ai_next) {
				if ((sockfd3 = socket(p3->ai_family, p3->ai_socktype,
						p3->ai_protocol)) == -1) {
					perror("listener: socket");
					continue;
				}

				if (bind(sockfd3, p3->ai_addr, p3->ai_addrlen) == -1) {
					close(sockfd3);
					perror("listener: bind");
					continue;
				}

				break;
			}

			if (p3 == NULL) {
				fprintf(stderr, "listener: failed to bind socket\n");
				return 2;
			}

			freeaddrinfo(servinfo3);

			printf("listener: waiting to recvfrom...: 32255 \n" );

			addr_len3 = sizeof their_addr3;
			if ((numbytes3 = recvfrom(sockfd3, buf3, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr3, &addr_len3)) == -1) {
				perror("recvfrom");
				exit(1);
			}

			printf("listener: got packet from %s\n",
				inet_ntop(their_addr3.ss_family,
					get_in_addr((struct sockaddr *)&their_addr3),
					s3, sizeof s3));
			printf("listener: packet is %d bytes long\n", numbytes3);
			buf3[numbytes3] = '\0';
			printf("listener: packet contains \"%s\"\n", buf3);


			close(sockfd2);
			close(sockfd3);


            //





			if (send(new_fd, buf3, strlen(buf3), 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

