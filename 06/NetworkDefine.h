#pragma once

// include 및 미리 정의가 필요한 구조체, enum 작성

#include <iostream>
#include <thread>
#include <vector>
#include <deque>
#include <queue>
#include <mutex>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// 작업 동작 종류
enum class IOOperation
{
	RECV,
	SEND
};

// WSAOVERLAPPED의 확장 구조체
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped I/O 구조체
	SOCKET m_socketClient;			// 클라이언트 소켓
	WSABUF m_wsaBuf;				// Overlapped I/O 작업 버퍼
	IOOperation m_eOperation;		// 작업 동작 종류
};