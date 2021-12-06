#include <iostream>
#include <Windows.h>
#include <string>

using namespace std;


HANDLE mutexFileUser = CreateMutexA(NULL, TRUE, "mutexFileUser"); //common mutex for working with message file
HANDLE mutexFileMsg = CreateMutexA(NULL, TRUE, "mutexFileMsg"); //common mutex for working with message queue file
HANDLE mutexFileHuman = CreateMutexA(NULL, TRUE, "mutexFileHuman"); //common mutex for working with message queue for human moderator
HANDLE mutexBadWordsDB = CreateMutexA(NULL, TRUE, "mutexBadWordsDB"); //common mutex for working with database of bad words
HANDLE modAIEvent = CreateEventA(NULL, TRUE, FALSE, "modAIEvent"); //event to give signal for AI moderator
HANDLE modHumEvent = CreateEventA(NULL, TRUE, FALSE, "modHumEvent"); //event to give signal for moderator-human
HANDLE* usersEvent; //array of events for each user

string user_filepath = "C:/Users/ВАНЯ/Downloads/lol/course1.exe"; //path of user executing file and its arguments "C:/Folder/user.exe"
string aimod_filepath = "C:/Users/ВАНЯ/Downloads/lol/course1.exe"; //path of AI moderator executing file and its arguments "C:/Folder/ai.exe"
string humod_filepath = "C:/Users/ВАНЯ/Downloads/lol/course1.exe"; //path of moderator-human executing file and its arguments "C:/Folder/human.exe"

//structure with all process creation information
struct pinfo
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    string path;
    
    pinfo()
    {
        ZeroMemory(&si, sizeof(STARTUPINFO));
        this->si.cb = sizeof(STARTUPINFO);
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    }
    ~pinfo()
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
};

//creating of process
bool pcreate(pinfo& proc_inf)
{
    setlocale(LC_ALL, "Russian");
    wchar_t converted_path[500];
    mbstowcs(converted_path, proc_inf.path.c_str(), proc_inf.path.size());
    if (!CreateProcessW(NULL, 
                        converted_path, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &proc_inf.si, 
                        &proc_inf.pi))
        return false;
    return true;
}

//terminating a process
void pterminate(pinfo& proc_inf)
{
    TerminateProcess(proc_inf.pi.hProcess, 0);
}

void close_handles(int usersAm)
{
    for (size_t i = 0; i < usersAm; i++)
        CloseHandle(usersEvent[i]);
    CloseHandle(mutexFileUser);
    CloseHandle(modAIEvent);
    CloseHandle(modHumEvent);
}

int main()
{
    cout << "=================================================== Forum Process Manager ===================================================" << endl << endl;
    int usersAm;
    cout << "Enter the amount of users: " << endl;
    cin >> usersAm; //amount of users to create
    usersEvent = new HANDLE[usersAm];
    for (size_t i = 0; i < usersAm; i++)
    {
        string user = "userEvent" + to_string(i + 1);
        usersEvent[i] = CreateEventA(NULL, TRUE, FALSE, user.c_str());
    }
    pinfo* usersProc = new pinfo[usersAm];
    for (size_t i = 0; i < usersAm; i++)
    {
        usersProc[i].path = user_filepath + " " + to_string(i + 1);
        if (!pcreate(usersProc[i]))
        {
            cout << "Error creating user #" << i + 1 << endl;
            return -(i + 1);
        }
    }
    pinfo modAIProc, modHumProc;
    modAIProc.path = aimod_filepath + " " + to_string(usersAm);
    modHumProc.path = humod_filepath + " " + to_string(usersAm);
    if (!pcreate(modAIProc))
    {
        cout << "Error creating computer moderator!" << endl;
        return 1;
    }
    if (!pcreate(modHumProc))
    {
        cout << "Error creating person moderator!" << endl;
        return 2;
    }

    system("pause");

    for (size_t i = 0; i < usersAm; i++)
        pterminate(usersProc[i]);
    pterminate(modAIProc);
    pterminate(modHumProc);
    close_handles(usersAm);
    
    return 0;
}