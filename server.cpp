#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#define BUFFER_SIZE 256
using namespace std;

int main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5002;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */
   cout<<"before listen"<<endl;;
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   cout<<"AFTER LISTEN"<<endl;
   /* Accept actual connection from the client */
   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,(socklen_t *)&clilen);
	
   if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
   }
   
   // /* If connection is established then start communicating */
   // bzero(buffer,256);
   // n = read( newsockfd,buffer,255 );
   
   // if (n < 0) {
   //    perror("ERROR reading from socket");
   //    exit(1);
   // }
   
   // printf("Here is the MSG: %s\n",buffer);
 

   /* Now read server response */
   bzero(buffer,256);
   int filesize;
   n = recv(newsockfd, &filesize, sizeof(filesize), 0);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   cout<< "file size is "<<filesize<<endl;
   
   FILE *fp = fopen("img_d.JPG","w");

   while( (n=recv(newsockfd, buffer, BUFFER_SIZE,0))>0 && filesize>0 )
   {	
   		//cout<<"n : "<<n<<endl;
   		fwrite(buffer, sizeof(char), n, fp);
   		printf("%s",buffer);
   		filesize = filesize - n;
   }

   if (n < 0) {
      perror("ERROR while reading file from socket");
      exit(1);
   }
   cout<<"File downloaded successfully"<<endl;
   fclose(fp);
   close(sockfd);
   return 0;
}