// This is serverA

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

#define SERVER_A_PORT "30255"

#define MAIN_SERVER_PORT "32255"

char* strcon(string s){
	char *cstr = new char[s.length() + 1];
	strcpy(cstr, s.c_str());
	return cstr;
}

int udp_listen_on(string port, char (&buf)[MAXBUFLEN]){  // Reused code from Beej’s Tutorial
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

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	buf[numbytes] = '\0';

	close(sockfd);

	return 0;

}

int udp_talk_on(string port, char* message){  // Reused code from Beej’s Tutorial

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
		}
		else{
			string r = line_vec[0];
			int pivot, node;
			stringstream convert(r);
			convert >> pivot;
			
			set<int> neighbours;
			for(int i = 1; i < line_vec.size(); i++){
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
	
	if(all_connected.find(c)->second.find(uid) != all_connected.find(c)->second.end()){
        // early exit: uid is already connected to all other users in the country 
		return -1;
	}

    map<string, map<int, set<int> > >::const_iterator pos = graph.find(c);
	map<int, set<int> > map_int = pos->second;
	map<int, set<int> >::const_iterator p = map_int.find(uid);

	if(p == map_int.end()){
		// early exit: uid not found
		return -2;
	}	
			
	set<int> set_int = p->second;
	int max = 0, max_node = 2147483647;

	for(map<int, set<int> >::const_iterator it2 = map_int.begin();
	    it2 != map_int.end(); ++it2){
	    // finding user with highest common neighbours for uid	
		
        if(uid != it2->first && set_int.find(it2->first) == set_int.end()){ 
            // skip sets for uid and all current friends of uid
		    int count = 0;

		    for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {
	        	if(set_int.find(*it3) != set_int.end()){
	        		count++;
	        	}
		    }

			if(count == max){ 
            	// Tie Breaker for n common neighbours / smaller ID wins
				if(it2->first < max_node){ 
					max = count;
					max_node = it2->first;
				}
			}

	        else if(count > max){
	        	max = count;
	        	max_node = it2->first;
	        }
	    } 		
	}

	if(set_int.size() == 0 || max == 0) { 
        // uid not connected to any other user in the country, so return node with highest degree
	    for(map<int, set<int> >::const_iterator it2 = map_int.begin();
	    	it2 != map_int.end(); ++it2){
			if(uid != it2->first && set_int.find(it2->first) == set_int.end()){
				// only use nodes which are not connected to uid and skip uid itself
				if(it2->second.size() > max){
					max = it2->second.size();
					max_node = it2->first;
				}
				else if(it2->second.size() == max){
					if(it2->first < max_node){
						max =  it2->second.size();
						max_node = it2->first; 
					}
				}
			}
		}
	}
	suggestion = max_node;
    cout << endl;
	return 0;
}


int main(void)
{
	// Read in the file, process and store the data
	map<string, map<int, set<int> > > graph;
	map<string, set<int> > all_connected;
	int error;
	char buf[MAXBUFLEN];

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

		cout << "The server A is up and running using UDP on port " << SERVER_A_PORT << endl << endl;

		if ((error = udp_listen_on(SERVER_A_PORT, buf)) != 0) {
			return error;
		}

		if(strcmp(buf, "RD") == 0){

			usleep(1000000);

			if ((error = udp_talk_on(MAIN_SERVER_PORT, cstr)) != 0) {
				return error;
			}
			cout << "The server A has sent a country list to Main Server" << endl << endl;
			break;
		}
	}

	
	// Start listening and answering user queries
	while(1){

		string country;
		int uid;

		// listen to mainserver
		if ((error = udp_listen_on(SERVER_A_PORT, buf)) != 0) {
			return error;
		}

		deserialize(buf, country, uid);

		// Resolve query

		int suggestion = -1;
		char * cstr;

		error = execute_query(country, uid, graph, all_connected, suggestion);
		
		if(error == -1){
            cstr = strcon("-1");
			cout << "User " << uid << " is already connected to all other users, no new recommendation" << endl; 
			cout << "The server A has sent 'User " << uid << " is already connected to all users in " << country << "'" << " to Main Server" << endl;

        }
		else if (error == -2){
            cstr = strcon("-2");
			cout << "User " << uid << " does not show up in " << country << endl;
			cout << "The server A has sent 'User " << uid << " not found' to Main Server" << endl;
		}
		else{
			ostringstream oss;
			oss << suggestion;
			const string temp = oss.str();
			cstr = strcon(temp);
			cout << "The server A is searching possible friends for User " << uid << "..." << endl;
			cout << "Here are the results: " << suggestion << endl;
			cout << "The server A has sent the results to Main Server " << endl; 
		}


		// talk to mainserver
		if ((error = udp_talk_on(MAIN_SERVER_PORT, cstr)) != 0) {
			return error;
		}

		cout << endl;
	}

	return 0;
}