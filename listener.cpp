#include "client_header"

extern unordered_map<string, vector<int>> hashchunks_map;
extern unordered_map<string, string> hashfile_map;

inline void printHash(string hash)
{
  	for(int i=0;i<hash.length();i++)
  		printf("%0.2x",(unsigned char )hash[i]);
}

void printSHA(const unsigned char buffer[], int n, int chunk_no)
{
	cout<<chunk_no<<" : "<<n<<" : ";
	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
  	SHA1(buffer, n - 1, hash);
  	for(int i=0;i<SHA_DIGEST_LENGTH;i++)
  		//cout<<hex<<buffer[i];
  		printf("%0.2x",hash[i]);
  	cout<<endl;
}

void *serverthread(void *portnoadd)
{
   int listener_port = *(int *)portnoadd;
   cout<<"listing at listener_port "<<listener_port<<endl;
   int listener_fd,   peerlen, n;
   struct sockaddr_in listener_address, peer_address;

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
   
   while(1)
   {
      int *peer_fd_ptr= (int *)malloc(sizeof(int));
      *peer_fd_ptr = accept(listener_fd, (struct sockaddr *)&peer_address,(socklen_t *)&peerlen);

      if (*peer_fd_ptr < 0) {
         perror("ERROR on acceptingg peer request");
         continue;
      }
      cout<<"Connection established in listener "<<endl;
      pthread_t thread_id; 
      pthread_create(&thread_id, NULL, sendingthread, (void *)peer_fd_ptr); 
   }

}

void * sendingthread(void * peer_fd_ptr)
{
   int peer_fd = *(int *)peer_fd_ptr;
   char buffer[BUFFER_SIZE];
   string filename;
   vector<int> chunks;
   
    cout<<"listener thread Connection established to "<<endl;
   // bzero(buffer,BUFFER_SIZE);
   // strcpy(buffer, "Connection established to listener");
   // send(peer_fd, buffer, BUFFER_SIZE , 0);


   //Receive chunkst to be sent 
    bzero(buffer,BUFFER_SIZE);
    int n = recv(peer_fd, buffer, BUFFER_SIZE, 0);
    if(n < 0)
   	{
   	   	 perror("Error while sending");
   	   	 return NULL;
   	}
   	cout<<"unordered_map ";
   	for(auto x: hashchunks_map)
   	{
   		cout<<x.first<<" : "<<x.second.size()<<endl;
   	}
   string bufferhash(buffer), chunks_stg="";
   if(bufferhash.find("hash") != string::npos)
   {
   		string hash = bufferhash.substr(bufferhash.find("hash")+4);
   		cout<<"HASH REQUEST CAME :";//<<hash<<":";
  		printHash(hash);
   		if(hashchunks_map.find(hash)==hashchunks_map.end())
   		{
   			 chunks_stg = "ERROR : HASH NOT FOUND";
   			 cout<<"hash not found";
   		}
   		else
   		{
	   		vector<int> chunks = hashchunks_map[hash];
	   		for(int i=0;i<chunks.size();i++)
	   		{	   			
	   			chunks_stg = chunks_stg + to_string(chunks[i]) + " ";
	   		}
	   		
	   		//cout<<"chunks vector size is "<<chunks.size()<<endl;
   	   }
   	   //cout<<"chunk stg"<<chunks_stg<<"is chunks"<<endl;
   	   int n =send(peer_fd, chunks_stg.c_str(), chunks_stg.length(), 0);
   	   if(n < 0)
   	   {
   	   	 perror("Error while sending");
   	   	 return NULL;
   	   }
   }
   else
   {
	   string fd_chunks(buffer);
	   cout<<"File_chunk is "<<fd_chunks<<endl;
	   
	   vector<string> tokens = commandTokenize(fd_chunks);

	   filename = tokens[0];
	   
	   for(ll i=1;i<tokens.size();i++)
	         chunks.push_back(stoi(tokens[i]));

	   sendFile(peer_fd, filename, chunks);
	}

}

void sendFile(int peerfd, string filename, vector<int> chunks)
{
   char buffer[BUFFER_SIZE];
   FILE *fp = fopen(filename.c_str(),"rb+");
   if(fp == NULL)
   {
   		perror("Error while opening file");
   		return;
   }

   for(ll i=0;i<chunks.size();i++)
   {
   		sendChunk(fp, peerfd, chunks[i]); 
   		//recv(peerfd, buffer, BUFFER_SIZE, 0);
   		//cout<<endl<<buffer<<endl;
   }
   

   fclose(fp);

}

void sendChunk(FILE * fp ,int peer_fd, int chunkno)
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

    n  = fread(chunk_buffer, sizeof(char), CHUNK_SIZE, fp);
	if(n < 0)
	{
		string error_msg = "Error while reading" + to_string(chunkno) + " chunk from file ";
		perror(error_msg.c_str());
		return;
	}

	if(n==0)
	{
		return;
	}

	int n1 = send(peer_fd, chunk_buffer, n, 0);

	if(n1 < 0)
	{
		perror("Error while sending chunkno " + chunkno);
		return;
	}


	// int total=0;
	// int size = n;
	// while(total < size)
	// {
	// 	int n1 = send(peer_fd, chunk_buffer+total, size - total, 0);
	// 	if(n1 < 0)
	// 	{
	// 		perror("Error while sending chunkno " + chunkno);
	// 		return;
	// 	}
	// 	total = total + n1;
	// }

	printSHA((unsigned char *)chunk_buffer,	n1, chunkno);	
}



vector<string> commandTokenize(string command)
{
   vector<string> v;
   string word = ""; 
   for (auto x : command) 
   { 
       if (x == ' ') 
       { 
           v.push_back(word);
           word = ""; 
       } 
       else
       { 
           word = word + x; 
       } 
   }  

   v.push_back(word);

   return v;
}
