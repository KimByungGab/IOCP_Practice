#include <iostream>
#include <string>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_BUFFER_SIZE 1024

int main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKET hSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (hSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(12345);

	if (connect(hSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return 0;
	}

	while (true)
	{
		string message;

		cout << "메시지: ";
		getline(cin, message);

		send(hSocket, message.c_str(), message.size(), 0);

		char buffer[MAX_BUFFER_SIZE] = {};
		recv(hSocket, buffer, sizeof(buffer), 0);
		cout << "서버: " << buffer << endl;
	}

	cout << "클라 종료" << endl;
	closesocket(hSocket);
	WSACleanup();
}