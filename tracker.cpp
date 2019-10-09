#include "header.h"
unordered_map<int, struct sockaddr_in> fdsockmap;

inline void errorchecking(int n)
{
	if(n<0)
    {
      perror("Error while communicating to client ");
      exit(1);
    } 
}
int main( int argc, char *argv[] ) {
   int tracker_fd, client_fd, portno, clilen;
   char buffer[256];		
   struct sockaddr_in server_address, client_address;
   int  n;
   
   /* First call to socket() function */
   tracker_fd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (tracker_fd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &server_address, sizeof(server_address));
   portno = 5001;
   
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = INADDR_ANY;
   server_address.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(tracker_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */
   
   listen(tracker_fd,SOMAXCONN);
   clilen = sizeof(client_address);
   


   fd_set fdset;
   FD_ZERO(&fdset);
   FD_SET(tracker_fd, &fdset );
   int maxfd=2;
   
   init();

   while(1)
   {

   
      fd_set cfdset = fdset;
      
      int countfd = select(50, &cfdset, NULL, NULL, NULL);
    
      if(countfd < 0)
      {
         cout<<"select error occured"<<endl;
         exit(1);
      }

      if(FD_ISSET(tracker_fd , &cfdset))
      {
        
         client_fd = accept(tracker_fd, (struct sockaddr *)&client_address,(socklen_t *)&clilen);
         printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , client_fd , inet_ntoa(client_address.sin_addr) , ntohs 
                  (client_address.sin_port));   
         if (client_fd < 0) {
            perror("ERROR on accept");
            exit(1);
         }
         fdsockmap[client_fd] = client_address;
         send(client_fd,"connection established",sizeof("connection established"),0);
        
      }
      else
      {
         for(int i=0;i<50;i++)
         {
            int client_fd = i;
            if(FD_ISSET(client_fd, &cfdset))
            {
               cout<<"socket "<<client_fd<<endl;
               bzero(buffer, BUFFER_SIZE);
               int n =read(client_fd, buffer, BUFFER_SIZE);
               errorchecking(n);
               
               if(n==0)
               {
               		cout<<"client disconnetcted : "<<endl;
               		close(client_fd);
               }
               cout<<"Got message "<<buffer<<endl;

               try{
                  string msg = executeCommand(string(buffer), client_fd);
                  cout<<msg<<endl;
                  int n = send(client_fd,msg.c_str(), msg.length(),0);
               	  errorchecking(n);

               }
               catch(string error_msg)
               {
                  cout<<"Erro : "<<error_msg<<endl;
                  int n = send(client_fd,error_msg.c_str(), error_msg.length(),0);
            	  errorchecking(n);
               }
            }
         }
      }
      FD_SET(client_fd, &fdset);
   }


   /* Accept actual connection from the client */
   
   
   // /* start communicating */
   // while(1)
   // {
   //    bzero(buffer,256);
   //    n = read( client_fd,buffer,255 );
      
   //    if (n < 0) {
   //       perror("ERROR reading from socket");
   //       exit(1);
   //    }
      
   //    printf("Here is the command: %s\n",buffer);

   //    if(strcmp(buffer,"Exit")==0)
   //    {
   //      break;
   //    }
   // }    


 
   close(tracker_fd);
   return 0;
}