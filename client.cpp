/*
** client 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

using namespace std;

#define MAXDATASIZE 1000 // max number of bytes we can get at once 

#define MAIN_SERVER_PORT "33255"

int get_socket_fd(int *sockfd, char *port) // Reused code from Beej’s Tutorial
{
	int rv;
	char s[INET6_ADDRSTRLEN];
	struct addrinfo hints, *p, *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("localhost", port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(*sockfd);
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	return 0;
}

char* strcon(string s){
	char *cstr = new char[s.length() + 1];
	strcpy(cstr, s.c_str());
	return cstr;
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes, error;  
	char buf[MAXDATASIZE];

	string country;
	string userId;

	cout << "Client is up and running" << endl << endl;

	while(1) {

		cout << "Please enter the User ID: ";
		cin >> userId;
		cout << "Please enter the country name: ";
		cin >> country;
        cout << endl;

		string message = country + " " + userId;

		// Get socket file descriptor for main-server
		if ((error = get_socket_fd(&sockfd, strcon(MAIN_SERVER_PORT))) != 0) {
			return error;
		}

		// Send data
		if (send(sockfd, strcon(message), strlen(strcon(message)), 0) == -1)
			perror("send");

		cout << "Client has sent User " << userId << " and Country " << country << " to Main Server using TCP" << endl << endl;

		// Receive data 
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buf[numbytes] = '\0';

		// Print results
		if (strcmp(buf, "-1") == 0){
			cout << "User " << userId << " is already connected to all users in " << country << endl;
		} 
		else if (strcmp(buf, "-2") == 0){
			cout << "User " << userId << " not found in " << country << endl;
		}
		else if (strcmp(buf, "-3") == 0){
			cout << "Country " << country << " not found" << endl;
		}
		else{
			cout << "Client has received results from Main Server: " << buf << " is possible friend of " << userId << " in " << country << endl << endl;
		}

		close(sockfd);

		cout << endl;

	}

	return 0;
}
