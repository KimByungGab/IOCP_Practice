#pragma once

// include �� �̸� ���ǰ� �ʿ��� ����ü, enum �ۼ�

#include <iostream>
#include <thread>
#include <vector>
#include <deque>
#include <queue>
#include <mutex>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

using namespace std;

// �۾� ���� ����
enum class IOOperation
{
	ACCEPT,
	RECV,
	SEND
};

// WSAOVERLAPPED�� Ȯ�� ����ü
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped I/O ����ü
	SOCKET m_socketClient;			// Ŭ���̾�Ʈ ����
	WSABUF m_wsaBuf;				// Overlapped I/O �۾� ����
	IOOperation m_eOperation;		// �۾� ���� ����
	unsigned int m_sessionIndex;
};