#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <cctype>
#include <algorithm>
#include <map>
#include <list>
#include <cassert>
#include <stdbool.h>
#include <sstream>

#include "constants.h"


using namespace std;



	map<string,string> memberInfo;
	map<string,vector<string> > map_Sequencer;

	map<string,bool > memInfo;


	void addMember (string key, string username) {

	//string ipPort = ipaddress + string(":") + to_string(portNo);

	//string uname = username;

		map<string,string>::iterator it;

		it = memberInfo.find(key);
        	if (it == memberInfo.end()){
				memberInfo.insert(it, std::pair<string,string>(key, username));
    		}
	
	}

		
	
	char* getListOfMembers () {
		
		stringstream ss;
		//if (memberInfo.size() > 0) {
		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); it1++) {
			ss << it1->first << "|" << it1->second << "~";
		}
		//}
		//return the list of user
		return strcpy((char*)malloc(ss.str().length()+1), ss.str().c_str());
		
	}

	
	string getUserName(string key){
		map<string,string>::iterator it;

		it = memberInfo.find(key);
		//cout<<getListOfMembers();
    		if (it != memberInfo.end()){
			return it->second;
    		}
	}

	void deleteMember (string entry) {

	//deletion logic goes here
		map<string,string>::iterator it;

		it = memberInfo.find(entry);
    		if (it != memberInfo.end()){
    			memberInfo.erase (it);
    		}
	}

	//Notice change of return type


	void addFromStr(char* input) {
		char* ipPortEntry = strtok (input,"-");
		char* unameEntry = strtok (NULL, "-");

		addMember(string(ipPortEntry), string(unameEntry));
	}

	void getMapFromString (char * input) {
		
		std::vector<char *> myvector;

		char* entry = strtok (input,"~");
  		while (entry != NULL) {
			myvector.push_back(entry);
			entry = strtok (NULL,"~");
  		}

		for (vector<char *>::iterator it = myvector.begin() ; it != myvector.end(); it++)
			addFromStr(*it);

	}

	string* listOfUsers(){
		string* user = new string[memberInfo.size()];
		int i = 0; 
		//if (memberInfo.size() > 0) {
		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); it1++) {
			user[i] = it1->second;
			i++;
		}
		//}
		return user;

	}

	

	