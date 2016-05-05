#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h> 
#include <iostream>
#include <ctime>

#include "constants.h"

using namespace std;

char * getip() {
    int fd;
    struct ifreq ifr;
    char iface[] = DEFAULT_NETWORK_INTERFACE;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    //display result
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

bool validPorts(int p) {

    	int sockfd;
        struct sockaddr_in cliaddr;
        struct hostent *server;
        bool unused = false;
    	int portNo;
    	socklen_t len;

        //cout << "[DEBUG] I am here in isAvailable -1"<<endl;

        server = gethostbyname("localhost");

        memset(&cliaddr,0,sizeof(cliaddr));
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_port = htons(p);
        //cliaddr.sin_addr.s_addr = INADDR_ANY;

        //cout << "[DEBUG] I am here in isAvailable -2"<<endl;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        int response = bind(sockfd, (struct sockaddr *) &cliaddr,sizeof(cliaddr));
    if (response < 0) {
        return false;
    }

    close(sockfd);

    return true;
}

int getPort(){

        int sockfd;
        struct sockaddr_in cliaddr;
        struct hostent *server;
        bool unused = false;
		int portNo;
		socklen_t len;

        //cout << "[DEBUG] I am here in isAvailable -1"<<endl;

        server = gethostbyname("localhost");

        memset(&cliaddr,0,sizeof(cliaddr));
        cliaddr.sin_family = AF_INET;
        cliaddr.sin_port = htons(0);
        //cliaddr.sin_addr.s_addr = INADDR_ANY;

        //cout << "[DEBUG] I am here in isAvailable -2"<<endl;

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        int response = bind(sockfd, (struct sockaddr *) &cliaddr,sizeof(cliaddr));
        //cout << "[DEBUG] I am here in isAvailable -3"<<endl;
        //cout << "Response:"<<response<<endl;

	len = sizeof(cliaddr);
	if (getsockname(sockfd, (struct sockaddr *) &cliaddr, &len) == -1) {
      		perror("getsockname() failed");
      		return -1;
   	}

	portNo = (int) ntohs(cliaddr.sin_port);
   	//printf("Local port is: %d\n", portNo);

        close(sockfd);

        for (int i=1; i<10; i++) {
        	if (!validPorts(portNo+i) ) {
            	return -1;
       		}
    	}


        return portNo;
}