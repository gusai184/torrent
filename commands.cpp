#include "header.h"


unordered_map <string, User> usermap;
unordered_map <string, Group> groupmap;
string current_user;

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

void createUser(vector<string> tokens)
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

void authenticateUser(vector<string> tokens)
{
	if(tokens.size() != 3)
	{
		throw string("Invalid arguments");
	}

	string username = tokens[1];
	string password = tokens[2];
	
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
	}

}

void listRequests(vector<string> tokens)
{
// 	if(tokens.size() != 2)
// 	{
// 		throw string("Invalid arguments.");
// 	}

// 	if(current_user == "")
// 	{

// 		throw string("You need to login first to execute this command");
// 	}

// 	User c_user = usermap[current_user];
	
// 	for(auto grp : c_user.groups)
// 	{
// 		if(grp.gid == )
// 	}


// 	for(auto req : grp.requests)
// 	{
		
// 	}

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

}




void logout()
{
	current_user = "";
}

void listGroups()
{
	for(auto grp : groupmap)
	{
		cout<<grp.first<<endl;
	}
}

void printUsers()
{
	for(auto usr : usermap)
	{
		cout<<usr.first<<" : "<<usr.second.password<<" : ";
		for(auto grp : usr.second.groups)
		{
			cout<<grp<<" ";
		}
	}
	cout<<endl;
}

void printGroups()
{
	for(auto grp : groupmap)
	{
		cout<<grp.first<<" : "<<grp.second.owner<<" ";
		for(auto mem : grp.second.members)
		{
			cout<<mem<<" ";
		}
	}
	cout<<endl;
}

string executeCommand(string command)
{
	string success_msg;

	try
	{

		vector<string> tokens = commandTokenize(command);
		
		string command_name = tokens[0];
		if(command_name == "create_user")
		{
			createUser(tokens);
			printUsers();
			success_msg = "User account created successfullly";
		
		}
		else if(command_name == "login")
		{
			
			authenticateUser(tokens);
			success_msg = "Logged in successfullly";
			
		}
		else if(command_name ==  "create_group")
		{
			createGroup(tokens);
			printGroups();
			success_msg = "Group created successfullly";
		}
		else if(command_name == "list_groups")
		{
			listGroups();
		}
		else if(command_name == "join_group")
		{
			joinGroup(tokens);
		}
		else if(command_name == "list_requests")
		{
			listRequests(tokens);
		}
		else if(command_name == "logout")
		{
			logout();
			success_msg = "Logged out successfullly";
		}
		else if(command_name == "exit")
		{
			exit(1);
		}
		else
		{
			success_msg = "Invalid Command";
		}
	}catch(string error_msg)
	{
		throw error_msg;
	}
	return success_msg;
}