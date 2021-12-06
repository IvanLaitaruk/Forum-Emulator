#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <Windows.h>
#include <vector>
#include <sstream>

using namespace std;

HANDLE mutexFileUser = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexFileUser");
HANDLE mutexFileMsg = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexFileMsgAI");
HANDLE mutexFileHuman = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexFileHuman");
HANDLE mutexBadWordsDB = OpenMutexA(MUTEX_ALL_ACCESS, TRUE, "mutexBadWordsDB");
HANDLE modAIEvent = OpenEventA(EVENT_ALL_ACCESS, TRUE, "modAIEvent");
HANDLE modHumEvent = OpenEventA(EVENT_ALL_ACCESS, TRUE, "modHumEvent");

vector<string> msgInfo;
double maxBadWordPercentage = 10.0;
//full filepathes of DB files 
string badWordsBDFilePath = "badWordsDB.txt";
string checkQueueFilePath = "CheckQueueAIDB.txt";
string userDBFilePath = "UserDB.txt";
string checkHumanFilePath = "CheckQueueHumanDB.txt";
vector<string> badWordsDB;
double currentBadWordsPercentage;

bool GetBadWordsDB(){
	ifstream badWordsDBFile;
	badWordsDBFile.open(badWordsBDFilePath.c_str());
	if(!badWordsDBFile)
    {
        std::cerr << "Cannot open the File : " << badWordsBDFilePath <<std::endl;
        return false;
    }
	string tmpStr;
	while (getline(badWordsDBFile, tmpStr))
	{
		if (tmpStr.size() > 0)
			badWordsDB.push_back(tmpStr);
	}
	badWordsDBFile.close();
	return true;
}
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

void DeleteLineFromFile(string fileName, string deleteLine){
    string line;
	ifstream file;
	WaitForSingleObject(mutexFileMsg, INFINITE);
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
	ReleaseMutex(mutexFileMsg);
}

string ReadInputLine()
{
	ifstream checkQueueFile;
	WaitForSingleObject(mutexFileMsg, INFINITE);
	checkQueueFile.open(checkQueueFilePath.c_str());
	string line;
	getline(checkQueueFile, line);
	checkQueueFile.close();
	ReleaseMutex(mutexFileMsg);
	DeleteLineFromFile(checkQueueFilePath, line);
	//id|username|msg
	msgInfo = Split(line, "|");
	return msgInfo[2];
}

void OutputToHumFile(string msg){
   
	ofstream checkHumanFile;
	WaitForSingleObject(mutexFileHuman, INFINITE);
	checkHumanFile.open(checkHumanFilePath.c_str(), ios::app);
	if (checkHumanFile.is_open())
	{
		checkHumanFile << msg;
	}
	checkHumanFile.close();
	ReleaseMutex(mutexFileHuman);
}

int getProcessCount()
{
	return atoi(msgInfo[0].c_str());
}

string getUserName()
{
	return msgInfo[1];
}

int ScanMessage(string msg)
{
	vector<string> words = Split(msg, " ");
	double wordsCount = words.size();
	double badWordsCount = 0;
	double suspCount = 0; //suspicious chars count
	for (size_t i = 0; i < words.size(); i++)
	{
		for (size_t j = 0; j < badWordsDB.size(); j++)
		{
			if (words[i] == badWordsDB[j])
			{
				badWordsCount++;
			}
		}
		for (size_t j = 0; j < words[i].size(); j++)
		{
			if (!isalpha(words[i][j]))
			{
				suspCount++;
			}
			else
			{
				if (isupper(words[i][j]))
				{
					suspCount++;
				}
			}
		}
	}
	int result;
	currentBadWordsPercentage = badWordsCount / wordsCount;
	if (currentBadWordsPercentage * 100 > maxBadWordPercentage)
	{
		result = 1; //will be banned by AI
	}
	else if (suspCount / wordsCount * 100 > maxBadWordPercentage)
	{
		result = 2; //will be passed to the human
		OutputToHumFile(msg);
	}
	else
	{
		result = 0; //will pass checking
	}
	return result;
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


int main()
{
	string ai_text = "===================================================";
	cout << ai_text << "AI Moderator" << ai_text << endl;
	GetBadWordsDB(); //getting bad words from data base
	string userName;
	string userEventName;
	HANDLE userEvent;
	while (true)
	{
		cout << "waiting..." << endl;
		WaitForSingleObject(modAIEvent, INFINITE);
		string msg = ReadInputLine();
		userName = getUserName();
		switch (ScanMessage(msg))
		{
		case 0:
			cout << userName << "`s message passed checking:" << endl;
			cout << msg << endl;
			cout << "Percentage of forbiden words: " << currentBadWordsPercentage << endl;
			userEventName = "userEvent" + to_string(getProcessCount());
			userEvent = OpenEventA(EVENT_ALL_ACCESS, TRUE, userEventName.c_str());
			SetEvent(userEvent);
			CloseHandle(userEvent);
			break;
		case 1:
			WaitForSingleObject(mutexFileUser, INFINITE);
			BanUser(userName);
			ReleaseMutex(mutexFileUser);
			cout << userName << "`s message is banned:" << endl;
			cout << msg << endl;
			userEventName = "userEvent" + to_string(getProcessCount());
			userEvent = OpenEventA(EVENT_ALL_ACCESS, TRUE, userEventName.c_str());
			SetEvent(userEvent);
			CloseHandle(userEvent);
			break;
		case 2:
			cout << "AI can`t check " << userName << "`s message: " << endl;
			cout << msg << endl;
			cout << "Management is passed to human moderator." << endl;
			SetEvent(modHumEvent);
			break;
		}
	}

	
}

