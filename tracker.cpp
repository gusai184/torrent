#include "header.h"

inline void errorchecking(int n)
{
	if(n<0)
    {
      perror("Error while communicating to client ");
      exit(1);
    } 
}
int main( int argc, char *argv[] ) {
   int tracker_socket, client_socket, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n;
   
   /* First call to socket() function */
   tracker_socket = socket(AF_INET, SOCK_STREAM, 0);
   
   if (tracker_socket < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5001;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(tracker_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */
   cout<<"Before listen"<<endl;;
   listen(tracker_socket,SOMAXCONN);
   clilen = sizeof(cli_addr);
   cout<<"AFTER LISTEN"<<endl;


   fd_set fdset;
   FD_ZERO(&fdset);
   FD_SET(tracker_socket, &fdset );
   int maxfd=2;
   cout<<"Before while"<<endl;
   while(1)
   {

      cout<<"in while"<<endl;
      fd_set cfdset = fdset;
      cout<<"Before select";
      int countfd = select(50, &cfdset, NULL, NULL, NULL);
      cout<<"countfd is "<<countfd<<endl;
      if(countfd < 0)
      {
         cout<<"select error occured"<<endl;
         exit(1);
      }

      if(FD_ISSET(tracker_socket , &cfdset))
      {
         cout<<"tracker_socket has been pinged"<<endl;
         client_socket = accept(tracker_socket, (struct sockaddr *)&cli_addr,(socklen_t *)&clilen);
         printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , client_socket , inet_ntoa(cli_addr.sin_addr) , ntohs 
                  (cli_addr.sin_port));   
         if (client_socket < 0) {
            perror("ERROR on accept");
            exit(1);
         }
         send(client_socket,"connection established",sizeof("connection established"),0);
        
      }
      else
      {
         for(int i=0;i<50;i++)
         {
            int client_socket = i;
            if(FD_ISSET(client_socket, &cfdset))
            {
               cout<<"socket "<<client_socket<<endl;
               bzero(buffer, BUFFER_SIZE);
               int n =read(client_socket, buffer, BUFFER_SIZE);
               errorchecking(n);
              
               cout<<"Got message "<<buffer<<endl;

               try{
                  string msg = executeCommand(string(buffer));
                  cout<<msg<<endl;
                  int n = send(client_socket,msg.c_str(), msg.length(),0);
               	  errorchecking(n);

               }
               catch(string error_msg)
               {
                  cout<<"Erro : "<<error_msg<<endl;
                  int n = send(client_socket,error_msg.c_str(), error_msg.length(),0);
            	  errorchecking(n);
               }
            }
         }
      }
      FD_SET(client_socket, &fdset);
   }


   /* Accept actual connection from the client */
   
   
   // /* start communicating */
   // while(1)
   // {
   //    bzero(buffer,256);
   //    n = read( client_socket,buffer,255 );
      
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


 
   close(tracker_socket);
   return 0;
}