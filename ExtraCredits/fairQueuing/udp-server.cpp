#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h> 

using namespace std;
 
class server_Udp
{
   public:
    server_Udp (int portNumber);
    ~server_Udp();
    int receive_String(struct sockaddr_in clientaddr,char *buf, size_t BUFSIZE );
    int send_String(struct sockaddr_in clientaddr,char *buf , size_t BUFSIZE );
    char* send_receive(struct sockaddr_in clientaddr,char *buf , size_t BUFSIZE, struct sockaddr_in recvaddr);
    int sockfd;

   private:
   int portNo;
   struct sockaddr_in serverAddr;
};

  server_Udp::server_Udp(int portNumber){
    portNo = portNumber;
    sockfd = socket(AF_INET, SOCK_DGRAM , 0);
    if(sockfd < 0 ){
      perror("Error while creating socket, Please try again \n");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNo);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd,(struct sockaddr *) &serverAddr, sizeof(serverAddr))< 0 ) {
      perror("Error while binding the socket \n");
  }
}

  server_Udp::~server_Udp(){
    close(sockfd);

  }

  int server_Udp::receive_String(struct sockaddr_in clientaddr,char *buf, size_t BUFSIZE){
    //cout << "[DEBUG]Receive String Line 1"<<endl;
    int clientlen = sizeof(struct sockaddr);
    //cout << "[DEBUG]Receive String Line 2"<<endl;
    fflush(stdout);
    int result_recv = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, (socklen_t *)&clientlen);
    //cout << "[DEBUG]Receive String Line 3"<<endl;
    return result_recv;
  }

  int server_Udp::send_String(struct sockaddr_in clientaddr,char *buf, size_t BUFSIZE){
    //cout << "[DEBUG]Send String Line 1"<<endl;
    int result_send = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr,(socklen_t)sizeof(struct sockaddr));
    return result_send;
  }

  char* server_Udp::send_receive(struct sockaddr_in clientaddr,char *buf, size_t BUFSIZE, struct sockaddr_in recvaddr){
    //cout << "[DEBUG]Send String Line 1"<<endl;
    int result_send = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr,(socklen_t)sizeof(struct sockaddr));
    usleep(1);
    //server_Udp::receive_String(clientaddr, buf, BUFSIZE); 
    int clientlen = sizeof(struct sockaddr);
    int result_recv = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &recvaddr, (socklen_t *)&clientlen);
    return buf;
  }
