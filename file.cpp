#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <vector> 

using namespace std;

void print_map(map<string, map<int, set<int> > > graph){
	for(map<string, map<int, set<int> > >::const_iterator it = graph.begin();
	    it != graph.end(); ++it)
	{
	    cout << "Key: " << it->first << " " << endl;

	    for(map<int, set<int> >::const_iterator it2 = (it->second).begin();
	    it2 != (it->second).end(); ++it2){
	    	cout << it2->first << ": ";

	    	for (set<int>::iterator it3=(it2->second).begin(); it3 != (it2->second).end(); ++it3) {
        		cout << ' ' << *it3;
	    	}
        	cout << endl;
	    }
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

int main(void){

	ifstream infile;

	infile.open("./data1.txt"); 

	string country, line;
	map<string, map<int, set<int> > > graph;
	map<string, set<int> > all_connected;
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
			cout<< "Country: " << country << endl ;
		}
		else{
			string r = line_vec[0];
			int pivot, node;
			stringstream convert(r);
			convert >> pivot;

			cout << "Pivot: " << pivot << endl;
			
			set<int> neighbours;
			for(int i = 1; i < line_vec.size(); i++){
				cout << "Neighbours: " << line_vec[i] << endl;
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

	print_map(graph);

	check_if_all_friends(graph, all_connected);


	// find for a given node

	cout << "Executing query" << endl;

	string c = "SIs";
	int uid = 7;

    map<string, map<int, set<int> > >::const_iterator pos = graph.find(c);
	
	if(pos == graph.end()){
		// Country not found
		cout << "country not found" << endl;
		return 0;
	}
	else{
		// early exit (all-connected?)
		if(all_connected.find(c)->second.find(uid) != all_connected.find(c)->second.end()){
			cout << uid << " is already connected to all other users, no new recommendation" << endl; 
			return 0;
		}


		map<int, set<int> > map_int = pos->second;
		map<int, set<int> >::const_iterator p = map_int.find(uid);

		if(p == map_int.end()){
			// User not found
			cout << "user not found";
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
			cout << "Max: " << max << " , Max Node: " << max_node << endl;
		}

	}
	
	return 0;
}
