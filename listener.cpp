#include "client_header"


void *serverthread(void *portnoadd)
{

   int listener_port = *(int *)portnoadd;
   cout<<"listing at listener_port "<<listener_port<<endl;
   int listener_fd, peer_fd,  peerlen, n;
   char buffer[BUFFER_SIZE];
   string filename;
   struct sockaddr_in listener_address, peer_address;
   vector<int> chunks;

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

   string fd_chunks(buffer);
   cout<<"File_chunk is "<<fd_chunks<<endl;

   vector<string> tokens = commandTokenize(fd_chunks);

   filename = tokens[0];
   
   for(ll i=1;i<tokens.size();i++)
   		chunks.push_back(stoi(tokens[i]));

   sendFile(peer_fd, filename, chunks);

}

void sendFile(int peerfd, string filename, vector<int> chunks)
{
   char buffer[BUFFER_SIZE];
   FILE *fp = fopen(filename.c_str(),"rb");
   if(fp == NULL)
   {
   		perror("Error while opening file");
   		return;
   }

   for(ll i=0;i<chunks.size();i++)
   {
   		sendChunk(fp, peerfd, chunks[i]); 
   		recv(peerfd, buffer, BUFFER_SIZE, 0);
   		cout<<endl<<buffer<<endl;
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

	n = fread(chunk_buffer, sizeof(char), CHUNK_SIZE, fp);
	if(n < 0)
	{
		string error_msg = "Error while reading" + to_string(chunkno) + " chunk from file ";
		perror(error_msg.c_str());
		return;
	}
	cout<<"n is "<<n<<endl;
	cout<<"Ready to send chunk "<<chunkno<<" : "<<chunk_buffer<<endl;
	//cout<<"Ready to send chunk "<<chunkno<<endl;
	if(n==0)
	{
		return;
	}
	n = send(peer_fd, chunk_buffer, n, 0);
	if(n < 0)
	{
		perror("Error while sending chunkno " + chunkno);
		return;
	}

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
