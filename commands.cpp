#include "tracker_header"
#include <openssl/sha.h>

extern unordered_map<int, struct sockaddr_in> fdsockmap;
unordered_map <string, User> usermap;
unordered_map <string, Group> groupmap;
string current_user;

//this is for testing only
string printdummyHash(string filename)
{
	FILE *fp = fopen(filename.c_str(), "rb");	
	int n, no_chunks=0;
	string filehash = "";
	char chunk_buffer[CHUNK_SIZE];

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
  	}
  	return filehash;
}

void init()
{

	User u1;
	u1.username = "c1";
	u1.password = "123";
	u1.groups.push_back("g1");
	u1.isOnline = false;

	User u2;
	u2.username = "c2";
	u2.password = "123";
	u2.isOnline = false;

	User u3;
	u3.username = "c3";
	u3.password = "123";
	u3.isOnline = false;


	User u4;
	u4.username = "c4";
	u4.password = "123";
	u4.isOnline = false;

	usermap[u1.username] = u1;
	usermap[u2.username] = u2;
	usermap[u3.username] = u3;
	usermap[u4.username] = u4;
	
	Group g1;
	g1.gid = "g1";
	g1.owner = "c1";
	g1.members.push_back("c1");
	g1.members.push_back("c2");
	g1.members.push_back("c3");
	g1.members.push_back("c4");
	// g1.filehash_map["img.jpg"].first = printdummyHash("img.jpg");
	// g1.hashuser_map[printdummyHash("img.jpg")].push_back("c2");
	// g1.hashuser_map[printdummyHash("img.jpg")].push_back("c3");
	groupmap[g1.gid] = g1;

	printUsers();
	printGroups();
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

void createUser(vector<string> tokens, int fd)
{
	if(tokens.size() != 3)
	{
		throw string("Invalid arguments");
	}

	string username = tokens[1];
	string password = tokens[2];

	User usr;
	usr.username = username;
	usr.password = password;
	usr.address = fdsockmap[fd];
	usr.isOnline = false;

	if(usermap.find(username) == usermap.end())
	{
		usermap[username] = usr;
	}
	else
	{
		throw string("Username already exit");
	}

	return;
}

void authenticateUser(vector<string> tokens, int fd)
{
	if(tokens.size() != 4)
	{
		throw string("Invalid arguments");
	}

	string username = tokens[1];
	string password = tokens[2];
	int listenerport = stoi(tokens[3]);
	
	if(usermap.find(username) == usermap.end())
	{
		throw string("User does not exist");
	}
	else
	{
		User user = usermap[username];
		if(user.password != password)
			throw string("Invalid Credential");
		current_user = username;
		user.address = fdsockmap[fd];
		user.isOnline = true;
		user.listenerport = listenerport;
		usermap[username] = user;
	}

}

string listRequests(vector<string> tokens)
{
	string list_req = "";
	if(tokens.size() != 2)
	{
		throw string("Invalid arguments.");
	}

	if(current_user == "")
	{

		throw string("You need to login first to execute this command");
	}

	User c_user = usermap[current_user];
	string gid = tokens[1];

	cout<<current_user<<"is current user ";
	if(!c_user.containsGroup(gid))
	{
		throw string("Invalid group id");
	}

    Group grp = groupmap[gid];
	for(auto req : grp.requests)
	{
		cout << req << endl;
		list_req += req + "\n";
	}
	if(list_req=="")
		list_req = "No group requests found";

	return list_req;

}

void acceptRequests(vector<string> tokens)
{

	if(tokens.size() != 3)
	    throw string("Invalid arguments.");
	
	string gid = tokens[1];
	string uid = tokens[2];
	int i;
	Group group = groupmap[gid];
	//remove(group.requests.begin(), group.requests.end(), uid);
	for(i=0;i<group.requests.size();i++)
	{
		if(uid == group.requests[i])
			break;
	}
	if(i < group.requests.size())
	{
		group.requests.erase(group.requests.begin() + i);
		group.members.push_back(uid);
		groupmap[gid] = group;
	}
	else
	{
		throw string("Invalid userid");
	}
}

void createGroup(vector<string> tokens)
{
	if(tokens.size() != 2)
	{
		throw string("Invalid arguments.");
	}

	if(current_user == "")
	{
		throw string("You need to login first to execute this command");
	}

	string gid = tokens[1];

	Group grp;
	grp.gid = gid;
	grp.owner = current_user;
	grp.members.push_back(current_user);

	if(groupmap.find(gid)==groupmap.end())
	{
		groupmap[gid] = grp;
	}
	else
	{
		throw string("Group already exist");
	}
}

void joinGroup(vector<string> tokens)
{
	if(tokens.size() != 2)
	{
		throw string("Invalid arguments.");
	}

	if(current_user == "")
	{
		throw string("You need to login first to execute this command");
	}

	string gid = tokens[1];

	if(groupmap.find(gid)==groupmap.end())
	{
		throw string("Group does not exist");
	}

	Group grp = groupmap[gid];

	for(auto mem : grp.members)
	{
		if(mem == current_user)
			throw string("You are already group member");
	}

	grp.requests.push_back(current_user);
	groupmap[gid] = grp;
}

void logout()
{
	User usr = usermap[current_user];
	usr.isOnline = false;
	usermap[usr.username] = usr;
	current_user = "";
}

string listGroups()
{

	if(current_user == "")
	{
		throw string("You need to login first to execute this command");
	}

	string list_groups="";
	
	for(auto grp : groupmap)
	{
		//cout<<grp.first<<endl;
		list_groups += grp.first+"\n";
	}
	return list_groups;
}	

void printUsers()
{
	cout<<"User details are "<<endl;
	for(auto usr : usermap)
	{
		cout<<usr.first<<" : "<<usr.second.password<<" : ";
		for(auto grp : usr.second.groups)
		{
			cout<<grp<<" ";
		}
		cout <<" : "<< inet_ntoa(usr.second.address.sin_addr) <<"  "<< ntohs(usr.second.address.sin_port) <<" : ";
		usr.second.isOnline == true ? cout << "True" : cout << "False";
		cout<<endl;
	}
	cout<<endl;
}

void printGroups()
{
	cout<<"Groupt details are "<<endl;
	for(auto grp : groupmap)
	{
		cout<<grp.first<<" : "<<grp.second.owner<<" : ";
		
		cout<<endl<<"member : ";
		for(auto mem : grp.second.members)
			cout<<mem<<" ";

		cout<<endl<<"file hash pair "<<endl;
		for(auto filehash_map : grp.second.filehash_map)
		{
			cout<<" : "<<filehash_map.first<<" = ";
			cout<<"file size is "<<filehash_map.second.second<<" ";
			printHash(filehash_map.second.first.c_str());
		}

		cout<<endl<<"hash members pair "<<endl;
		for(auto hashuser_map : grp.second.hashuser_map)
		{
			cout<<" : ";
			printHash(hashuser_map.first);
			cout<<" = ";
			for(auto user : hashuser_map.second)
				cout<<" "<<user;
			cout<<endl;
		}

	}
	cout<<endl;
}

inline void printHash(string hash)
{
  	for(int i=0;i<hash.length();i++)
  		printf("%0.2x",(unsigned char )hash[i]);
}



string getCurrentUser(struct sockaddr_in client_address)
{
	for(auto x : usermap)
	{
		User user = x.second;
		if(user.address.sin_addr.s_addr == client_address.sin_addr.s_addr && user.address.sin_port == client_address.sin_port && user.isOnline)
		{
			return user.username;
		}
	}
	return string("Annonymous");
}

void uploadFile(vector<string> tokens, string command)
{
	
	if(tokens.size() < 4)
		throw string("Invalid arguments.");

	if(current_user == "")
		throw string("You need to login first to execute this command");
	
	string filename = tokens[1];
	string gid = tokens[2];
	string filesize = tokens[3];
	string hash = command.substr(command.find("hashkey") + 7);

	if(groupmap.find(gid) == groupmap.end()) 
		throw string("Invalid grouop id");

	Group group = groupmap[gid];

	if(group.containsUser(current_user) == false)
		throw string("You are not a memeber of this group. Join Group first to upload file");

	group.filehash_map[filename].first = hash;
	group.filehash_map[filename].second = stoll(filesize);
	group.hashuser_map[hash].push_back(current_user);

	groupmap[gid] = group;
}

string downloadFile(vector<string> tokens)
{
	if(current_user == "")
		throw string("You need to login first to execute this command");

	if(tokens.size() != 4)
		throw string("Invalid arguments.");

	
	string gid = tokens[1];
	string filename = tokens[2];


	if(groupmap.find(gid) == groupmap.end())
		throw string("Invalid group id");

	Group group = groupmap[gid];

	if(group.filehash_map.find(filename) == group.filehash_map.end())
		throw string("Invalid file name");

	string hash = group.filehash_map[filename].first;
	long long int  filesize = group.filehash_map[filename].second;

	vector<string> users = group.hashuser_map[hash];

	string clients = "file " + to_string(filesize);


	for(auto uid : users)
	{
		User usr = usermap[uid];
		if(usr.isOnline)
		{
			struct sockaddr_in sa = usr.address;
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN);
			clients += " " +string(str) + ":" + to_string(usr.listenerport);
		}
	}
	clients += " hashkey" + hash;
	return clients;
}

string executeCommand(string command, int client_fd)
{
	//printUsers();
	string success_msg="command executed";
//	cout<<"current user "<<current_user<<endl;
	struct sockaddr_in client_address = fdsockmap[client_fd];
	current_user = getCurrentUser(client_address);
	cout << "client "<<current_user <<" : "<<inet_ntoa(client_address.sin_addr) <<" : "<< ntohs(client_address.sin_port) <<endl;
	if(current_user== "Annonymous")
	{
		current_user = "";
	}
	cout<<command;
	try
	{
		vector<string> tokens = commandTokenize(command);
		
		string command_name = tokens[0];
		if(command_name == "create_user")
		{
			createUser(tokens, client_fd);
			printUsers();
			success_msg = "User account created successfully";		
		}
		else if(command_name == "login")
		{
			cout<<tokens.size()<<"is tokens szie ";
			authenticateUser(tokens, client_fd);
			success_msg = "Logged in successfully";		
		}
		else if(command_name ==  "create_group")
		{
			createGroup(tokens);
			printGroups();
			success_msg = "Group created successfully";
		}
		else if(command_name == "list_groups")
		{
			success_msg = listGroups();
			printGroups();
		}
		else if(command_name == "join_group")
		{
			joinGroup(tokens);
			success_msg = "Request sent successfully";
		}
		else if(command_name == "list_requests")
		{
			success_msg = listRequests(tokens);
		}
		else if(command_name == "accept_request")
		{
			acceptRequests(tokens);
			success_msg = "Requests accepted successfully";
		}
		else if(command_name == "upload_file")
		{
			uploadFile(tokens, command);
			printGroups();
			success_msg = "File uploaded successfully";
		}
		else if(command_name == "logout")
		{
			logout();
			success_msg = "Logged out successfullly";
		}
		else if(command_name == "download_file")
		{
			success_msg = downloadFile(tokens);
		}
		else
		{
			success_msg = "Invalid command";
		}
	}catch(string error_msg)
	{
		throw error_msg;
	}
	return success_msg;
}