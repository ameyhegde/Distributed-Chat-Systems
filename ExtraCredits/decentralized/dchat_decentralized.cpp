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
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/time.h>

#include "constants.h"
#include "ipPort.cpp"
#include "udp-server.cpp"
#include "member-p.cpp"
#include "blockingQueue.h"

using namespace std;

// Local Vector
vector<char*> holdbackMessage;
map<string,vector<int> > portInfo;

// Blocking Queue
 BlockingQueue<string> sequencerQueue;
// BlockingQueue<string> holdbackQueue;
// BlockingQueue<string> printQueue;
// Different Servers
server_Udp *server;
server_Udp *join_server;
server_Udp *chat_server;
server_Udp *broadcast_server;
server_Udp *printAck_server;
server_Udp *Leader_server;
server_Udp *checklive_server;
server_Udp *pingAck_server;

char ipaddress[IP_BUFSIZE];
int portNo;
char username[UNAME_BUFSIZE];
int xRan = 0;

void join_handler(char *ipaddr_new, char *port, char *uname) {
	
	struct sockaddr_in sender;
	struct sockaddr_in everyone;

	char joinMsg[MESSAGE_SIZE];
	char tempPort[MESSAGE_SIZE];
	char tempMsg[MESSAGE_SIZE];
	char joinMsg1[MESSAGE_SIZE];
		
		string ipPort = ipaddr_new + string(":") + string(port);

		sender.sin_addr.s_addr = inet_addr(ipaddr_new);
		sender.sin_family = AF_INET;
		sender.sin_port = htons(atoi(port));

		strcpy(joinMsg,"OK-");
	
		addMember(ipPort,string(uname));

		int retval = server->send_String(sender,joinMsg,sizeof(joinMsg));

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
		for (map<string,string>::iterator it2=memberInfo.begin(); it2!=memberInfo.end(); ++it2) {

			char* md = strcpy((char*)malloc( (it2->first).length()+1), (it2->first).c_str());
			strcat(map_details,md);
			strcat(map_details,"-");
			char* md1 = strcpy((char*)malloc((it2->second).length()+1), (it2->second).c_str());
			strcat(map_details,md1);
			strcat(map_details,"-");
			strcat(map_details,"~");

		}
		int rel2 = broadcast_server->send_String(sender,map_details,sizeof(map_details));

		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
						char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
			std::size_t pos = (it1->first).find(":");      // position of "live" in str

  			std::string str3 = (it1->first).substr (pos+1);				
			char *ipVal = strtok (ipPort_all,":");

			char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
			everyone.sin_addr.s_addr = inet_addr(ipVal);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(atoi(portVal)+2);

			int rel1 = broadcast_server->send_String(everyone,joinMsg1,sizeof(joinMsg1));

		}
}

void chat_handler (char *ip_addr,char *port_num,char *chatMsg) {

		struct sockaddr_in my;
		struct sockaddr_in everyone;

		char del_mes[MESSAGE_SIZE];
		char msg[MESSAGE_SIZE];

		map<string,string>::iterator it;

		string ipPort = ip_addr + string(":") + string(port_num);

		it = memberInfo.find(string(ipPort));
		if(strcmp(chatMsg,"DELETE_MEMBER") == 0) {
			
			//cout<<"Inside chat_handler"<<endl;

			strcpy(del_mes,"4");
			strcat(del_mes,"-");
			strcat(del_mes,ip_addr);
			strcat(del_mes,"-");
			strcat(del_mes,port_num);
			strcat(del_mes,"-");
		
			everyone.sin_addr.s_addr = inet_addr(ipaddress);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(portNo);

			int rel1 = server->send_String(everyone,del_mes,sizeof(del_mes));
	} else{
	string msg = ip_addr + string(":") + string (port_num) + string ("-") + string (chatMsg);
			
			sequencerQueue.push(msg);
					

}
}

void *latest_join(void *myvar){
	char msg[MESSAGE_SIZE];
	struct sockaddr_in client;
	char tempMsg[MESSAGE_SIZE];
	char * ip = (char *) myvar;
	client.sin_addr.s_addr = inet_addr(ipaddress);
	client.sin_family = AF_INET;
	//char *tokens2 = strtok(NULL,":");
	client.sin_port = htons(portNo+2);
	string ipPort = ipaddress + string(":") + to_string(portNo);
	char * client1 = strcpy((char*)malloc(ipPort.length()+1), ipPort.c_str());
	while(true){
		int result = broadcast_server->receive_String(client,msg,sizeof(msg));
			//cout<<"The message received in latest_join is:"<<msg<<endl;
				if (result < 0){
					perror("error while sending the message \n");
					//continue;
				}
				else{
					char* token_gotmsg;
					char* token_gotmsg1 = strtok(msg,"-");
					if(strcmp(token_gotmsg1,"NEWEST_CLIENT") == 0){
					char *token_gotmsg2 = strtok(NULL,"-");
					token_gotmsg = strtok(NULL,"-");
					//strcpy(token_gotmsg,"false");
					addMember(token_gotmsg2,token_gotmsg);
					if (strcmp(token_gotmsg2,client1) != 0){
					cout<<"NOTICE " <<token_gotmsg <<" joined the chat "<<token_gotmsg2<<endl;
					}
					listOfUsers();
					}
				}

	}

}

void *print_handler(void *myvar){

 	char * ip = (char *) myvar;

	while(true){
 
			string msgToSend = sequencerQueue.front();
			char *Msg2 = strcpy((char*)malloc((msgToSend).length()+1), (msgToSend).c_str());
			char *msg = strtok(Msg2,"-");
			char *msgtoPrint = strtok(NULL,"-");
			cout<<msgtoPrint<<endl;

			sequencerQueue.pop();
	}
}

void * join_send(void *myvar){
	char * ip = (char *) myvar;

	struct sockaddr_in client;

	client.sin_addr.s_addr = inet_addr(ipaddress);
	client.sin_family = AF_INET;
	client.sin_port = htons(portNo+6);

	char msg[MESSAGE_SIZE];

	while (true){

				int result = join_server->receive_String(client,msg,sizeof(msg));
				
				char *ipaddr_newcli = strtok(msg, "-");
				//cout<<"[DEBUG] In dispatcher Function,Ip Address:"<<ipaddress<<endl;
				char *port = strtok(NULL, "-");
				//int port_newMem = atoi(port);
				//cout<<"[DEBUG] In dispatcher Function,port:"<<port_newMem<<endl;
				char *uname = strtok(NULL,"-");
				//cout<<"[DEBUG] In dispatcher Function,uname:"<<uname<<endl;
			
				join_handler (ipaddress, port, uname);

	}
}

void *dispatcher(void *myvar){

	char msg[MESSAGE_SIZE];
	char msg_ack[MESSAGE_SIZE];
	char sendMsg1[MESSAGE_SIZE];
	struct sockaddr_in client;
	struct sockaddr_in intermediate;
	struct sockaddr_in leaderAddr;
	struct sockaddr_in bully;
	struct sockaddr_in priority;
	struct sockaddr_in sender;
	char tempMsg[MESSAGE_SIZE];
	char temp_port[MESSAGE_SIZE];

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
		//cout<<"Token is"<<tokens<<endl;
		int t = atoi(tokens);
		switch(t) {
    			case 1:
				{ 
           		break;
				}
				 case 2:
				 {
				 char *ip_addr = strtok(NULL,"-");
				 //cout<<"[DEBUG] In dispatcher Function,Ip_Address:"<<ip_addr<<endl;
				 char *port_num = strtok(NULL,"-");
				 //cout<<"[DEBUG] In dispatcher Function,PortNo:"<<portNo<<endl;
				 chatMsg = strtok(NULL,"-");
				 //cout<<"[DEBUG] In dispatcher Function,chatMsg:"<<chatMsg<<endl;
				 if (ip_addr != NULL && ip_addr[0] != '\0' && port_num != NULL && port_num[0] != '\0' && chatMsg != NULL && chatMsg[0] != '\0')
				 chat_handler (ip_addr,port_num,chatMsg);
					break;
				 }
					case 4:
				 	{
				 	char *ip_addr_die = strtok(NULL,"-");
				 	char *port_num_die = strtok(NULL,"-");
				 	//cout<<"The ip_addr is:"<<ip_addr<<endl;
				 	string ip_addr = ip_addr_die + string(":") + port_num_die;
				 	cout<<"NOTICE "<<getUserName(string(ip_addr))<<" left the chat or crashed"<< endl;
				 	
				 	deleteMember(string(ip_addr));

				 	strcpy(sendMsg1,"MEMBER_DELETED-");

				 	sender.sin_addr.s_addr = inet_addr(ip_addr_die);
					sender.sin_family = AF_INET;
					sender.sin_port = htons(atoi(port_num_die));

					int retval = chat_server->send_String(sender,sendMsg1,sizeof(sendMsg1));
					deleteMember(string(ip_addr));
				 	break;
				 	}
				case 5:
					{
						char *priority_ip = strtok(NULL,"-");
						char *priority_port = strtok(NULL,"-");
						char *priority_number = strtok(NULL,"-");
						char temp[MESSAGE_SIZE];
						priority.sin_addr.s_addr = inet_addr(priority_ip);
						priority.sin_family = AF_INET;
						priority.sin_port = htons(atoi(priority_port)+1);
						char priorityMsg[MESSAGE_SIZE];
						cout<<"checking priority"<<endl;
						
						if(holdbackMessage.empty()){
							strcpy(priorityMsg,"YES-");
							sprintf(temp,"%d",xRan);
							strcat(priorityMsg,temp);
						}
						else{
							if(xRan>atoi(priority_number)){
								strcpy(priorityMsg,"NO-");
							}
							else{
								strcpy(priorityMsg,"YES-");
								sprintf(temp,"%d",xRan);
								strcat(priorityMsg,temp);
							}
						}
						int result_priority = chat_server->send_String(priority, priorityMsg,sizeof(priorityMsg));
						break;
					}
		
				}
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
  			cout<<"str3:"<<str3<<endl;
  			char *portNo_del =  strcpy((char*)malloc(str3.length()+1), str3.c_str());	
			strcpy(del_mes,"2-");
			strcat(del_mes,ipaddress_del);
			strcat(del_mes,"-");
  			strcat(del_mes,portNo_del);
			strcat(del_mes,"-");
			strcat(del_mes,"DELETE_MEMBER");
			strcat(del_mes,"-");
			//cout<<"sending the eof message"<<endl;
			leaderAddr.sin_addr.s_addr = inet_addr(ipaddress);
			leaderAddr.sin_family = AF_INET;
			leaderAddr.sin_port = htons(portNo);
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
  	strcpy(ping_str, "6-");
    strcpy(ping_str, ipaddress);
    strcat(ping_str,"-");
    strcat(ping_str, ping_str2);
    strcat(ping_str,"-");
    getListOfMembers();
    //cout << ping_str << endl;
    leader.sin_addr.s_addr = inet_addr(ipaddress);
	leader.sin_family = AF_INET;
	leader.sin_port = htons(portNo+4);

	  for (map<string,string >::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {


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


		//cout<<"leader: " << port_leader <<endl;
		//cout << "portval: " << portVal << endl; 
		int pval = atoi(portVal);
		//if (port_leader != pval ){
		//cout << "ping to everyone" << ping_str<<endl;

		int rel1 = checklive_server->send_String(everyone,ping_str,sizeof(ping_str));
		//cout<<"sent the string"<<endl;
		struct timeval timeout={1,0};
		setsockopt(pingAck_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
		int recvlen = pingAck_server->receive_String(leader,msg,sizeof(msg));
		//cout << "received this: " << msg << endl;
		if (recvlen >= 0) {
			//cout << "ack came" << endl;
			//cout<<"got back response"<<endl;
			Member_Alive (it1->first, true);
		} else {
					//cout<<"didn't get back response"<<endl;
					Member_Alive (it1->first, false);
				}
		}
		//}

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
			
	 
			//cout << "leader: " << Leader() << " me: " << str2 << endl;
			while(true) {
				boost::asio::io_service io;
    	
    				boost::asio::deadline_timer t(io, boost::posix_time::seconds(4));

        			t.async_wait(&ping_members);

        			io.run();
        		}
   
}

void *deadLeader_handler(void *myvar){
	while(true){
						//cout<<"In deadLeader_handler"<<endl;
						struct sockaddr_in pingSenderAddr;
						struct sockaddr_in me;
						me.sin_addr.s_addr = inet_addr(ipaddress);
						me.sin_family = AF_INET;
						me.sin_port = htons(portNo+5);
						char msg[MESSAGE_SIZE];
						struct timeval timeout1={6,0};
						setsockopt(checklive_server->sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout1,sizeof(struct timeval));
						checklive_server->receive_String(me,msg,sizeof(msg));
						char *ping_ip_addr = strtok(msg,"-");
						char *ping_port_num = strtok(NULL,"-");
						pingSenderAddr.sin_addr.s_addr = inet_addr(ping_ip_addr);
						pingSenderAddr.sin_family = AF_INET;
						pingSenderAddr.sin_port = htons(atoi(ping_port_num)+4);
						

						char ping_ack[MESSAGE_SIZE];
						strcpy(ping_ack,"P_OK");
			
						//cout<<"the message sent back from deadLeader_handler "<< ping_ack<<endl;
						pingAck_server->send_String(pingSenderAddr,ping_ack,sizeof(ping_ack));
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
	struct sockaddr_in everyone;
	struct sockaddr_in priority_add;
	char joinMsg[MESSAGE_SIZE];
	char tempMsg[MESSAGE_SIZE];
	char recvMsg[MESSAGE_SIZE];
	char testmsg[MESSAGE_SIZE];
	char gotmsg[MESSAGE_SIZE];
	char testmsg1[MESSAGE_SIZE];
	int sockfd;
	unsigned long long Ag = 0;
	int t = 0;

	int port_member; //port number of random member

	//cout<<"Before thread declare"<<endl;
	pthread_t  th1,th2,th3,th4,th5,th6;

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

	server = new server_Udp(portNo);
	chat_server = new server_Udp(portNo+1);
	broadcast_server = new server_Udp(portNo+2);
	printAck_server = new server_Udp(portNo+3);
	pingAck_server = new server_Udp(portNo+4);
	checklive_server = new server_Udp(portNo+5);
	join_server = new server_Udp(portNo+6);


	if(argc == 2 ){
		string ipPort = ipaddress + string(":") + to_string(portNo);
		addMember(ipPort,string(username));

		cout <<username<< " started a new chat, listening on " << ipPort << "\nSucceeded, current users:" << endl;

		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); it1++) { 
			cout << it1->second << " " << it1->first << " " << endl;
		}
		
		cout << "Waiting for others to join..." << endl;

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
		oldClient.sin_port = htons(port_member+6);

		newClient.sin_addr.s_addr = inet_addr(ipaddress);
		newClient.sin_family = AF_INET;
		newClient.sin_port = htons(portNo);

  		//strcpy(joinMsg,"1-");
  		strcat(joinMsg,ipaddress);
  		strcat(joinMsg,"-");
  		sprintf(tempMsg,"%d",portNo);
  		strcat(joinMsg,tempMsg);
  		strcat(joinMsg,"-");
  		strcat(joinMsg,username);
  		strcat(joinMsg,"-");
		
  		//cout<<"[DEBUG]The content is:"<<joinMsg<<endl;


  		int result_send = join_server->send_String(oldClient,joinMsg,sizeof(joinMsg));
		//cout<<"The result join:"<<result_send<<endl;

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 2000000;
		setsockopt(server->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv));
  		int result_receive = server->receive_String(newClient,testmsg,sizeof(testmsg));
  			char *tokens_join = strtok(testmsg,"-");
			 if (strcmp(tokens_join,"OK") == 0){

  				cout<<"Join is complete"<<endl;
  			
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

					cout<<"testmsg:"<<testmsg1<<endl;
				}

				cout << username << "joining a new chat on " << ip_member << ":" << port_member << "listening on" << endl;
				cout << ipaddress << ":" << portNo << endl;
				cout << "Succeeded, current users:"<< endl;

		//repl_handler(result_join);

		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) { 
				cout << it1->second << " " << it1->first << endl;	
  		}

	}


  		pthread_create(&th1,NULL,dispatcher,(void*) ipaddress);
  		pthread_create(&th2,NULL,latest_join,(void*) ipaddress);
		pthread_create(&th3,NULL,print_handler,(void*) ipaddress);
		// pthread_create(&th4,NULL,recv_msg,(void*) ipaddress);
		struct arg_struct args;
   	 	strcpy(args.arg1_ip, ipaddress);
    		args.arg2_port = portNo;

		pthread_create(&th4,NULL,&check_live_members,(void*) &args);
		pthread_create(&th5,NULL,deadLeader_handler,(void*) ipaddress);
		pthread_create(&th6,NULL,join_send,(void*) ipaddress);
		
		newClient.sin_addr.s_addr = inet_addr(ipaddress);
		newClient.sin_family = AF_INET;
		newClient.sin_port = htons(portNo);

		while(true){

		char chatMsg[MESSAGE_SIZE];
		char msg [MESSAGE_SIZE];
		char tempMsg1[MESSAGE_SIZE];
		string input;
		//Cin from user
		getline(cin,input);
		strcpy(chatMsg, input.c_str());
		//cout<<"After getline:"<<chatMsg<<endl;
		holdbackMessage.push_back (chatMsg);
		if (cin.eof()){
			//cout<<"it's end of line"<<endl;
			char del_mes [MESSAGE_SIZE];
			strcpy(del_mes,"2-");
			strcat(del_mes,ipaddress);
			strcat(del_mes,"-");
			sprintf(tempMsg1,"%d",portNo);
  			strcat(del_mes,tempMsg1);
			strcat(del_mes,"-");
			strcat(del_mes,"DELETE_MEMBER");
			strcat(del_mes,"-");
			//cout<<"sending the eof message"<<endl;
			int result_join = server->send_String(newClient,del_mes,sizeof(del_mes));
			//cout<<"sent the eof message"<<endl;
			// int result_recv = broadcast_server->receive_String(newClient,gotmsg,sizeof(gotmsg));
			// 	char* token_got_msg = strtok(gotmsg,"-");
			// if(strcmp(token_got_msg,"DELETE_MEMBER") == 0){
			// 	char *token_got_ip = strtok(NULL,"-");
			// 	char *token_got_port = strtok(NULL,"-");
				exit(0);
			// 	string ipPort_del = token_got_ip + string(":") + string(token_got_port);
			
			// 	deleteMember(ipPort_del);
			//}
			
			
		}

		else {
			int not_get_it = 0;
			char tempvar[MESSAGE_SIZE];
			while(not_get_it == 0){
				xRan=rand()%1000+1; // Randomizing the number between 1-1000.
				if(xRan < Ag){
					xRan = Ag;
				}
				xRan++;
				strcpy(msg,"5-");
		 		strcat(msg,ipaddress);
				strcat(msg,"-");
				string s = to_string(portNo);
				char const *pchar = s.c_str();
				strcat(msg,pchar);
				strcat(msg,"-");
				sprintf(tempvar,"%d",xRan);
				strcat(msg,"-");
				not_get_it = 1;
				//cout<<"before priority for"<<endl;
				for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
									char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
			std::size_t pos = (it1->first).find(":");      // position of "live" in str

  			std::string str3 = (it1->first).substr (pos+1);				
			char *ipVal = strtok (ipPort_all,":");

			char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
					
						everyone.sin_addr.s_addr = inet_addr(ipVal);
						everyone.sin_family = AF_INET;
						everyone.sin_port = htons(atoi(portVal));
						if(strcmp(ipVal,ipaddress) != 0 && portNo != atoi(portVal)){
							int rel1 = server->send_String(everyone,msg,sizeof(msg));
							int result_rec = chat_server->receive_String(newClient1,gotmsg,sizeof(gotmsg));
						}
						else{
							strcpy(gotmsg, "YES-");
							sprintf(tempvar,"%d",xRan);
							strcat(msg,"-");
						}
					char* token_gotmsg = strtok(gotmsg,"-");
					if(strcmp(token_gotmsg,"YES") != 0){
						char* second_msg = strtok(NULL, "-");
						if(atoi(second_msg)>t){
							t = atoi(second_msg);
						}
						not_get_it = 0;
						//cout<<"The entry is erased"<<endl;
					}
					//cout<<"send priority to everyone"<<endl;
				}
			}
			xRan = 0;
			if(Ag<t){
				Ag = t;
			}
			// else if(Ag == 1000){
			// 	Ag = 0;
			// }
			t = 0;
		strcpy(msg,"2-");
		strcat(msg,ipaddress);
		strcat(msg,"-");
		string s = to_string(portNo);
		char const *pchar = s.c_str();
		strcat(msg,pchar);
		strcat(msg,"-");
		strcat(msg,username);
		strcat(msg,":");
		char *str = holdbackMessage.front();
		strcat(msg,str);
		strcat(msg,"-");
			
		for (map<string,string>::iterator it1=memberInfo.begin(); it1!=memberInfo.end(); ++it1) {
						char *ipPort_all = strcpy((char*)malloc( (it1->first).length()+1), (it1->first).c_str());
			std::size_t pos = (it1->first).find(":");      // position of "live" in str

  			std::string str3 = (it1->first).substr (pos+1);				
			char *ipVal = strtok (ipPort_all,":");

			char *portVal = strcpy((char*)malloc( (str3).length()+1), (str3).c_str());
			everyone.sin_addr.s_addr = inet_addr(ipVal);
			everyone.sin_family = AF_INET;
			everyone.sin_port = htons(atoi(portVal));
			//cout<<"The message ready be sent:"<<str<<endl;
			int rel1 = server->send_String(everyone,msg,sizeof(msg));


		}
	
	}	
	
}
		 pthread_join(th1,NULL);
		 pthread_join(th2,NULL);
		 pthread_join(th3,NULL);
		 pthread_join(th4,NULL);
		 pthread_join(th5,NULL);
}
