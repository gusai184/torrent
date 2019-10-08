#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 256
using namespace std;

class Group;

class User
{
	public:
		string username;
		string password;
		vector<string> groups;
		struct sockaddr address; 
};

class Group
{
	public:
		string gid;
		string owner;
		vector<string> members;	//member user ids
		vector<string> requests; //peding joing requests
};



string executeCommand(string command);

vector<string> commandTokenize(string command);

void createUser(vector<string> tokens);

void createGroup(vector<string> tokens);

void joinGroup(vector<string> tokens);

void listRequests(vector<string> tokens);

void logout();

void listGroups();