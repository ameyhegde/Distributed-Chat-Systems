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
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/time.h>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#include "constants.h"
#include "ipPort.cpp"
#include "udp-server.cpp"
#include "member.cpp"
#include "blockingQueue.h"

using namespace std;

map<string,vector<int> > portInfo;
// Local Vector
BlockingQueue<string> holdbackMessage;
std::mutex mutex_;
std::condition_variable lock_;
std::mutex mtx;
std::condition_variable cv;
std::mutex mtx1;
std::condition_variable cv1;
bool ready = false;
bool ready1 = true;

// Blocking Queue
BlockingQueue<string> sequencerQueue;
BlockingQueue<string> holdbackQueue;
BlockingQueue<string> printQueue;

// Different Servers
server_Udp *server;
server_Udp *chat_server;
server_Udp *join_server;
server_Udp *join1_server;
server_Udp *broadcast_server;
server_Udp *printAck_server;
server_Udp *Leader_server;
server_Udp *checklive_server;
server_Udp *pingAck_server;
server_Udp *election_server;
server_Udp *broadcastAck_server;

// current client info
char ipaddress[IP_BUFSIZE];
int portNo;
char username[UNAME_BUFSIZE];


// leader info
char ip_leader[IP_BUFSIZE];
int port_leader;
bool isleader;

int seq_num = 1;
int prev_seq = 0;

int stop = 0;

void join_handler(char *ipaddr_new, char *port, char *uname) {
	//cout<<"Inside Join handler"<<endl;
	
	struct sockaddr_in sender;
	struct sockaddr_in everyone;

	char joinMsg[MESSAGE_SIZE];
	char tempPort[MESSAGE_SIZE];
	char tempMsg[MESSAGE_SIZE];
	char joinMsg1[MESSAGE_SIZE];
		
		string ipPort = ipaddr_new + string(":") + string(port);

		sender.sin_addr.s_addr = inet_addr(ipaddr_new);
		sender.sin_family = AF_INET;
		sender.sin_port = htons(atoi(port)+9);

		strcpy(joinMsg,"OK-");

	
		string l = Leader();
		while (l.compare("") == 0){
			//cout << "leader waiting..." << endl;
			l = Leader();
		}
		//cout << "leader here " << l << endl;
		char* l1 = strcpy((char*)malloc(l.length()+1), l.c_str());
		strcpy(ip_leader, strtok(l1, ":") );
		std::size_t pos = l.find(":");      // position of "live" in str

		std::string str3 = l.substr (pos+1);
  		char *port_leader1 =  strcpy((char*)malloc(str3.length()+1), str3.c_str());	



		strcat(joinMsg,ip_leader);
		strcat(joinMsg,"-");
		//sprintf(tempPort,"%d",port_leader);
		strcat(joinMsg,port_leader1);
		strcat(joinMsg,"-");
	
		addMember(ipPort,string(uname),false);
		//cout<<"Member added"<<joinMsg<<endl;
		int retval = join_server->send_String(sender,joinMsg,sizeof(joinMsg));

		strcpy(joinMsg1,"NEWEST_CLIENT");
		strcat(joinMsg1,"-");
		strcat(joinMsg1,ipaddr_new);
  		strcat(joinMsg1,":");
  		sprintf(tempMsg,"%d",atoi(port));
  		strcat(joinMsg1,tempMsg);
  		strcat(joinMsg1,"-");
  		strcat(joinMsg1,uname);
  		strcat(joinMsg1,"-");
	
  		sender.sin_port = htons(atoi(port)+2);

		char map_details[MESSAGE_SIZE];

		strcpy(map_details,"");
		for (map<string,vector<string> >::iterator it2=memberInfo.begin(); it2!=memberInfo.end(); ++it2) {

			char* md = strcpy((char*)malloc( (it2->first).length()+1), (it2->first).c_str());
			strcat(map_details,md);
			strcat(map_details,"-");
			char* md1 = strcpy((char*)malloc((it2->second).at(0).length()+1), (it2->second).at(0).c_str());
			strcat(map_details,md1);
			strcat(map_details,"-");
			char *md2 = strcpy((char*)malloc((it2->second).at(1).length()+1), (it2->second).at(1).c_str());
			strcat(map_details,md2);
			strcat(map_details,"-");
			strcat(map_details,"~");

		}
		//cout<<"The map details"<<map_details<<endl;
		int rel2 = broadcast_server->send_String(sender,map_details,sizeof(map_details));

		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
			char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
			std::size_t pos = (it1->first).find(":");      // position of "live" in str

  			std::string str3 = (it1->first).substr (pos+1);				
			char *ipVal = strtok (ipPort_all,":");

			char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
			everyone.sin_addr.s_addr = inet_addr(ipVal);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(atoi(portVal)+2);
			//cout<<"Notice message"<<joinMsg1<<endl;
			int rel1 = broadcast_server->send_String(everyone,joinMsg1,sizeof(joinMsg1));

		}
}

void chat_handler (char *ip_addr,char *port_num,char *chatMsg) {
	
	map<string,vector<string>>::iterator it;
	map<string,vector<string>>::iterator it1;

	char sendMsg[MESSAGE_SIZE];
	struct sockaddr_in sender;
	struct sockaddr_in everyone;
	char del_mes[MESSAGE_SIZE];
	char tempMsg2[MESSAGE_SIZE];


	string ipPort = string(ip_addr) + string(":") + string(port_num);

	sender.sin_addr.s_addr = inet_addr(ip_addr);
	sender.sin_family = AF_INET;

	sender.sin_port = htons(atoi(port_num)+1);
	
	char IP_Char[MESSAGE_SIZE];
	it = memberInfo.find(string(ipPort));
	if(strcmp(chatMsg,"DELETE_MEMBER") == 0) {
			
			strcpy(del_mes,"4");
			strcat(del_mes,"-");
			strcpy(IP_Char, ipPort.c_str());
			strcat(del_mes,IP_Char);
			strcat(del_mes,"-");
		
		
			for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
				char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
				std::size_t pos = (it1->first).find(":");      // position of "live" in str

  				std::string str3 = (it1->first).substr (pos+1);				
				char *ipVal = strtok (ipPort_all,":");

				char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
			
				everyone.sin_addr.s_addr = inet_addr(ipVal);
				everyone.sin_family = AF_INET;
				everyone.sin_port = htons(atoi(portVal));

				int rel1 = server->send_String(everyone,del_mes,sizeof(del_mes));
			}
		} else {
			string msg = ip_addr + string(":") + string (port_num) + string ("-") + string (chatMsg);
			//cout<<"4. Message before push into sequencerQueue: "<<msg<<endl;
			sequencerQueue.push(msg);
			
			strcpy(sendMsg,"GOT_THE_MESSAGE-");
					int retval = chat_server->send_String(sender,sendMsg,sizeof(sendMsg));
			}
					//cout<<"Sent:GOT_THE_MESSAGE"<<endl;
					

}

void *latest_join(void *myvar){
	char msg[MESSAGE_SIZE];
	struct sockaddr_in client;
	struct sockaddr_in client2;
	char tempMsg[MESSAGE_SIZE];
	char * ip = (char *) myvar;
	client.sin_addr.s_addr = inet_addr(ipaddress);
	client.sin_family = AF_INET;
	//char *tokens2 = strtok(NULL,":");
	client.sin_port = htons(portNo+2);
	string ipPort = ipaddress + string(":") + to_string(portNo);
	char * client1 = strcpy((char*)malloc(ipPort.length()+1), ipPort.c_str());
	//std::unique_lock<std::mutex> lck(mtx);

	while(true){
		int result = broadcast_server->receive_String(client,msg,sizeof(msg));
			//cout<<"The message received in latest_join is:"<<msg<<endl;
				if (result < 0){
					perror("error while latest join \n");
					//continue;
				}
				else{
					char* token_gotmsg;
					char* token_gotmsg1 = strtok(msg,"-");
					if(strcmp(token_gotmsg1,"NEWEST_CLIENT") == 0){
					char *token_gotmsg2 = strtok(NULL,"-");
					token_gotmsg = strtok(NULL,"-");
					//strcpy(token_gotmsg,"false");
					addMember(token_gotmsg2,token_gotmsg,false);
					if (strcmp(token_gotmsg2,client1) != 0){
					cout<<"NOTICE " <<token_gotmsg <<" joined the chat "<<token_gotmsg2<<endl;
					}
					listOfUsers();
					}
					else if (strcmp(token_gotmsg1,"NEW_LEADER") == 0){
						std::unique_lock<std::mutex> lck(mtx);
						char msg_send[MESSAGE_SIZE];
						char *newleader_ipmsg = strtok(NULL,"-");
						char *newleader_portmsg = strtok(NULL,"-");

						if (newleader_ipmsg != NULL && newleader_ipmsg[0] != '\0' && newleader_portmsg != NULL && newleader_portmsg[0] != '\0') {

							client2.sin_addr.s_addr = inet_addr(newleader_ipmsg);
							client2.sin_family = AF_INET;
							client2.sin_port = htons(atoi(newleader_portmsg)+8);
							strcpy(msg_send,"ACK_OK");
							int result = broadcastAck_server->send_String(client2,msg_send,sizeof(msg_send));

							string newleader_msg = string (newleader_ipmsg)+string(":")+string(newleader_portmsg);
							newLeader(string(newleader_msg));
							strcpy(ip_leader, newleader_ipmsg);
							port_leader = atoi(newleader_portmsg);
							//cout << "before notice: " << ip_leader << port_leader << endl;
							cout<<"NOTICE "<< getUserName(string(newleader_msg))<< " is the new leader"<<endl;
							seq_num = 1;
							prev_seq = 0;
							//cout << "after notice" << Leader() << endl;
							ready = true;
							cv.notify_one();
						}
						//std::unique_lock<std::mutex> lck1(mtx1);
  						//ready1 = true;
						//cv1.notify_one();
					}
				}

	}

}

void *print_handler(void *myvar){

 	char * ip = (char *) myvar;

	struct sockaddr_in everyone;
	struct sockaddr_in leader;
	

	while(true){
		char tempMsg[MESSAGE_SIZE];
		char temp_seq_num[MESSAGE_SIZE];
 		char str[MESSAGE_SIZE];
 		char msg[MESSAGE_SIZE];
		
			if (seq_num < 32000){
			strcpy(str,"3-");
			sprintf(temp_seq_num,"%d",seq_num);
  			strcat(str,temp_seq_num);
			strcat(str,"-");
			string msgToSend = sequencerQueue.front(); 
			//cout << "6. sequencerQueue pop: " << msgToSend << endl;
			char *Msg2 = strcpy((char*)malloc((msgToSend).length()+1), (msgToSend).c_str());
			strcat(str,Msg2);
			strcat(str,"-");
			seq_num++;
			//cout<<"Sequence_Number incremented: "<<seq_num<<endl;	
			} else{
				seq_num = 1;
				continue;
			}

			//holdbackQueue.push(str);


		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
			char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
			char *ipVal = strtok (ipPort_all,":");
			//char *portVal = strtok (NULL,":");

			std::size_t pos = (it1->first).find(":");      // position of "live" in str

  				std::string str3 = (it1->first).substr (pos+1);				
				//char *ipVal = strtok (ipPort_all,":");

				char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
						
			
			everyone.sin_addr.s_addr = inet_addr(ipVal);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(atoi(portVal));
			//cout<<"7. The message ready to print: "<<str<<endl;
			int rel1 = server->send_String(everyone,str,sizeof(str));

			leader.sin_addr.s_addr = inet_addr(ip_leader);
			leader.sin_family = AF_INET;
			leader.sin_port = htons(port_leader+3);
			struct timeval timeout3={2,0};
			setsockopt(printAck_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout3,sizeof(struct timeval));
			if(printAck_server->receive_String(leader,msg,sizeof(msg))<0){
				int rel2 = server->send_String(everyone,str,sizeof(str));
				struct timeval timeout7={2,0};
				setsockopt(printAck_server->sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout7,sizeof(timeval));
				if ( printAck_server->receive_String(leader,msg,sizeof(msg)) < 0)
					continue;
			} 
		}
		sequencerQueue.pop();
	}
}


	void Member_Alive (string key, bool alive) {

		char del_mes[MESSAGE_SIZE];
		map<string,bool>::iterator it;
		struct sockaddr_in leaderAddr;

		it = memInfo.find(key);
			if (it == memInfo.end()){
				//cout << "about to insert: " << key << endl;
			memInfo.insert(it, std::pair<string,bool>(key,alive));
			} else {
			if (!memInfo[key] ) {
				//cout << "before alive" << endl;
				if (!alive) {
				//time for deletion
			char *token =  strcpy((char*)malloc(key.length()+1), key.c_str());		
			char *ipaddress_del = strtok(token,":");
//			char *portNo_del = strtok(NULL,":");
			std::size_t pos = key.find(":");      // position of "live" in str

  			std::string str3 = key.substr (pos+1);
  			//cout<<"str3:"<<str3<<endl;
  			char *portNo_del =  strcpy((char*)malloc(str3.length()+1), str3.c_str());	
			strcpy(del_mes,"2-");
			strcat(del_mes,ipaddress_del);
			strcat(del_mes,"-");
  			strcat(del_mes,portNo_del);
			strcat(del_mes,"-");
			strcat(del_mes,"DELETE_MEMBER");
			strcat(del_mes,"-");
			//cout<<"sending the eof message"<<endl;
			leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
			leaderAddr.sin_family = AF_INET;
			leaderAddr.sin_port = htons(port_leader);
			int result_join = server->send_String(leaderAddr,del_mes,sizeof(del_mes));
				//deleteMember(key);
			} else {
				memInfo[key] = alive;
			}
			} else {
				memInfo[key] = alive;
			}   
		}
	
	}




void ping_members(const boost::system::error_code& ) {

  //cout << "ping" << endl;
  char ping_str2[MESSAGE_SIZE];

  struct sockaddr_in everyone;
  struct sockaddr_in leader;
  char ping_str[MESSAGE_SIZE]; 
  char msg[MESSAGE_SIZE];

  //strcpy(ping_str, "7-");
  sprintf(ping_str2, "%d", portNo);

    //cout << "there is a leader after all" << endl;
    strcpy(ping_str, ipaddress);
    strcat(ping_str,"-");
    strcat(ping_str, ping_str2);
    strcat(ping_str,"-");
    //getListOfMembers();
    //cout << ping_str << endl;

  for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {


	char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
	char *ipVal = strtok (ipPort_all,":");

	size_t pos = (it1->first).find(":");      // position of "live" in str

  	std::string str3 = (it1->first).substr (pos+1);
  	//cout<<"str3 ping mem: "<<str3<<endl;
  	char *portVal =  strcpy((char*)malloc(str3.length()+1), str3.c_str());	


	//char *portVal = strtok (NULL,":");
	everyone.sin_addr.s_addr = inet_addr(ipVal);
	everyone.sin_family = AF_INET;
	everyone.sin_port = htons(atoi(portVal)+5);


	leader.sin_addr.s_addr = inet_addr(ip_leader);
	leader.sin_family = AF_INET;
	leader.sin_port = htons(port_leader+6);

	//cout<<"leader: " << port_leader <<endl;
	//cout << "portval: " << portVal << endl; 
	int pval = atoi(portVal);
	//if (port_leader != pval ){
		//cout << "insode" << endl;

	int rel1 = checklive_server->send_String(everyone,ping_str,sizeof(ping_str));

	struct timeval timeout={2,0};
	setsockopt(pingAck_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	int recvlen = pingAck_server->receive_String(leader,msg,sizeof(msg));
	//cout << "received this: " << msg << endl;
	if (recvlen >= 0) {
		//cout << "ack came" << endl;
		Member_Alive (it1->first, true);
	} else {
		Member_Alive (it1->first, false);
	}
	//}

  }
}

void *check_live_members(void *myvar){

    struct arg_struct *args = (struct arg_struct *)myvar;

    char temp_str[10];

    char str2[MESSAGE_SIZE];

    //cout << "printing ip: " << ip << endl;
    sprintf(temp_str, "%d", args->arg2_port);

    //cout << "there is a leader after all" << endl;
    strcpy(str2, args->arg1_ip);
	strcat(str2,":");
	strcat(str2, temp_str);
	std::unique_lock<std::mutex> lck(mtx);	
	 
		
			//cout << "leader: " << Leader() << " me: " << str2 << endl;
			while(true) {
			 	
			 		if (Leader().compare(string(str2)) == 0) {
						boost::asio::io_service io;
    	
    					boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));

        				t.async_wait(&ping_members);

        				io.run();
        			} else {
        				ready = false;
        				
  						while (!ready) {
  							//cout << "im waiting" << endl;
  							cv.wait(lck);
  							//cout << "notified" << endl;
  						}
        			}
        		
			}
}
void *deadLeader_handler(void *myvar) {

 	char * ip = (char *) myvar;
	struct sockaddr_in pingSenderAddr;
	struct sockaddr_in me;
	struct sockaddr_in everyone;
	vector<string> port_all;
	char str[MESSAGE_SIZE];
	char tempStr[MESSAGE_SIZE];
	struct sockaddr_in my;
	bool flag = false;

	//std::unique_lock<std::mutex> lck(mtx);
	//cout<<"Ip_lead"<<ip_lead<<endl; 
	//if (Leader().compare(ip_lead) != 0) {
		//cout << Leader() << " " << ip_lead << endl;
		while(true){

			if (stop == 0){
			//cout << "enter dead leader handler" << endl;
			char msg[MESSAGE_SIZE];
			me.sin_addr.s_addr = inet_addr(ip);
			me.sin_family = AF_INET;
			me.sin_port = htons(portNo+5);
			struct timeval timeout1={12,0};
			setsockopt(checklive_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout1,sizeof(struct timeval));
			if(checklive_server->receive_String(me,msg,sizeof(msg))<0){

				//std::unique_lock<std::mutex> lck1(mtx1);
				//ready1 = false;
				//cv1.notify_one();
				//cout << "dont get into this" << endl;
				string ip_lead = string(ip_leader) + string(":") + to_string(port_leader);
				deleteMember(ip_lead);

				string curr_ip = string(ipaddress) + string(":") + to_string(portNo);

			//char *portVal = strtok (NULL,":");
			for (map<string,vector<string>>::reverse_iterator it1=memberInfo.rbegin(); it1!=memberInfo.rend(); ++it1) {
			//for (vector<string>::reverse_iterator it1 = port_all.rbegin() ; it1 != port_all.rend(); ++it1){

			if (it1->first.compare(curr_ip) != 0) {
			strcpy(str,"5-YOU_ALIVE-");
			strcat(str,ipaddress);
			strcat(str,"-");
			sprintf(tempStr,"%d",portNo);
			strcat(str,tempStr);
			strcat(str,"-");
			string ip_Port = it1->first;//*it1;
			//cout<<"Ip_port:"<<ip_Port<<endl;
			//get port
			std::size_t pos = ip_Port.find(":");      // position of "live" in str
  			std::string str3 = ip_Port.substr (pos+1);
  			int portVal = stoi(str3);
  			//cout<<"PortVal:"<<portVal<<endl;
  			char *str4 = strcpy((char*)malloc(ip_Port.length()+1), ip_Port.c_str());
  			char* ippp = strtok(str4,":");
  			//cout<<ippp<<"ipPort"<<endl;
  			everyone.sin_addr.s_addr = inet_addr(ippp);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(portVal);
			//cout<<"The message ready be sent:"<<str<<endl;
				
			my.sin_addr.s_addr = inet_addr(ipaddress);
			my.sin_family = AF_INET;
			my.sin_port = htons(portNo+4);
			for (int i=0; i<4; i++) {
				struct timeval timeout5={2,0};
				setsockopt(Leader_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout5,sizeof(struct timeval));

				if (Leader_server->receive_String(my,msg,sizeof(msg))<0){

					if (i==3) {
						flag = true;
						break;
					}
					//cout << "danger zone" << endl;
					int rel1 = server->send_String(everyone,str,sizeof(str));
					//continue;
				}else{
        			//cout<<"the message received from bigger port-"<<msg<<endl;
					char *msg_received = strtok(msg,"-");
					flag = false;
					//cout<<msg_received<<endl;
					if (strcmp(msg_received,"YES_I_AM") == 0)
						break;
				}
			}
			if (!flag)
				break;
			
			
			} else { //i am the new leader
					isleader = true;
					string ipPort = string(ipaddress) + string(":") + to_string(portNo);
					//newLeader(ipPort);
					char joinMsg1[MESSAGE_SIZE];
					char tempMsg[MESSAGE_SIZE];
					char msg1[MESSAGE_SIZE];
					struct sockaddr_in leaderUpdate;
					struct sockaddr_in newleader;
					strcpy(joinMsg1,"NEW_LEADER");
		           	strcat(joinMsg1,"-");
					strcat(joinMsg1,ipaddress);
			  		strcat(joinMsg1,"-");
			  		sprintf(tempMsg,"%d",portNo);
			  		strcat(joinMsg1,tempMsg);
			  		strcat(joinMsg1,"-");

			  		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
				  		char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
						char *ipVal = strtok (ipPort_all,":");
						char *portVal = strtok (NULL,":");
				
						newleader.sin_addr.s_addr = inet_addr(ipaddress);
						newleader.sin_family = AF_INET;
						newleader.sin_port = htons(portNo+8);

						leaderUpdate.sin_addr.s_addr = inet_addr(ipVal);
						leaderUpdate.sin_family = AF_INET;
						leaderUpdate.sin_port = htons(atoi(portVal)+2);
						//for (int i = 0; i<3;i++) {
							int result_newleader = broadcast_server->send_String(leaderUpdate,joinMsg1,sizeof(joinMsg1) );
							struct timeval timeout6={2,0};
							setsockopt(broadcastAck_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout6,sizeof(struct timeval));
							if (broadcastAck_server->receive_String(newleader,msg1,sizeof(msg1))<0){
								continue;
							} 
							//else {
							// 	break;
							// }	
						//}//for 3 times
					}//for all members
					
				break;
			}//else
		} //for ends
		} else {
			//cout << " else part of deadLeader_handler "<< endl;
			char *ping_ip_addr = strtok(msg,"-");
			
			//cout<<"[DEBUG] In dispatcher Function,Ip_Address:"<<ping_ip_addr<<endl;
			char *ping_port_num = strtok(NULL,"-");
			
			//cout<<"[DEBUG] In dispatcher Function,port:"<<ping_port_num<<endl;
			 if (ping_ip_addr != NULL && ping_ip_addr[0] != '\0' && ping_port_num != NULL && ping_port_num[0] != '\0') {
				pingSenderAddr.sin_addr.s_addr = inet_addr(ping_ip_addr);
				pingSenderAddr.sin_family = AF_INET;
				pingSenderAddr.sin_port = htons(atoi(ping_port_num)+6);
						

				char ping_ack[MESSAGE_SIZE];
				strcpy(ping_ack,"P_OK");
			
				pingAck_server->send_String(pingSenderAddr,ping_ack,sizeof(ping_ack));
			}
		}	
	

		}//if stop
	}//while true
}//func

void *chat_send(void *myvar) {

	char * ip = (char *) myvar;
	struct sockaddr_in leaderAddr;
	char msg[MESSAGE_SIZE];
	struct sockaddr_in newClient;
	char gotmsg[MESSAGE_SIZE];

	newClient.sin_addr.s_addr = inet_addr(ipaddress);
	newClient.sin_family = AF_INET;
	newClient.sin_port = htons(portNo);

	while (true) {

			strcpy(msg,"2-");
			strcat(msg,ipaddress);
			strcat(msg,"-");
			string s = to_string(portNo);
			char const *pchar = s.c_str();
			strcat(msg,pchar);
			strcat(msg,"-");
			strcat(msg,username);
			strcat(msg,":");
			string str = holdbackMessage.front();
			//cout << "1. holdback message has this: " << str << endl;
			char *str1 =  strcpy((char*)malloc(str.length()+1), str.c_str());
			strcat(msg,str1);
			strcat(msg,"-");

			//std::unique_lock<std::mutex> lck1(mtx1);
			for (int i=0; i<3; i++) {
				//while (!ready1)
				//	cv1.wait(lck1);
				string l = "";

				while (l.compare("") == 0){
					//cout << "leader waiting..." << endl;
					l = Leader();
				}

				//cout << "before sending: " << l << endl;
				char* l1 = strcpy((char*)malloc(l.length()+1), l.c_str());
				strcpy(ip_leader, strtok(l1, ":") );
				std::size_t pos = l.find(":");      // position of "live" in str

				std::string str3 = l.substr (pos+1);
		  		char *port_leader1 =  strcpy((char*)malloc(str3.length()+1), str3.c_str());

		  		leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
				leaderAddr.sin_family = AF_INET;
				leaderAddr.sin_port = htons(atoi(port_leader1) );
				
				//cout<<"2. The msg before sending in chat_Send"<<msg<<endl;
				string g = msg;
				if (g.length() == 0) {
					continue;
				} 
				int result_join = server->send_String(leaderAddr,msg,sizeof(msg));
				
				struct timeval timeout11={2,0};
				setsockopt(chat_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout11,sizeof(struct timeval));

				int result_recv = chat_server->receive_String(newClient,gotmsg,sizeof(gotmsg));
				if (result_recv < 0) {
					continue;
				} else {
					//cout<<"5. About to pop: "<<gotmsg<<endl;
					string d = gotmsg;
					if (d.length() > 0) {
						char* token_gotmsg = strtok(gotmsg,"-");
						if(strcmp(token_gotmsg,"GOT_THE_MESSAGE") == 0){
							holdbackMessage.pop();
							//cout << "5. popped from holdback" << endl;
							break;
							//cout<<"The entry is erased"<<endl;
						}
					}
					
				}
				
			}

		}
}

void *join_send(void *myvar) {

	char * ip = (char *) myvar;
	struct sockaddr_in intermediate;
	struct sockaddr_in client;
	char msg[MESSAGE_SIZE];

	client.sin_addr.s_addr = inet_addr(ipaddress);
	client.sin_family = AF_INET;
	client.sin_port = htons(portNo+10);

	while(true){

		int result = join1_server->receive_String(client,msg,sizeof(msg));
		//cout<<"The message received in dispatcher is:"<<msg<<endl;

			if (isleader) {
				char*ipaddr_newcli = strtok(msg, "-");
				//cout<<"[DEBUG] In dispatcher Function,Ip Address:"<<ipaddress<<endl;
				char* port = strtok(NULL, "-");
				int port_newMem = atoi(port);
				//cout<<"[DEBUG] In dispatcher Function,port:"<<port_newMem<<endl;
				char* uname = strtok(NULL,"-");
				//cout<<"[DEBUG] In dispatcher Function,uname:"<<uname<<endl;
			
				join_handler (ipaddr_newcli, port, uname);
				
				} else {
					//cout<<"I am here in else"<<endl;
					char *i = strtok(msg,"-");
					char *p = strtok(NULL,"-");

					char replMsg[MESSAGE_SIZE];

					string l = Leader();
					while (l.compare("") == 0){
						//cout << "leader waiting..." << endl;
						l = Leader();
					}
					char *l1 = strcpy((char*)malloc(l.length()+1), l.c_str());
					strcpy(ip_leader, strtok(l1, ":") );

					std::size_t pos = l.find(":");      // position of "live" in str

  					std::string str3 = l.substr (pos+1);
  					char *port_leader1 =  strcpy((char*)malloc(str3.length()+1), str3.c_str());	
			

					strcpy(replMsg,"CALL_LEADER-");
					strcat(replMsg,ip_leader);
					strcat(replMsg,"-");
					//sprintf(tempMsg,"%d",port_leader);
  					strcat(replMsg,port_leader1);
  					strcat(replMsg,"-");

  					// declaring struct for reply message
  					intermediate.sin_addr.s_addr = inet_addr(i);
					intermediate.sin_family = AF_INET;
					intermediate.sin_port = htons(atoi(p)+9);
					//cout<<"reply intermediate msg"<<replMsg<<endl;
					int result_send = join_server->send_String(intermediate,replMsg,sizeof(replMsg));

				}
	}
}

void *dispatcher(void *myvar){

	char msg[MESSAGE_SIZE];
	char msg_ack[MESSAGE_SIZE];
	struct sockaddr_in client;
	struct sockaddr_in leaderAddr;
	struct sockaddr_in bully;
	struct sockaddr_in bully1;
	char tempMsg[MESSAGE_SIZE];
	char temp_port[MESSAGE_SIZE];
	char portNo_temp[MESSAGE_SIZE];
	char str [MESSAGE_SIZE];
	
	int ack_limit = 0;

	pthread_t th6;

	char * ip = (char *) myvar;

	client.sin_addr.s_addr = inet_addr(ipaddress);
	client.sin_family = AF_INET;
	client.sin_port = htons(portNo);

	while(true){

		//cout << "waiting to receive messages\n";
		int result = server->receive_String(client,msg,sizeof(msg));



		//cout<<"The message received in dispatcher is:"<<msg<<endl;
			if (result < 0){
				perror("error while sending the message \n");
				//continue;
			}
		
		char *ipaddr_newcli;
		char *port;
		char *uname;
		char *chatMsg;
		char *ipPort;
		char *uname_leader;
		char *sendMsg[MESSAGE_SIZE];
		char *flag_val;
		//cout <<"[DEBUG]Receive Message Function entered"<<endl;
		
		char *tokens = strtok(msg,"-");
		if (tokens == NULL || tokens[0] == '\0')
			continue;
		//cout<<"Token is"<<tokens<<endl;
		int t = atoi(tokens);
		switch(t) {
    			case 1:
				{ 
				
					//cout << "OK SENT\n";
            		break;
				}
				case 2:
				{
				//std::unique_lock<std::mutex> mlock (mutex_);
				//lock_.wait(mlock);
				char *ip_addr = strtok(NULL,"-");
				//cout<<"[DEBUG] In dispatcher Function,Ip_Address:"<<ip_addr<<endl;
				char *port_num = strtok(NULL,"-");
				//cout<<"[DEBUG] In dispatcher Function,PortNo:"<<portNo<<endl;
				chatMsg = strtok(NULL,"-");
				//mlock.unlock();
				//cout<<"3. In dispatcher Function: "<<ip_addr << " " << portNo << " " << chatMsg<<endl;
				if (ip_addr != NULL && ip_addr[0] != '\0' && port_num != NULL && port_num[0] != '\0' && chatMsg != NULL && chatMsg[0] != '\0')
					chat_handler (ip_addr,port_num,chatMsg);
				break;
				}
				case 3:
				{	
					//std::unique_lock<std::mutex> mlock (mutex_);					
					//lock_.wait(mlock);
					//cout << "case 3" << endl;
					char *seq_number = strtok(NULL,"-");

					if (seq_number != NULL && seq_number[0] != '\0') {


					int current_seq = stoi(string(seq_number));
					//cout<<"Current Seq_num "<<current_seq<<endl;
					char *ip_Port_val = strtok(NULL,"-");
					//cout<<"Previous seq_number "<<prev_seq<<endl;
					if (prev_seq == 0){
						prev_seq = current_seq - 1;
					} 
					if(current_seq == prev_seq + 1){
							char *message = strtok(NULL,"-");
							//mlock.unlock();
							//string m = string(message);
							if (message != NULL && message[0] != '\0') {
							//if(!printQueue.empty())
							//cout<<"8. Pushed to print Queue "<<message<<endl;
							printQueue.push(string(message));

							string l = Leader();
							while (l.compare("") == 0){
								//cout << "leader waiting..." << endl;
								l = Leader();
							}
							char *l1 = strcpy((char*)malloc(l.length()+1), l.c_str());
							strcpy(ip_leader, strtok(l1, ":") );

							std::size_t pos = l.find(":");      // position of "live" in str

		  					std::string str3 = l.substr (pos+1);
		  					char *port_leader1 =  strcpy((char*)malloc(str3.length()+1), str3.c_str());

							char msg_ack[MESSAGE_SIZE];
							strcpy(msg_ack,"GOT_CHAT_MESSAGE");
							strcat(msg_ack,"-");
							strcat(msg_ack,ip_leader);
							strcat(msg_ack,":");
							//sprintf(temp_port,"%d",port_leader);
  							strcat(msg_ack,port_leader1);
							strcat(msg_ack,"-");

							leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
							leaderAddr.sin_family = AF_INET;
							leaderAddr.sin_port = htons(atoi(port_leader1)+3);
							//cout<<"Ack sending:"<<msg_ack<<endl;
							int result_send = printAck_server->send_String(leaderAddr,msg_ack,sizeof(msg_ack));
							prev_seq = current_seq;
							cout<<printQueue.front()<<endl;
							printQueue.pop();
							ack_limit++;

						}
							
						}
						else if(ack_limit <3 && current_seq == prev_seq){
							//cout << "im getting here" << endl;
						//	mlock.unlock();
							int result_send = printAck_server->send_String(leaderAddr,msg_ack,sizeof(msg_ack));
							ack_limit++;
						}
						else{
						//	mlock.unlock();
							ack_limit = 0;
							printQueue.pop();

						}
					}
						break;	
					}
					case 4:
					{
					//lock_.wait(mlock);
					char *ip_addr = strtok(NULL,"-");
					//mlock.unlock();
					
					if (ip_addr != NULL && ip_addr[0] != '\0') {
						//cout<<"The ip_addr is:"<<ip_addr<<endl;
						cout<<"NOTICE "<<getUserName(string(ip_addr))<<" left the chat or crashed"<< endl;
						deleteMember(string(ip_addr));
					}
					break;
					}
					case 5:
					{   
						//cout<<"Case 5"<<endl;
						char msgByLeader[MESSAGE_SIZE];
						char *msga = strtok(NULL,"-");
						//mlock.unlock();
						
						if (msga != NULL && msga[0] != '\0') {
            				//cout<<"9. Alive or not " << msga<<endl;
         
							if (strcmp(msga,"YOU_ALIVE") == 0){
								char *ip_sender = strtok(NULL,"-");
								char *port_sender = strtok(NULL,"-");
								bully.sin_addr.s_addr = inet_addr(ip_sender);
								bully.sin_family = AF_INET;
								bully.sin_port = htons(atoi(port_sender)+4);
								strcpy(msgByLeader,"YES_I_AM-");
								//cout<<msgByLeader<<endl;
								int result_bully = Leader_server->send_String(bully,msgByLeader,sizeof(msgByLeader));
						
							}
						}
						break;
					}
					case 6:
					{
						//std::unique_lock<std::mutex> mlock (mutex_);						
						//lock_.wait(mlock);
					 	char *new_ipleader = strtok(NULL,"-");
					 	char *new_portleader = strtok(NULL,"-");
						//mlock.unlock();
					 	string oldleader = Leader();
						while (oldleader.compare("") == 0){
							//cout << "leader waiting..." << endl;
							oldleader = Leader();
						}

					 	deleteMember(oldleader);
					 	//string xv = string(new_ipleader);
					 	//string xv1 = string(new_portleader);
						if (new_ipleader != NULL && new_ipleader[0] != '\0' && new_portleader != NULL && new_portleader[0] != '\0') {
					 		string newleader = string(new_ipleader)+ string(":")+ string(new_portleader);;
					 		strcpy(ip_leader,new_ipleader);
					 		port_leader = atoi(new_portleader);
					 		newLeader(newleader);
					 		struct sockaddr_in leader_elect;
					 	 	leader_elect.sin_addr.s_addr = inet_addr(ip_leader);
						 	leader_elect.sin_family = AF_INET;
						 	leader_elect.sin_port = htons(port_leader+7);
					 		char msg_leader[MESSAGE_SIZE];
					 		strcpy(msg_leader,"NEWLEADER_UPDATED-");
					 		int result_bully = election_server->send_String(leader_elect,msg_leader,sizeof(msg_leader));
					 	}
					 	break;
					}	
				}//switch
			} 
		}


int main(int argc, char *argv[]) {
	
	char ip_member[IP_BUFSIZE];
	int port;
	struct sockaddr_in newClient;
	struct sockaddr_in leaderAddr;
	struct sockaddr_in oldClient;
	struct sockaddr_in leader;
	struct sockaddr_in newClient1;
	char joinMsg[MESSAGE_SIZE];
	char tempMsg[MESSAGE_SIZE];
	char recvMsg[MESSAGE_SIZE];
	char testmsg[MESSAGE_SIZE];
	char gotmsg[MESSAGE_SIZE];
	char testmsg1[MESSAGE_SIZE];

	int port_member; //port number of random member

	isleader = false;
	strcpy(ip_leader,"dummy");
	//cout<<"Before thread declare"<<endl;
	pthread_t  th1,th2,th3,th4,th5,th6,th7;

	if(argc < 2){
		cout<<"Invalid number of arguments"<<endl;
		return 1;
	}

	strcpy(username,argv[1]);
	//cout<<"Username of new client:"<<username<<endl;
	strcpy(ipaddress,getip());
	//cout<<"Ip Address of new client:"<<ipaddress<<endl;
	portNo = getPort();
	//cout<<"Port is of new client:"<<portNo<<endl;

	while (portNo == -1) {      
        portNo = getPort();
    }

    //cout<<"Port is of new client:"<<portNo<<endl;

	server = new server_Udp(portNo);
	chat_server = new server_Udp(portNo+1);
	broadcast_server = new server_Udp(portNo+2);
	printAck_server = new server_Udp(portNo+3);
	Leader_server = new server_Udp(portNo+4);
	checklive_server = new server_Udp(portNo+5);
	pingAck_server = new server_Udp(portNo+6);
	election_server = new server_Udp(portNo+7);
	broadcastAck_server = new server_Udp(portNo+8);
	join_server = new server_Udp(portNo+9);
	join1_server = new server_Udp(portNo+10);

	if(argc == 2 ){
		isleader = true;
		//cout <<username <<" started a new chat, listening on "<<ipaddress<<":"<<portNo<<endl;
		strcpy(ip_leader,ipaddress);
		port_leader = portNo;
		string ipPort = ipaddress + string(":") + to_string(portNo);
		addMember(ipPort,string(username),true);

		cout <<username<< " started a new chat, listening on " << ipPort << "\nSucceeded, current users:" << endl;

		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) { 
			cout << it1->second.at(0) << " " << it1->first << " " << "(Leader)" << endl;
				
		}
		
		cout << "Waiting for others to join..." << endl;

		leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
		leaderAddr.sin_family = AF_INET;
		leaderAddr.sin_port = htons(port_leader);
	}


	if(argc == 3){
		
		char *tokens = strtok (argv[2],":");
		strcpy(ip_member,tokens);
		while (tokens != NULL) {
    			tokens = strtok (NULL, ":");
    			port_member = atoi(tokens);
    			break;
  		}
  		//cout <<username <<" joining a new chat on "<<ip_member<<":"<<port_member<<", listening on"<<ipaddress<<":"<<portNo<<endl;
		oldClient.sin_addr.s_addr = inet_addr(ip_member);
		oldClient.sin_family = AF_INET;
		oldClient.sin_port = htons(port_member+10);

		newClient.sin_addr.s_addr = inet_addr(ipaddress);
		newClient.sin_family = AF_INET;
		newClient.sin_port = htons(portNo+9);

  		//strcpy(joinMsg,"1-");
  		strcat(joinMsg,ipaddress);
  		strcat(joinMsg,"-");
  		sprintf(tempMsg,"%d",portNo);
  		strcat(joinMsg,tempMsg);
  		strcat(joinMsg,"-");
  		strcat(joinMsg,username);
  		strcat(joinMsg,"-");
		
  		//cout<<"[DEBUG]The content is:"<<joinMsg<<endl;


  		int result_send = join1_server->send_String(oldClient,joinMsg,sizeof(joinMsg));
		//cout<<"The result join:"<<result_send<<endl;
  		struct timeval timeout12={2,0};
		setsockopt(join_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout12,sizeof(struct timeval));
		
  		int result_receive = join_server->receive_String(newClient,testmsg,sizeof(testmsg));
  		if (result_receive < 0) {
  			cout << "Sorry, no chat is active on " << ip_member << ":" << port_member << ", try again later. Bye." << endl;
  			exit(0);
  		}
  			char *tokens_join = strtok(testmsg,"-");
  			if(strcmp(tokens_join,"CALL_LEADER") == 0){
  				strcpy(ip_leader,strtok(NULL,"-"));
  				char *port_temp = strtok(NULL,"-");
  				port_leader = atoi(port_temp); 
  				
  				leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
				leaderAddr.sin_family = AF_INET;
				leaderAddr.sin_port = htons(port_leader+10);

				//strcpy(joinMsg,"1-");
  				strcat(joinMsg,ipaddress);
  				strcat(joinMsg,"-");
  				sprintf(tempMsg,"%d",portNo);
  				strcat(joinMsg,tempMsg);
  				strcat(joinMsg,"-");
  				strcat(joinMsg,username);
  				strcat(joinMsg,"-");

  				int result_send = join1_server->send_String(leaderAddr,joinMsg,sizeof(joinMsg));

  				result_receive = join_server->receive_String(newClient,testmsg,sizeof(testmsg));
  				char *tokens_join1 = strtok(testmsg,"-");
  				if(strcmp(tokens_join1,"OK") == 0){
  				//cout<<"Join is complete"<<endl;
  				}


  			} else if (strcmp(tokens_join,"OK") == 0){
  				strcpy(ip_leader,strtok(NULL,"-"));
  				char *port_temp = strtok(NULL,"-");
  				port_leader = atoi(port_temp);

  				leaderAddr.sin_addr.s_addr = inet_addr(ip_leader);
				leaderAddr.sin_family = AF_INET;
				leaderAddr.sin_port = htons(port_leader);

  				//cout<<"Join is complete"<<endl;
  			}

  			newClient1.sin_addr.s_addr = inet_addr(ipaddress);
			newClient1.sin_family = AF_INET;
			newClient1.sin_port = htons(portNo+2);

  			 result_receive = broadcast_server->receive_String(newClient1,testmsg1,sizeof(testmsg1));

  			if (result_receive < 0){
					perror("error while receiving the message \n");
					//continue;
				}
				else{

					getMapFromString (testmsg1);

					//cout<<"testmsg:"<<testmsg1<<endl;
				}

				cout << username << " joining a new chat on " << ip_member << ":" << port_member << "listening on" << endl;
				cout << ipaddress << ":" << portNo << endl;
				cout << "Succeeded, current users:"<< endl;

		//repl_handler(result_join);

		for (map<string,vector<string> >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) { 
			if (it1->second.at(1).compare("true") == 0)
				cout << it1->second.at(0) << " " << it1->first << " " << "(Leader)" << endl;
			else
				cout << it1->second.at(0) << " " << it1->first << endl;	
		}
  		}

  		


  		pthread_create(&th1,NULL,dispatcher,(void*) ipaddress);
  		pthread_create(&th2,NULL,latest_join,(void*) ipaddress);
		pthread_create(&th3,NULL,print_handler,(void*) ipaddress);

		struct arg_struct args;
   	 	strcpy(args.arg1_ip, ipaddress);
    		args.arg2_port = portNo;

		pthread_create(&th4,NULL,&check_live_members,(void*) &args);
		pthread_create(&th5,NULL,deadLeader_handler,(void*) ipaddress);

		pthread_create(&th6,NULL,chat_send,(void*) ipaddress);
		pthread_create(&th7,NULL,join_send,(void*) ipaddress);

		while(true){

		char chatMsg[MESSAGE_SIZE];
		char msg [MESSAGE_SIZE];
		char tempMsg1[MESSAGE_SIZE];
		string input;

		


		//Cin from user
		std::getline(cin,input);

		
		//cout << "just before getline leader: " << ip_leader << port_leader << endl;
		//strcpy(chatMsg, input.c_str());
		//cout << "1. pushing to holdbackMessage: " << input << endl;
		holdbackMessage.push (input);
		
	
	}
	pthread_join(th1,NULL);
	pthread_join(th2,NULL);
	pthread_join(th3,NULL);
	pthread_join(th4,NULL);
	pthread_join(th5,NULL);
	pthread_join(th6,NULL);
	pthread_join(th7,NULL);
}	

