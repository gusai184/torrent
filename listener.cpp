#include "client_header"


void *serverthread(void *portnoadd)
{

   int listener_port = *(int *)portnoadd;
   cout<<"listing at listener_port "<<listener_port<<endl;
   int listener_fd, peer_fd,  peerlen;
   char buffer[256];
   struct sockaddr_in listener_address, peer_address;
   int  n;

   listener_fd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (listener_fd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   bzero((char *) &listener_address, sizeof(listener_address));

   
   listener_address.sin_family = AF_INET;
   listener_address.sin_addr.s_addr = INADDR_ANY;
   listener_address.sin_port = htons(listener_port);

   if (bind(listener_fd, (struct sockaddr *) &listener_address, sizeof(listener_address)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   listen(listener_fd,SOMAXCONN);
   peerlen = sizeof(peer_address);
   
   peer_fd = accept(listener_fd, (struct sockaddr *)&peer_address,(socklen_t *)&peerlen);
	
   if (peer_fd < 0) {
      perror("ERROR on accept");
      exit(1);
   }
   cout<<"listener Connection established to "<<ntohs(peer_address.sin_port)<<endl;
   bzero(buffer,BUFFER_SIZE);
   strcpy(buffer, "Connection established to listener");
   send(peer_fd, buffer, BUFFER_SIZE , 0);

   bzero(buffer,BUFFER_SIZE);
   recv(peer_fd, buffer, BUFFER_SIZE, 0);

   cout<<"File name is "<<buffer<<endl;
   sendFile(peer_fd);
 
}

void sendFile(int sockfd)
{
  
   FILE *fp = fopen("Assignmet_2.pdf","r");
   if(fp == NULL)
   {
   		perror("Error while opening file");
   		return;
   }
   fseek(fp,0,SEEK_END);
   int filesize = ftell(fp);
   char buffer[BUFFER_SIZE];
   rewind(fp);
   cout<<"Beforer file size is"<<filesize;
   int 	n = send(sockfd,&filesize,sizeof(filesize),0);
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
     
   while((n=fread(buffer,sizeof(char), BUFFER_SIZE, fp))>0 && filesize>0)
   {
     int x = send(sockfd, buffer, n, 0);
      
      if (x < 0) {
        perror("ERROR while writing file to socket");
        exit(1);
      }
       //cout<<n<<" bytes send"<<endl;
       buffer[n]='\0';
      // printf("%s",buffer);
       memset(buffer, '\0', BUFFER_SIZE);
       filesize = filesize - n;
   }
}
