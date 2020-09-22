/* **********************************************************************
 *	PoC: GetUserFromProc	
 *	Author: lorenzoinvidia
 *
 *  Description:
 *	Get the domain\user of a given pid
 *
 *	Compile: cl /EHsc GetUserFromProc.cpp
 *
 * ********************************************************************** 
 * */

#include <Windows.h>
#include <stdio.h>

#define MAX_NAME 256


// adapted from https://stackoverflow.com/questions/2686096/c-get-username-from-process
int main(int argc, const char* argv[]){

    if (argc != 2) {
        printf("[i] %s <PID>\r\n", argv[0]);
        return 1;
    };

    BOOL var = FALSE;
    HANDLE  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, atoi(argv[1])),
            hToken;
    if (!hProcess) {
        // error = 5 --> user doesn't have sufficient privileges to query info
        printf("[-] OpenProcess: %d\n", GetLastError());
        return 1;
    }

    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        printf("[-] OpenProcessToken: %d\n", GetLastError());
        return 1;
    }


    PTOKEN_USER pTokenUser = NULL;
    DWORD dwLength = 0;

    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, 0, &dwLength)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            printf("[-] GetTokenInformation: %d\n", GetLastError());
            return 1;
        }
    }
    pTokenUser = (PTOKEN_USER)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwLength);

    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength)) {
        printf("[-] GetTokenInformation: %d\n", GetLastError());
        return 1;
    }

    SID_NAME_USE SidType;
    char lpName[MAX_NAME];
    char lpDomain[MAX_NAME];
    DWORD dwSize = MAX_NAME;

    if (!LookupAccountSid(NULL, pTokenUser->User.Sid, lpName, &dwSize, lpDomain, &dwSize, &SidType)){
        DWORD dwResult = GetLastError();
        if (dwResult == ERROR_NONE_MAPPED)
            strcpy_s(lpName, MAX_NAME, "NONE_MAPPED");
        else{
            printf("[-] LookupAccountSid: %d\n", GetLastError());
        }
    }

    printf("[i] Current user is %s\\%s\n", lpDomain, lpName);

    HeapFree(GetProcessHeap(), 0, (LPVOID)pTokenUser);

    CloseHandle(hToken);
    CloseHandle(hProcess);
    return 0;
}
