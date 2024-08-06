#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include "OpenFile.h"

/* 
* 
*          READ ME !!!
* 
    This dll injector is not meant for games with anti-cheats. if you do so expect a ban when you inject
    because this uses loadlibrary method to map its contents. 
*/


namespace inputs
{
    bool sortAlphabetically = false;
}

std::vector<std::string> EnumProcs()
{
    std::vector<std::string> pids;
    std::map<int, std::string> processMap; // Map to associate an index with each process name


    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

        int index = 1; // Start index from 1

        if (Process32First(snapshot, &pe32))
        {
            do
            {
                pids.push_back(pe32.szExeFile);
                processMap[index++] = pe32.szExeFile;
               

            } while (Process32Next(snapshot, &pe32));
        }
        CloseHandle(snapshot);

        if (inputs::sortAlphabetically)
        {
            // Sort the vector alphabetically
            std::sort(pids.begin(), pids.end());
        }
    }
    return pids;
}

int main()
{

    extern std::string sSelectedFile;
    extern std::string sFilePath;
    DWORD exitCode = 0;


    std::cout << "Sort processes alphabetically? 1 = yes/0 = no: ";
    std::cin >> inputs::sortAlphabetically;


    std::vector<std::string> pids = EnumProcs();
    std::string processToUnload;

    for (int i = 0; i < pids.size(); i++) {
        std::cout << pids[i];
        std::cout << " | Index " << i + 1 << std::endl;
    }

    int processIndex;
    std::cout << "Enter a process index: ";
    std::cin >> processIndex;

    if (processIndex > 0 && processIndex <= static_cast<int>(pids.size()))
    {
        std::cout << "Process name at index " << processIndex << ": " << pids[processIndex - 1] << std::endl;
        processToUnload = pids[processIndex - 1];
    }
    else
    {
        std::cout << "Invalid process index." << std::endl;
    }

    bool result = openFile();
    if (result) {
        printf("SELECTED FILE: %s\nFILE PATH: %s\n\n", sSelectedFile.c_str(), sFilePath.c_str());
    }

    HANDLE snapshot = 0;
    PROCESSENTRY32 pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32);
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    Process32First(snapshot, &pe32);
    do {
        if (strcmp(pe32.szExeFile, processToUnload.c_str()) == 0) {
            HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);
            if (!process) {
    
                std::cout << "Could not inject!\n";
            }
    
    
            void* lpBaseAddress = VirtualAllocEx(process, NULL, strlen(sFilePath.c_str()) + 1, MEM_COMMIT, PAGE_READWRITE);
    
            WriteProcessMemory(process, lpBaseAddress, sFilePath.c_str(), strlen(sFilePath.c_str()) + 1, NULL);
    
            // Creating the thread
            // Loadlib lives in kernel32
            HMODULE kernel32base = GetModuleHandle("kernel32.dll");
    
            HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32base, "LoadLibraryA"), lpBaseAddress, 0, NULL);
    
            WaitForSingleObject(thread, INFINITE);
            GetExitCodeThread(thread, &exitCode);
    
            VirtualFreeEx(process, lpBaseAddress, 0, MEM_RELEASE);
            CloseHandle(thread);
            CloseHandle(process);
            break;
        }
    
    } while (Process32Next(snapshot, &pe32));

    system("pause");

}
