#include "client_header"

unordered_map<string, vector<int>> hashchunks_map;
unordered_map<string, string> filehash_map;

void * downloadingthread(void * args_passed)
{
	cout<<"INSIDE downloading  ---  thread"<<endl;
	struct args_struct args = * ((struct args_struct *) args_passed);
	
	int listener_port, n, listener_fd, last_chunk, last_chunk_size;
	string filename;
	vector<int> chunks;
	struct sockaddr_in listener_address;
	struct hostent *listener_ip;
	char buffer[BUFFER_SIZE];
  cout<<"Establishing connection to ";
	cout<<" ip  "<<args.ip;
	cout<<" port "<<args.port<<endl;
	listener_port = args.port;
	filename = args.filename;
	listener_ip = gethostbyname(args.ip.c_str());
 	chunks = args.chunks;
  last_chunk = args.last_chunk;
  last_chunk_size = args.last_chunk_size;
	//cout<<"chucnks size "<<chunks.size();
	// for(auto c : chunks)
	// 	cout<<c<<endl;
	//cout<<"before listenr ip "<<endl;
    listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd < 0) {
    	
    	perror("ERROR opening socket");
        exit(1);
    }
	
    //cout<<"listenre port is "<<listener_ip<<endl;
    if (listener_ip == NULL) {
    	cout<<"ERROR no "<<errno<<endl;	
    	perror("ERROR, no such host\n");
        //fprintf(stderr,"ERROR, no such host\n");
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
       
    string file_chunks = filename;
    for(auto chunk_no : chunks)
    	file_chunks = file_chunks+" "+ to_string(chunk_no);
    
    strcpy(buffer, file_chunks.c_str());
    cout<<file_chunks<<endl;
    
    n = send(listener_fd, buffer, BUFFER_SIZE, 0);
    if(n<0)
    {
      perror("Error while sending data");
      return NULL; 
    }
	  readFile(listener_fd, filename, chunks,last_chunk, last_chunk_size);   
 	  return NULL;
}

void readFile(int peer_fd, string filename, vector<int> chunks, int last_chunk, int last_chunk_size)
{
   
   FILE *fp = fopen((filename).c_str(),"rb+");
   if(fp == NULL)
   {
      cout<<filename<<endl;
      perror("Unable to open file");
      return;
   }

   for(int i=0;i<chunks.size();i++)
    {
     	receiveChunk(fp ,peer_fd, chunks[i], last_chunk, last_chunk_size);	     	
	  	//string ack ="Ack" + to_string(chunks[i]);
		  //send(peer_fd, ack.c_str(), ack.length(), 0);
    }
   
   fclose(fp);
}

void receiveChunk (FILE * fp ,int peer_fd, int chunkno, int last_chunk, int last_chunk_size)
{
	char chunk_buffer[CHUNK_SIZE];
	memset(chunk_buffer, '\0', CHUNK_SIZE);
	int start_address = chunkno * CHUNK_SIZE, n, size;
	
	n = fseek(fp, start_address, SEEK_SET);
	if(n < 0)
	{
		perror("Invalid reading pointer while reading chunkno " + chunkno);
		return;
	}

    if(chunkno==last_chunk)
    	size = last_chunk_size;
    else
    	size = CHUNK_SIZE;
	
	int len = 0;
	while(len < size)
	{
    	if(chunkno==last_chunk)
    		n = recv(peer_fd, &chunk_buffer, last_chunk_size, 0);
    	else	
		  	n = recv(peer_fd, &chunk_buffer, CHUNK_SIZE, 0);
	    
	    if(n < 0)
  	 	{
  			perror("Error while receiving chunk no " + chunkno);
  			return;
  		}

  		int n1 = fwrite(chunk_buffer, sizeof(char), n, fp);
  		if(n1 < 0)
  		{
  			perror("Error while writing chunk to file " + chunkno);
  			return;
  		}
  		len = len + n;
	}
	printSHA((unsigned char *)chunk_buffer, size, chunkno);
}
	
void getPeers(string buffer, vector<Peer> &peers1)
{
  buffer = buffer.substr(0, buffer.find("hashkey"));
  //cout<<"Buffer is "<<buffer<<endl;
  vector<string> tokens = commandTokenize(buffer);
  for(int i=2;i<tokens.size()-1;i++)
  {
    string ip = tokens[i].substr(0,tokens[i].find(":"));
    string port = tokens[i].substr(tokens[i].find(":")+1);
    //cout<<"ip is "<<ip<<" port is "<<port<<endl;
    Peer peer;
    peer.ip = ip;
    peer.port = stoi(port);
    peers1.push_back(peer);
  }

}

vector<int> getChunksFromPeer(string ip, int port,string hash)
{
  int listener_fd, n;
  struct sockaddr_in listener_address;
  struct hostent *listener_ip;
  char buffer[BUFFER_SIZE];
  vector<int> chunks;

  cout<<"Getting Chunk details from Ip "<<ip<<" "<<" Port "<<port<<endl;

  int listener_port = port;
  listener_ip = gethostbyname(ip.c_str());
  listener_fd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (listener_fd < 0) {
    perror("ERROR opening socket");
    return chunks;
  }

  if (listener_ip == NULL) {
    cout<<"ERROR no "<<errno<<endl; 
    perror("ERROR, no such host\n");
    return chunks;
  }

  bzero((char *) &listener_address, sizeof(listener_address));
  listener_address.sin_family = AF_INET;
  bcopy((char *)listener_ip->h_addr, (char *)&listener_address.sin_addr.s_addr, listener_ip->h_length);
  listener_address.sin_port = htons(listener_port);
  
  if (connect(listener_fd, (struct sockaddr*)&listener_address, sizeof(listener_address)) < 0) {
      cout<<"error no is "<<errno<<endl;
      perror("ERROR connecting");
      return chunks;
  }
  hash = "file"+hash;
  n = send(listener_fd, hash.c_str(), BUFFER_SIZE, 0);
  if(n < 0)
  {
    perror("Error while sending data");
    return chunks;
  }
  
  bzero(buffer, BUFFER_SIZE);
  n =recv(listener_fd, buffer, BUFFER_SIZE, 0);
  if(n < 0)
  {
    perror("Error while receiving data");
    return chunks;
  }

  //cout << "Got Chunks "<<buffer<<endl;
  vector<string> chunks_stg = commandTokenize(buffer);
      
  try{
    for(int i=0;i<chunks_stg.size();i++)
    {
       //cout<<":"<<chunks_stg[i]<<":";
       if(chunks_stg[i]!="" && chunks_stg[i]!= " ")
         chunks.push_back(stoi(chunks_stg[i]));
    }
  }
  catch(...)
  {
    cout<<"Peer do no have file"<<endl;
    throw string("Peer no have file");
  }

  close(listener_fd);
  return chunks;
}


void download_file(string filename, string gid, string buffer, int tracker_fd)
{
  try{
  vector<string> tokens = commandTokenize(buffer);
  if(tokens[0] != "file")
	{
		cout<<buffer;
		return;
	}

	ll filesize = stoll(tokens[1]);
	string hash = buffer.substr(buffer.find("hashkey") + 7);
  ll no_chunks = ceil((float)filesize / CHUNK_SIZE);
  vector<Peer> peer1;

  cout<<"no of chunks are "<<no_chunks<<endl;
  getPeers(buffer, peer1);
  int no_peers = peer1.size();
  vector<Peer> peer2(no_peers);
  vector<pair<int, vector<int>>> chunkset(no_chunks);

  cout<<"peers are "<<endl;
  for(int i=0;i<peer1.size();i++)
  {
    cout<<peer1[i].ip<<" "<<peer1[i].port<<endl;
    peer2[i].ip = peer1[i].ip;
    peer2[i].port = peer1[i].port;
  }

  for(int i=0;i<peer1.size();i++)
  {
     peer1[i].chunks = getChunksFromPeer(peer1[i].ip, peer1[i].port, filename);
  }
  no_peers = peer1.size();
  for(int i=0;i<no_chunks;i++)
  {
    vector<int> v;
    chunkset[i] = {i,v};
  }

  //cout<<"peer details is "<<endl;
  for(int i=0;i<peer1.size();i++)
  {
     //cout<<peer1[i].ip<<" : "<<peer1[i].port<<" : ";
     for(auto c : peer1[i].chunks)
      {
        chunkset[c].second.push_back(i);
        //cout<<c<<" ";
      }
  }
  //cout<<"chunkset is "<<endl;
  // for(int i=0;i<chunkset.size();i++)
  // {
  //   //cout<<chunkset[i].first<<" : ";
  //   for(auto p : chunkset[i].second)
  //     cout<<p<<" ";
  //   cout<<endl;
  // }

  //sort(chunkset.begin(), chunkset.end());

  // cout<<"After sorting chunkset is "<<endl;
  // for(int i=0;i<chunkset.size();i++)
  // {
  //  // cout<<chunkset[i].first<<" : ";
  //   for(auto p : chunkset[i].second)
  //     cout<<p<<" ";
  //   cout<<endl;
  // }

  //Modified Rarest piece selection algorithm
  //pic piece from rear to most occurs and select peer randomely
  for(int i=0;i<chunkset.size();i++)
  {
    int chunk_no = chunkset[i].first;
    vector<int> chunks = chunkset[i].second;
    int vect_size = chunks.size();
    int peer = chunks[random() % vect_size];
    peer2[peer].chunks.push_back(chunk_no);
  }
	
  cout<<"algo output peer details is "<<endl;
  for(int i=0;i<peer2.size();i++)
  {
     cout<<peer2[i].ip<<" : "<<peer2[i].port<<" : ";
     for(auto c : peer2[i].chunks)
      {
        chunkset[c].second.push_back(i);
        cout<<c<<" ";
      }
      cout<<endl;
  }
  
	//parameter will be list of client and file name

	//will create thread for each cilent and ask them what chunks they have 

	//will decide what to get from each client and processed as follows : thread for client ip:port and chuncks to be downloaded

  	
  	int listener_port, listener_port1;
  	vector<int> chunks;
  	listener_port = 7000;

  	FILE *fp = fopen((filename).c_str(),"wb+");
  	int i=0;
  	while(i<filesize)
  	{
  		fputc('0',fp);
  		i++;
  	}
    fclose(fp);

  	int last_chunk = no_chunks - 1;
  	int last_chunk_size = filesize - (filesize/CHUNK_SIZE) * CHUNK_SIZE;

   // cout<<"no of chunks "<<no_chunks<<endl;
  	//cout<<"Last chunk "<<last_chunk<<" "<<last_chunk_size<<endl;


  	struct args_struct *args = new struct args_struct[no_peers];
  	pthread_t *thread_id = new pthread_t[no_peers];

  	for(int i=0;i<no_peers;i++)
  	{
	
  		args[i].port = peer2[i].port;
  		args[i].ip = peer2[i].ip;
  		args[i].chunks = peer2[i].chunks;
  		args[i].filename = filename;
  	  args[i].last_chunk = last_chunk;
	    args[i].last_chunk_size = last_chunk_size;
	    
	    pthread_create(&thread_id[i], NULL, downloadingthread, (void *)&args[i]); 
	  }
    
   
     cout<<"File downloaded successfully"<<endl;
      
      char buffer1[BUFFER_SIZE];
      //send tracker acknowledngemt of downloaded and update tracker file hash     
      string ack = "upload_file "+filename+" "+gid+" "+to_string(filesize);
      strcpy(buffer1,(ack + " hashkey" + hash).c_str());
      int n = send(tracker_fd, buffer1, strlen(buffer1),0);  
      if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
      }
     
      bzero(buffer1,BUFFER_SIZE);
      n = recv(tracker_fd, buffer1, BUFFER_SIZE, 0);
      if(n<0)
      {
        perror("Error Reading from tracker socket");
        exit(1);
      }

      //upload local details
      for(int i=0;i<no_chunks;i++)
        chunks.push_back(i);
      
      hashchunks_map[hash] = chunks;
      filehash_map[filename] = hash;

      for(int i=0;i<no_peers;i++) 
        pthread_join(thread_id[i], NULL);
 
    
  }
  catch(...)
  {
    perror("Error while downloading ");
  }

}

int main(int argc, char *argv[]) {
   int tracker_fd, tracker_port, listener_port, n;
   struct sockaddr_in tracker_address;
   struct hostent *tracker_ip;
   string filehash;
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

   n = recv(tracker_fd, buffer, BUFFER_SIZE, 0);
  
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   cout<<"Msg from tracker_ip "<<buffer<<":"<<endl;
 
   while(1)
   {
      cout<<"Enter commands "<<endl;
      bzero(buffer,BUFFER_SIZE);
      //fgets(buffer,255,stdin);
      cin.getline(buffer,BUFFER_SIZE);
      
      vector<string> tokens  = commandTokenize(buffer);

      if(strcmp(buffer,"logout")==0)
      {
        islogout = true;
      }
      else if(tokens[0] == "login")
      {
      	//passing listening port to tracker along with login command
      	 strcpy(buffer,(string(buffer) + " " + to_string(listener_port)).c_str());
      }
      else if(tokens[0] == "upload_file")
      {
      	if(tokens.size() != 3)
      	{
      		cout<<"Invalid arguments"<<endl;
      		continue;
      	}
      	filehash = calcuteHash(tokens[1]);
      	if(filehash == "error")
      		continue;
      	
        FILE* fp1 = fopen(tokens[1].c_str(), "r"); 
  	    if (fp1 == NULL) { 
  	        printf("File Not Found!\n"); 
  	        return 1; 
  	    } 
  	    fseek(fp1, 0L, SEEK_END); 
  	    long int filesize = ftell(fp1); 
  	    fclose(fp1); 

      	//passing hash to tracker along with command
      	strcpy(buffer,(string(buffer) + " "+ to_string(filesize) + " hashkey" + filehash).c_str());
      	//cout<<"buffer is "<<buffer<<endl;
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

      if(tokens.size() == 4 && tokens[0] == "download_file")
        	download_file(tokens[2],tokens[1],buffer,tracker_fd);
 	    else
 	  	    cout<<buffer<<":"<<endl; 

      // if(islogout)
      // 	exit(1);   
  }

   pthread_join(thread_id, NULL);
   return 0;
}

string calcuteHash(string filename)
{
	char chunk_buffer[CHUNK_SIZE];
	FILE *fp = fopen(filename.c_str(), "rb");	
	int n, no_chunks=0;
	string filehash = "";

	if(fp == NULL)
	{
		perror("file not found");
		return "error";	
	}

	while((n = fread(chunk_buffer, sizeof(char), CHUNK_SIZE, fp)) > 0)
	{			
		unsigned char hash[SHA_DIGEST_LENGTH]; 
  		SHA1((const unsigned char *)chunk_buffer, n - 1, hash);
  		//for(int i=0;i<SHA_DIGEST_LENGTH;i++)
	  	//	printf("%0.2x",hash[i]);
	  	//cout<<endl;
	  	filehash += (char *)hash;
	  	no_chunks++;
  	}

  	// for(int i=0;i<filehash.length();i++)
  	// 	printf("%0.2x",(unsigned char )filehash[i]);

  	vector<int> chunks;

  	for(int i=0;i<no_chunks;i++)
  		chunks.push_back(i);
    
    hashchunks_map[filehash] = chunks;
  	filehash_map[filename] = filehash;
  	fclose(fp);
	
	  return filehash;
}


