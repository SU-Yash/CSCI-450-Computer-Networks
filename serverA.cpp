// This is serverA
/*
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <vector> 

using namespace std;

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* strcon(string s){
	char *cstr = new char[s.length() + 1];
	strcpy(cstr, s.c_str());
	return cstr;
}

int udp_listen_on(string port, char (&buf)[MAXBUFLEN]){
	int sockfd, numbytes, rv;
	struct sockaddr_storage their_addr;
	char s[INET6_ADDRSTRLEN];
	socklen_t addr_len;
	struct addrinfo hints, *servinfo, *p;


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

	//cout << "The server A is up and running using UDP on port " << port << endl;

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	buf[numbytes] = '\0';

	/*

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	printf("listener: packet contains \"%s\"\n", buf);

	*/

	close(sockfd);

	return 0;

}

int udp_talk_on(string port, char* message){

	struct addrinfo hints, *servinfo, *p;
	int sockfd, rv, numbytes;

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

	freeaddrinfo(servinfo);

	if ((numbytes = sendto(sockfd, message, strlen(message), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	//printf("talker: sent %s bytes to \n", message);
	close(sockfd);

	return 0;
}

void check_if_all_friends(map<string, map<int, set<int> > > graph, map<string, set<int> > &all_friends){

	for(map<string, map<int, set<int> > >::const_iterator it = graph.begin();
	    it != graph.end(); ++it){
		
		set<int> all_users;

	    for(map<int, set<int> >::const_iterator it2 = (it->second).begin();
	    it2 != (it->second).end(); ++it2){
			all_users.insert(it2->first);
	    	for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {
        		all_users.insert(*it3);
	    	}
	    }

		set<int> all_connected;

		for(map<int, set<int> >::const_iterator it2 = (it->second).begin();
	    it2 != (it->second).end(); ++it2){

			bool all_co = true;

	    	for (set<int>::iterator it3=(all_users).begin(); it3 != (all_users).end(); ++it3) {
        		if((it2->second).find(*it3) == (it2->second).end() && *it3 != (it2->first)){
					all_co = false;
				}
	    	}

			if(all_co == true){
				all_connected.insert(it2->first);
			}

	    }

		all_friends.insert(make_pair(it->first, all_connected));

	}	

}

void read_file(char * file, map<string, map<int, set<int> > > & graph, 
	map<string, set<int> > & all_connected){
	
	ifstream infile;

	infile.open(file); 

	string country, line;
	map< int, set<int> > ajm;
	

	while(getline(infile, line)){

		istringstream stream(line);

		string s;
		vector<string> line_vec;


		while(stream >> s){
			line_vec.push_back(s);
		}
		
		if(line_vec.size() == 1 && isdigit(line_vec[0][0]) == false){
			if(country != ""){
				graph.insert(make_pair(country, ajm));
			}
			ajm.clear();
			country = line_vec[0];
			//cout<< "Country: " << country << endl ;
		}
		else{
			string r = line_vec[0];
			int pivot, node;
			stringstream convert(r);
			convert >> pivot;

			//cout << "Pivot: " << pivot << endl;
			
			set<int> neighbours;
			for(int i = 1; i < line_vec.size(); i++){
				//cout << "Neighbours: " << line_vec[i] << endl;
				r = line_vec[i];
				stringstream convert(r);
				convert >> node;
				neighbours.insert(node);
			}
			ajm.insert(make_pair(pivot, neighbours));
		}
	
		line_vec.clear();
	
	}

	graph.insert(make_pair(country, ajm));

	check_if_all_friends(graph, all_connected);
}

void deserialize(string line, string &country, int &uid){

	istringstream stream(line);
	string s;
	vector<string> line_vec;


	while(stream >> s){
		line_vec.push_back(s);
	}

	country = line_vec[0];
	stringstream convert(line_vec[1]);
	convert >> uid;

	cout << "The server A has received request for finding possible friends of User " << uid << " in " << country << endl << endl;

}

int execute_query(string c, int uid, map<string, map<int, set<int> > > & graph, 
	map<string, set<int> > & all_connected, int & suggestion){
	
	//cout << "Executing query" << endl;

    map<string, map<int, set<int> > >::const_iterator pos = graph.find(c);
	
	if(pos == graph.end()){
		// Country not found
		cout << "country not found" << endl;
		return -3;
	}
	else{
		// early exit (all-connected?)
		if(all_connected.find(c)->second.find(uid) != all_connected.find(c)->second.end()){
			return -1;
		}


		map<int, set<int> > map_int = pos->second;
		map<int, set<int> >::const_iterator p = map_int.find(uid);

		if(p == map_int.end()){
			// User not found
			return -2;
		}	
		else{
			
			set<int> set_int = p->second;
			int max = 0;
			int max_node = -1;

			for(map<int, set<int> >::const_iterator it2 = map_int.begin();
	    	it2 != map_int.end(); ++it2){
	    		
				if(uid != it2->first && set_int.find(it2->first) == set_int.end()){

		    		int count = 0;

		    		for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {

	        			if(set_int.find(*it3) != set_int.end()){
	        				count++;
	        				if(count > max){
	        					max = count;
	        					max_node = it2->first;
	        				}
	        			}
		    		}
	    		}
        		
			}
			//cout << "Max: " << max << " , Max Node: " << max_node << endl;
			suggestion = max_node;
		}

	}

	return 0;
}

int main(void)
{
	// Read in the file, process and store the data
	map<string, map<int, set<int> > > graph;
	map<string, set<int> > all_connected;
	int error;
	char buf[MAXBUFLEN];
	string port1 = "30255", port2 = "32255";

	read_file(strcon("./data1.txt"), graph, all_connected);

	
	// Wait for mainserver to request the stored information
	string countries;
	
	for(map<string, map<int, set<int> > >::const_iterator it = graph.begin();
	    it != graph.end(); ++it){
		countries = countries + " " + it->first; 
	}

	char *cstr = new char[countries.length() + 1];
	strcpy(cstr, countries.c_str());

	while(1){

		cout << "The server A is up and running using UDP on port " << port1 << endl << endl;

		if ((error = udp_listen_on(port1, buf)) != 0) {
			return error;
		}

		if(strcmp(buf, "RD") == 0){
			//cout << "Received request from mainserver for data" << endl; 

			usleep(3000000);

			if ((error = udp_talk_on(port2, cstr)) != 0) {
				return error;
			}
			cout << "The server A has sent a country list to Main Server" << endl << endl;
			break;
		}
		else{
			cout << "Does not match" << endl;
		}

	}

	
	// Start listening and answering user queries
	while(1){

		//cout << "Serving Queries now: " << endl;

		string country;
		int uid;

		// listen to mainserver
		if ((error = udp_listen_on(port1, buf)) != 0) {
			return error;
		}

		deserialize(buf, country, uid);

		// Resolve query

		int suggestion = -1;
		char * cstr;

		execute_query(country, uid, graph, all_connected, suggestion);
		
		if(suggestion == -1){
            stringstream ss;
            ss << uid;
            string str = ss.str();
            const string temp = " " + str + " is already connected to all other users, no new recommendation";
            cstr = new char[temp.length() + 1];
            strcpy(cstr, temp.c_str());
			cout << uid << " is already connected to all other users, no new recommendation" << endl; 
			cout << "The server A has sent 'User " << uid << " connected to all other users, no new recommendation' to Main Server" << endl << endl;

        }
		else if (suggestion == -2){
            stringstream ss;
            ss << uid;
            string str = ss.str();
            const string temp = "User " + str + " not found";
            cstr = new char[temp.length() + 1];
            strcpy(cstr, temp.c_str());
			cout << "User " << uid << " does not show up in " << country << endl;
			cout << "The server A has sent 'User " << uid << " not found' to Main Server" << endl << endl;
		}
		else{
			ostringstream oss;
			oss << suggestion;
			const string temp = oss.str();
			cstr = new char[temp.length() + 1];
			strcpy(cstr, temp.c_str());
			cout << "The server A is searching possible friends for User " << uid << "..." << endl;
			cout << "Here are the results: " << suggestion << endl;
			cout << "The server A has sent the results to Main Server " << endl << endl; 
		}


		// talk to mainserver
		if ((error = udp_talk_on(port2, cstr)) != 0) {
			return error;
		}

		cout << endl << endl;
	}

	return 0;
}