#pragma once

#include "NetworkDefine.h"

#define MAX_SOCKBUF 1024						// ���� ũ��
#define MAX_SOCK_SENDBUF 4096					// send ���� ũ��

// Ŭ���̾�Ʈ ���� Ŭ����
class ClientInfo
{
public:
	ClientInfo();
	unsigned int GetIndex();
	bool IsConnected();
	SOCKET GetSocket();
	void SetSocket(SOCKET socket);
	void SetIndex(const unsigned int index);
	char* GetRecvBuffer();
	void Clear();

public:
	int mIndex = 0;							// Ŭ�� ������(mClientInfos)�� �ε���
	SOCKET mSocket;							// Ŭ����� ���� ����
	stOverlappedEx m_stRecvOverlappedEx;	// Recv Overlapped I/O ����ü ����
	stOverlappedEx m_stSendOverlappedEx;	// Send Overlapped I/O ����ü ����

	char mRecvBuf[MAX_SOCKBUF];				// Recv ������ ����
	char mSendBuf[MAX_SOCK_SENDBUF];		// Send ������ ����
};