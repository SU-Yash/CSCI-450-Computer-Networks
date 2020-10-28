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

char* strcon(string s){
	char *cstr = new char[s.length() + 1];
	strcpy(cstr, s.c_str());
	return cstr;
}

void print_file(map<string, map<int, set<int > > > graph){
    for(map<string, map<int, set<int> > >::const_iterator it = graph.begin();
	it != graph.end(); ++it){
        cout << "Country: " << it->first << endl;
        for(map<int, set<int> >::const_iterator it2 = (it->second).begin();
        it2 != (it->second).end(); ++it2){
            cout << it2->first;
            for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {
                cout << " " << *it3;
            }
            cout << endl;
        }
        cout << endl;
    }
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

int execute_query(string c, int uid, map<string, map<int, set<int> > > & graph, 
	map<string, set<int> > & all_connected, int & suggestion){
	
	if(all_connected.find(c)->second.find(uid) != all_connected.find(c)->second.end()){
        // early exit: uid is already connected to all other users in the country 
        cout << "early exit: uid is already connected to all other users in the country " << endl;
		return -1;
	}

    map<string, map<int, set<int> > >::const_iterator pos = graph.find(c);
	map<int, set<int> > map_int = pos->second;
	map<int, set<int> >::const_iterator p = map_int.find(uid);

	if(p == map_int.end()){
		// early exit: uid not found
        cout << "early exit: uid not found" << endl;
		return -2;
	}	
			
	set<int> set_int = p->second;
	int max = 0, max_node = -1;

	if(set_int.size() == 0) { 
        // early exit: uid not connected to any other user in the country, so return node with highest degree
	    for(map<int, set<int> >::const_iterator it2 = map_int.begin();
	    	it2 != map_int.end(); ++it2){
			if(uid != it2->first){
				if(it2->second.size() > max){
					max = it2->second.size();
					max_node = it2->first;
				}
				else if(it2->second.size() == max){
                    if(max_node == -1){
                        max =  it2->second.size();
						max_node = it2->first;
                    }

					if(it2->first < map_int.find(max_node)->first){
						max =  it2->second.size();
						max_node = it2->first;
                        cout << "Reached here!!";
					}
				}
			}
		}
        cout << "early exit: uid not connected to any other user in the country, so return node with highest degree" << endl;
        suggestion = max_node;
	    return 0;
	}

	for(map<int, set<int> >::const_iterator it2 = map_int.begin();
	    it2 != map_int.end(); ++it2){
	    // finding user with highest common neighbours for uid	
		
        if(uid != it2->first && set_int.find(it2->first) == set_int.end()){ 
            // skip sets for uid and all current friends of uid
		    int count = 0;

		    for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {

	        	if(set_int.find(*it3) != set_int.end()){
	        		count++;
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
	    } 		
	}
	cout << "Max: " << max << " , Max Node: " << max_node << endl;
	suggestion = max_node;
    cout << "" << endl;
	return 0;
}

int main(void){
    
    map<string, map<int, set<int> > > graph;
	map<string, set<int> > all_connected;
    
    read_file(strcon("./data1.txt"), graph, all_connected);
    
    int userId, suggestion = -1, error;
    string country;

    print_file(graph);

    

    // User already connected to all other
    error = execute_query(strcon("SI"), 7, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value: " << error << endl << "Expected: -1" << endl;
    cout << "Suggestion: " << suggestion << endl << "Expected: None " << endl;

    cout << "*****" << endl;

    // User not present 
    error = execute_query(strcon("SI"), 999, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value : " << error << endl << "Expected: -2" << endl;
    cout << "Suggestion : " << suggestion << endl << "None" << endl;

    cout << "*****" << endl;

    

    // highest degree - user not connected to any other user, return recommendation with highest degree
    error = execute_query(strcon("abc"), 29, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value : " << error << endl << "Expected: 0" << endl;
    cout << "Suggestion : " << suggestion << endl << " Expected: 31" << endl;

    cout << "*****" << endl;

    
    // highest degree - user not connected to any other, multiple users have same highest degree, return one with smaller userID
    error = execute_query(strcon("bcd"), 30, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value : " << error << endl << "Expected: 0" << endl;
    cout << "Suggestion : " << suggestion << endl << "Expected: 1" << endl;

    cout << "*****" << endl;

    // most common neighbours 
    error = execute_query(strcon("xyz"), 31, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value : " << error << endl << "Expected: 0" << endl;
    cout << "Suggestion : " << suggestion << endl << "Expected: 67" << endl;

    cout << "*****" << endl;

    // most common neighbours - Tie Breaker, should return user with smaller ID
    error = execute_query(strcon("xyz"), 15, graph, all_connected, suggestion);
    cout << endl;
    cout << "Return value : " << error << endl << "Expected: 0" << endl;
    cout << "Suggestion : " << suggestion << endl << "Expected: 11" << endl;

    cout << "*****" << endl;

}