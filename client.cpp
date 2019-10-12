#include "client_header"

void * downloadingthread(void * args_passed)
{
	cout<<"INSIDE downloadingthread"<<endl;
	
	struct args_struct args = * ((struct args_struct *) args_passed);
	
	int listener_port, n, listener_fd, last_chunk, last_chunk_size;
	string filename;
	vector<int> chunks;
	struct sockaddr_in listener_address;
	struct hostent *listener_ip;
	char buffer[BUFFER_SIZE];

	listener_port = args.port;
	filename = args.filename;
	listener_ip = gethostbyname(args.ip.c_str());
	chunks = args.chunks;
  last_chunk = args.last_chunk;
  last_chunk_size = args.last_chunk_size;
	cout<<"chucnks size "<<chunks.size();
	// for(auto c : chunks)
	// 	cout<<c<<endl;

    listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) {
    	
    	perror("ERROR opening socket");
        exit(1);
    }
	
    
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

    string file_chunks = filename;
    for(auto chunk_no : chunks)
    	file_chunks = file_chunks+" "+ to_string(chunk_no);
    
    cout<<file_chunks<<endl;
    
    send(listener_fd, file_chunks.c_str(), file_chunks.length(), 0);
	
	 readFile(listener_fd, filename, chunks,last_chunk, last_chunk_size);   
 	
 	return NULL;
}

void readFile(int peer_fd, string filename, vector<int> chunks, int last_chunk, int last_chunk_size)
{
   
   FILE *fp = fopen(("downloaded_" + filename).c_str(),"r+");

   for(int i=0;i<chunks.size();i++)
    {
     	receiveChunk(fp ,peer_fd, chunks[i], last_chunk, last_chunk_size);	     	
	  	string ack ="Ack" + to_string(chunks[i]);
		  send(peer_fd, ack.c_str(), ack.length(), 0);
    }
   
   fclose(fp);
}

void receiveChunk (FILE * fp ,int peer_fd, int chunkno, int last_chunk, int last_chunk_size)
{
	char chunk_buffer[CHUNK_SIZE];
	memset(chunk_buffer, '\0', CHUNK_SIZE);
	int start_address = chunkno * CHUNK_SIZE, n;
	
	n = fseek(fp, start_address, SEEK_SET);
	if(n < 0)
	{
		perror("Invalid reading pointer while reading chunkno " + chunkno);
		return;
	}

  if(chunkno==last_chunk)
    n = recv(peer_fd, &chunk_buffer, last_chunk_size, 0);
  else
    n = recv(peer_fd, &chunk_buffer, CHUNK_SIZE, 0);
	if(n < 0)
	{
		perror("Error while receiving chunk no " + chunkno);
		return;
	}
	
	cout<<"n is "<<n<<endl;
	cout<<"Received Chunk "<<chunkno<<" content"<<chunk_buffer<<endl;
	//cout<<"Received Chunk "<<chunkno<<endl;

	n = fwrite(chunk_buffer, sizeof(char), n, fp);
	if(n < 0)
	{
		perror("Error while writing chunk to file " + chunkno);
		return;
	}

}

void download_file()
{

	//parameter will be list of client and file name

	//will create thread for each cilent and ask them what chunks they have 

	//will decide what to get from each client and processed as follows : thread for client ip:port and chuncks to be downloaded

	
	int listener_port, filesize = 63;
	string filename = "abc.txt";
	vector<int> chunks, chunks1;
	// cout<<"Enter client 1 port where to download_file"<<endl;
	// cin>>listener_port;
	// cin.ignore();  
  listener_port = 8001;

	 FILE *fp = fopen(("downloaded_" + filename).c_str(),"wb");
     //memset(fp, 0, filesize);
     fclose(fp);

  int no_chunks = ceil((float)filesize / CHUNK_SIZE);
  int last_chunk = no_chunks - 1;
  int last_chunk_size = filesize - (filesize/CHUNK_SIZE) * CHUNK_SIZE;


  cout<<"no of chunks "<<no_chunks<<endl;
  cout<<"Last chunk "<<last_chunk<<" "<<last_chunk_size<<endl;
  //  for(int i=0;i<no_chunks;i++)
		// chunks.push_back(i);
	
  // for(int i=no_chunks-1;i>=0;i--)
  //    chunks.push_back(i);
  
   chunks.push_back(6);
  // chunks.push_back(5);
  // chunks.push_back(4);
  chunks.push_back(3);
  chunks.push_back(0);
  chunks.push_back(2);
   // chunks.push_back(1);

	struct args_struct args, args1;
	args.port = listener_port;
	args.ip = "127.0.0.1";
	args.chunks = chunks;
	args.filename = filename;
  args.last_chunk = last_chunk;
  args.last_chunk_size = last_chunk_size;

  pthread_t thread_id; 
  pthread_create(&thread_id, NULL, downloadingthread, (void *)&args); 
	
    int listener_port1;
	// cout<<"Enter client 2 port where to download_file"<<endl;
	// cin>>listener_port1;
	// cin.ignore();  
  listener_port1 = 8002;
	chunks1.push_back(1);
	chunks1.push_back(5);
	chunks1.push_back(4);

	
	args1.port = listener_port1;
	args1.ip = "127.0.0.1";
	args1.chunks = chunks1;
	args1.filename = filename;

	pthread_t thread_id1; 
  pthread_create(&thread_id1, NULL, downloadingthread, (void *)&args1); 


	pthread_join(thread_id, NULL);
	pthread_join(thread_id1, NULL);

}

int main(int argc, char *argv[]) {
   int tracker_fd, tracker_port, listener_port, n;
   struct sockaddr_in tracker_address;
   struct hostent *tracker_ip;
   
   char buffer[BUFFER_SIZE];
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
   ///-----------------------
    
 //-----------------------

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