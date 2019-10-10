#include "client_header"

void * downloadingthread(void * port)
{
	
	int listener_port = *(int *)port, n, listener_fd;
	string filename = "abc.txt";

	struct sockaddr_in listener_address;
	struct hostent *listener_ip;
	char buffer[BUFFER_SIZE];

    listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) {
    	
    	perror("ERROR opening socket");
        exit(1);
    }
	
    listener_ip = gethostbyname("127.0.0.1");
    if (listener_ip == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &listener_address, sizeof(listener_address));
    listener_address.sin_family = AF_INET;
    bcopy((char *)listener_ip->h_addr, (char *)&listener_address.sin_addr.s_addr, listener_ip->h_length);
    listener_address.sin_port = htons(listener_port);
    
    if (connect(listener_fd, (struct sockaddr*)&listener_address, sizeof(listener_address)) < 0) {
        cout<<"error no is "<<errno<<endl;
        perror("ERROR connecting");
        return NULL;
    }
   
    
    n = recv(listener_fd, buffer, BUFFER_SIZE, 0);
    if (n < 0) {

        perror("ERROR writing to socket");
        exit(1);
    }

    cout<<"Msg from listener "<<listener_port<< " : "<<buffer<<endl;
    send(listener_fd, filename.c_str(), filename.length(), 0);
	readFile(listener_fd);   
 	
 	return NULL;
}

void readFile(int newsockfd)
{
   char buffer[BUFFER_SIZE];
   bzero(buffer,256);
   int filesize;
   int n = recv(newsockfd, &filesize, sizeof(filesize), 0);
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   cout<< "file size is "<<filesize<<endl;
   
   FILE *fp = fopen("downloaded","w");

   while(filesize>0 && (n=recv(newsockfd, buffer, BUFFER_SIZE,0))>0  )
   {	
   		cout<<"n : "<<n<<endl;
   		fwrite(buffer, sizeof(char), n, fp);
   		//printf("%s",buffer);
   		filesize = filesize - n;
   }

   if (n < 0) {
      perror("ERROR while reading file from socket");
      exit(1);
   } 
   cout<<"File downloaded successfully"<<endl;
   fclose(fp);

}


void download_file()
{
	int listener_port;
	cout<<"Enter client port where to download_file"<<endl;
	cin>>listener_port;
	cin.ignore();  
	pthread_t thread_id; 
    pthread_create(&thread_id, NULL, downloadingthread, &listener_port); 
	pthread_join(thread_id, NULL);
}

int main(int argc, char *argv[]) {
   int tracker_fd, tracker_port, listener_port, n;
   struct sockaddr_in tracker_address;
   struct hostent *tracker_ip;
   
   char buffer[256];
   bool islogout=false;

   cout<<"Enter client's  listing port "<<endl;
   cin >> listener_port;
   cin.ignore();  
   pthread_t thread_id; 
   pthread_create(&thread_id, NULL, serverthread, &listener_port); 
    
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }   	    


   tracker_port = atoi(argv[2]);
   tracker_fd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (tracker_fd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   tracker_ip = gethostbyname(argv[1]);
   
   if (tracker_ip == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &tracker_address, sizeof(tracker_address));
   tracker_address.sin_family = AF_INET;
   bcopy((char *)tracker_ip->h_addr, (char *)&tracker_address.sin_addr.s_addr, tracker_ip->h_length);
   tracker_address.sin_port = htons(tracker_port);
   

   if (connect(tracker_fd, (struct sockaddr*)&tracker_address, sizeof(tracker_address)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   cout<<"reached "<<endl;
   n = recv(tracker_fd, buffer, BUFFER_SIZE, 0);
  
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   cout<<"Msg from tracker_ip "<<buffer<<":";
 
   while(1)
   {
      cout<<"Enter commands "<<endl;
      bzero(buffer,BUFFER_SIZE);
      //fgets(buffer,255,stdin);
      cin.getline(buffer,BUFFER_SIZE);
      
      if(strcmp(buffer,"logout")==0)
        islogout = true;

      if(strcmp(buffer,"download_file") == 0)
      {
      	download_file();
      	continue;
      }
      n = send(tracker_fd, buffer, strlen(buffer),0);
      if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
      }
     // cout<<n<<" bytes sent success fully "<<endl;
      bzero(buffer,BUFFER_SIZE);
      n = recv(tracker_fd, buffer, BUFFER_SIZE, 0);
      if(n<0)
      {
        perror("Error Reading from tracker socket");
        exit(1);
      }

      cout<<buffer<<":"<<endl;
      if(islogout)
      	exit(1);   
   }

   pthread_join(thread_id, NULL);
   return 0;
}