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
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <vector> 

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 100

#define MAXBUFLEN 100

using namespace std;

char* strcon(string s){
	char *cstr = new char[s.length() + 1];
	strcpy(cstr, s.c_str());
	return cstr;
}

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

int get_socket_fd(int * sockfd, string port){
	struct addrinfo hints, *servinfo, *p;
	int rv, yes=1;
	struct sigaction sa;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, strcon(port), &hints, &servinfo)) != 0) {
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

	//printf("server: waiting for connections...\n");

	return 0;

}

int udp_talk_to(string port, string message){

	struct addrinfo hints, *servinfo, *p;
	int rv, numbytes, sockfd;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo("localhost", strcon(port), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, strcon(message), strlen(strcon(message)), 0,
			p->ai_addr, p->ai_addrlen)) == -1) {
			perror("talker: sendto");
			exit(1);
	}

	//printf("talker: sent %d bytes to 30255\n", numbytes);
	close(sockfd);

	return 0;
}

int udp_listen_on(string port, char (&buf)[MAXBUFLEN]){
	struct addrinfo hints, *servinfo, *p;
	int rv, sockfd, numbytes;
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr;
	socklen_t addr_len;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, strcon(port), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
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

	//cout << "listener: waiting to recvfrom... " <<  port;

	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	buf[numbytes] = '\0';

	/*printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));

	printf("listener: packet is %d bytes long\n", numbytes);*/
	//printf("listener: packet contains \"%s\"\n", buf);
	close(sockfd);
	return 0;
}

void deserialize(string line, map<string, string> &forward, string port){
	istringstream stream(line);
	string s;
	vector<string> line_vec;

	while(stream >> s){
		line_vec.push_back(s);
	}

	for(int i = 0; i < line_vec.size(); i++){
		forward.insert(make_pair(line_vec[i], port));
	}
}

void deserialize_client(string line, string &country, int &uid){

	istringstream stream(line);
	string s;
	vector<string> line_vec;


	while(stream >> s){
		line_vec.push_back(s);
	}

	country = line_vec[0];
	stringstream convert(line_vec[1]);
	convert >> uid;

	//cout<< "Desearlised: " << country << ", " << uid << endl; 
}

void print_forwarding_table(map<string, string> forward){
	vector<string> A, B;
	
	for(map<string, string>::const_iterator it = forward.begin();
	    	it != forward.end(); ++it){
				if(it->second == "30255"){
					A.push_back(it->first);
				}
				else{
					B.push_back(it->first);
				}
	}
	cout << "Server A | Server B" << endl;
	
	vector<string>::iterator it1 = A.begin();
	vector<string>::iterator it2 = B.begin();

	while(it1 != A.end() || it2 != B.end()){
		if(it1 != A.end()){
			cout << *it1;
			it1++;
		}

		cout << " | ";

		if(it2 != B.end()){
			cout << *it2;
			it2++;
		}
		cout << endl;
	}
	cout << endl;
}

int main(void)
{
	// Contact backend servers and get data related to countries
	string port1 = "30255", port2 = "32255", port3 = "31255",  port4 = "33255", signal="RD";
	char buf[MAXDATASIZE], s[INET6_ADDRSTRLEN];
	int sockfd, new_fd, numbytes, error;
	struct sockaddr_storage their_addr; 
	map<string, string> forward;
	socklen_t sin_size;

	cout << "The Main Server is up and running " << endl << endl; 

	// Talker - serverA
	if ((error = udp_talk_to(port1, signal)) != 0) {
		return error;
	}

	// Listener - serverA
	if ((error = udp_listen_on(port2, buf)) != 0) {
		return error;
	}

	cout << "The Main Server has received the country list from server A using UDP over port " << port2 << endl << endl; 

	deserialize(buf, forward, port1);

	// Talker - serverB
	if ((error = udp_talk_to(port3, signal)) != 0) {
		return error;
	}

	// Listener - serverB
	if ((error = udp_listen_on(port2, buf)) != 0) {
		return error;
	}

	cout << "The Main Server has received the country list from server B using UDP over port " << port2 << endl << endl; 

	deserialize(buf, forward, port3);


	print_forwarding_table(forward);


	// Get socket file descriptor for endpoint facing client
	if ((error = get_socket_fd(&sockfd, port4)) != 0) {
		return error;
	}

	while(1) { 
		sin_size = sizeof their_addr;
		
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		/*inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);*/

		if (!fork()) { // this is the child process
			
			close(sockfd); // child doesn't need the listener

			// **** Start conversing with the client ****
			
			
			// Receive from client
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
	    		perror("recv");
	    		exit(1);
			}
			//printf("server: received : %s from client\n \n", buf);

			string country, port, server;
			int uid;

			deserialize_client(buf, country, uid);

			if(forward.find(country) == forward.end()){
				cout << country << "does not show up in server A&B " << endl;
				cout << "The Main Server has sent '" << country << " : Not found' to client # using TCP over port " << port4 << endl << endl;
				const string temp = country + "does not show up in server A&B ";
           	 	strcpy(buf, temp.c_str());
			}

			else{
				port = forward.find(country)->second;

				cout << "The Main Server has received the request on User " << uid << " in " << country << " from client # using TCP over port " << port4 << endl;
				
				if(port == port1){
					server = "A";
				}
				else{
					server = "B";
				}

				cout << country << " shows up in server" << server << endl;
				
				// Talker
				if ((error = udp_talk_to(port, buf)) != 0) {
					return error;
				}

				cout << "The Main Server has sent request from User " << uid << " to server" << server << " using UDP over port " << port << endl;

				// Listener
				if ((error = udp_listen_on(port2, buf)) != 0) {
					return error;
				}
			}

			// ** Check if user not found **

			cout << "The Main Server has received searching results of User " << uid << " from server" << server << endl;

			// Send to client
			if (send(new_fd, buf, sizeof buf , 0) == -1)
				perror("send");

			cout << "The Main Server has sent searching result to client using TCP over port " << port4 << endl;

			// **** End conversing with the client ****

			close(new_fd);
			exit(0);
		}

		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

