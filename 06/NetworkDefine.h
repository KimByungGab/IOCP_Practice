#pragma once

// include �� �̸� ���ǰ� �ʿ��� ����ü, enum �ۼ�

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

// �۾� ���� ����
enum class IOOperation
{
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
};