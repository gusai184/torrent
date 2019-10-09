#include <stdio.h>
#include <algorithm>
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
		bool isOnline;
		vector<string> groups;
		struct sockaddr_in address; 

	public:
		bool containsGroup(string gid)
		{			
			vector<string> groups = this->groups;
			int i;
			for(i = 0; i < groups.size(); i++)
			{
				if(groups[i] == gid)
					return true;
			}

			return false;
		}
};

class Group
{
	public:
		string gid;
		string owner;
		vector<string> members;	//member user ids
		vector<string> requests; //peding joing requests

	public:
		bool containsUser(string uid)
		{
			vector<string> users = this->members;
			int i;
			for(int i = 0; i < users.size(); i++)
			{
				if(users[i] == uid)
					return true;
			}
			return false;
		}
};

void init();

string executeCommand(string command, int fd);

vector<string> commandTokenize(string command);

void createUser(vector<string> tokens, int fd);

void createGroup(vector<string> tokens);

void joinGroup(vector<string> tokens);

string listRequests(vector<string> tokens);

void logout();

string listGroups();