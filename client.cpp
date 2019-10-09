#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#define BUFFER_SIZE 256
using namespace std;

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   bool islogout=false;
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
	  n = recv(sockfd, buffer, BUFFER_SIZE, 0);
      if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
      }
      cout<<"Msg from server "<<buffer<<":";
     // printf("%s is buffer\n",buffer );
   
   //  Send commands to the tracker
   while(1)
   {
      cout<<"Enter commands "<<endl;
      bzero(buffer,BUFFER_SIZE);
      //fgets(buffer,255,stdin);
      cin.getline(buffer,BUFFER_SIZE);
      
      if(strcmp(buffer,"logout")==0)
        islogout = true;

      n = send(sockfd, buffer, strlen(buffer),0);
      if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
      }

      bzero(buffer,BUFFER_SIZE);
      n = recv(sockfd, buffer, BUFFER_SIZE, 0);
      if(n<0)
      {
        perror("Error Reading from tracker socket");
        exit(1);
      }

      cout<<buffer<<":"<<endl;
      if(islogout)
      	exit(1);
     

   }

  
   return 0;
}