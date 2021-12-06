#include <fstream>
#include <stdlib.h>
#include <Windows.h>
#include <vector>
#include <sstream>

using namespace std;
string checkHumanFile = "CheckQueueHumanDB.txt";
HANDLE mutexFileUser = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexFileUser");
HANDLE mutexFileHuman = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexFileHuman");
HANDLE modHumEvent = OpenEventA(EVENT_ALL_ACCESS, "modHumEvent");

vector<string> msgInfo;

vector<string> Split(string line, string delimiter){
	size_t pos = 0;
	string token;
	vector<string> splitRes;
	while ((pos = line.find(delimiter)) != string::npos) { //parse on id, username and msg
		token = line.substr(0, pos);
		splitRes.push_back(token); 
		line.erase(0, pos + delimiter.length());
	}
	return splitRes;
}

string getUserName()
{
	return msgInfo[1];
}

inline void BanUser(string username)
{
	string line;
	ifstream file;
	//username|active(a)/banned(b)
    file.open(userDBFilePath);
	string output;
    while (getline(file, line))
    {
        line.replace(line.find(username), (username+ "|a").length(), username + "|b");//username|a 
		output += line + "\n";
    }
	file.close();
	ofstream res;
	res.open(userDBFilePath, std::ofstream::out | std::ofstream::trunc);
	res << output;
	res.close();
}

void DeleteLineFromFile(string fileName, string deleteLine){
    string line;
	ifstream file;
	WaitForSingleObject(mutexFileHuman, INFINITE);
    file.open(fileName);
	string text;
    while (getline(file, line))
    {
        line.replace(line.find(deleteLine), deleteLine.length(), "");
		text += line + "\n";
    }
	file.close();
	ofstream res;
	res.open(fileName, std::ofstream::out | std::ofstream::trunc);
	res << text;
	res.close();
	ReleaseMutex(mutexFileHuman);
}

string ReadInputLine()
{
	ifstream checkHumanFile;
	WaitForSingleObject(mutexFileHuman, INFINITE);
	checkHumanFile.open(checkQueueFilePath.c_str());
	string line;
	getline(checkHumanFile, line);
	checkHumanFile.close();
	ReleaseMutex(mutexFileHuman);
	DeleteLineFromFile(checkHumanFile, line);
	//id|username|msg
	msgInfo = Split(line, "|");
	return msgInfo[2];
}


int main(){
	string ai_text = "===================================================";
	cout << ai_text << "Human Moderator" << ai_text << endl;
	string username;
	string msg;
	while (1){
		username = getUserName();
		msg = ReadInputLine();
		cout << "This message contains uncommon symbols:" << endl;
		
		cout << username << ": " << msg << endl << endl;
		cout << "Ban user: Y | N";
		char ch;
		switch (ch){
			case 'y'{

			}
		}
	}
}