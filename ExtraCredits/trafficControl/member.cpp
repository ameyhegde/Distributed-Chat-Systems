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



	map<string,vector<string> > memberInfo;
	//map<string,vector<string> > map_Sequencer;
	map<string,bool> memInfo;
	map<string,int> messageCount; 


	void addMember (string key, string username, bool lead) {

	//string ipPort = ipaddress + string(":") + to_string(portNo);

	//string uname = username;

		map<string,vector<string> >::iterator it;

		it = memberInfo.find(key);
			if (it == memberInfo.end()){
				memberInfo.insert(it, std::pair<string,vector<string> >(key, vector<string>()));
				memberInfo[key].push_back(username);
				if(lead){
					memberInfo[key].push_back("true");
				}
				else{
					memberInfo[key].push_back("false");
				}
			
			}
	
	}

		char* getListOfMembers () {
		
		stringstream ss;
		//if (memberInfo.size() > 0) {
		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
			ss << it1->first << "|" << it1->second.at(0) << "|" << it1->second.at(1) << "~";
		}
		//}
		//return the list of user
		return strcpy((char*)malloc(ss.str().length()+1), ss.str().c_str());
		
	}

	// std::vector<string> getListOfkeys() {
		
	//  stringstream ss;
	//  vector<string> v;
	//  //if (memberInfo.size() > 0) {
	//  for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
	//      v.push_back(it->first);
	//  }
	//  //return the list of user
	//  return v;
		
	// }

		void deleteMember (string entry) {

	//deletion logic goes here
		map<string,vector<string> >::iterator it;

		it = memberInfo.find(entry);
			if (it != memberInfo.end()){
				memberInfo.erase (it);
			}
	}

	
	string getUserName(string key){
		map<string,vector<string> >::iterator it;

		it = memberInfo.find(key);
		//cout<<getListOfMembers();
			if (it != memberInfo.end()){
			return it->second.at(0);    
			}
			else {
			return "UDE";
		}
	}


	//Notice change of return type


	void addFromStr(char* input) {
		char* ipPortEntry = strtok (input,"-");
		char* unameEntry = strtok (NULL, "-");
		char* flagEntry = strtok (NULL, "-");

		if (strcmp(flagEntry,"true") == 0)
			addMember(string(ipPortEntry), string(unameEntry), true);
		else
			addMember(string(ipPortEntry), string(unameEntry), false);
	}

	void getMapFromString (char * input) {
		
		std::vector<char *> myvector;

		char* entry = strtok (input,"~");
		while (entry != NULL) {
			myvector.push_back(entry);
			entry = strtok (NULL,"~");
		}

		for (vector<char *>::iterator it = myvector.begin() ; it != myvector.end(); ++it)
			addFromStr(*it);

	}

	string* listOfUsers(){
		string* user = new string[memberInfo.size()];
		int i = 0; 
		//if (memberInfo.size() > 0) {
		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
			user[i] = it1->second.at(0);
			i++;
		}
		//}
		return user;

	}

	string Leader(){
		string tmp="";
		//map<string,vector<string> >::iterator it;

		if (memberInfo.size() > 0) {

			

			for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
				//cout<<it1->first<<endl;
				if (it1->second.size() == 2) {
						char* str = strcpy((char*)malloc(it1->second.at(1).length()+1), it1->second.at(1).c_str());
						if(strcmp(str,"true") == 0){
							tmp = it1->first;
							//cout << "inside if: " << tmp << endl;
						}
				}
			}
		}
		return tmp;
	}

	 void newLeader (string entry){
						map<string,vector<string> >::iterator it;
						string user;
						it = memberInfo.find(entry);
						if (it != memberInfo.end()){
								user = it->second.at(0);
								it->second.pop_back();
		       					it->second.pop_back();
								it->second.push_back(user);
								it->second.push_back("true");
						}							  
		}

		bool isLeader(){
						map<string,vector<string> >::iterator it;
		if (memberInfo.size() > 0) {
				for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
						if(it1->second.at(1).compare("true") == 0){
								return true;
						}
				}
		}
				return false;
		}

		
