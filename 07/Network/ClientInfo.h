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
	void SetConnect(bool isConnect = false);
	SOCKET GetSocket();
	void SetSocket(SOCKET socket);
	void SetIndex(const unsigned int index);
	UINT64 GetLatestClosedTimeSec();
	void SetLatestClosedTimeSec(UINT64 timeSec);
	char* GetRecvBuffer();
	void Clear();

public:
	int mIndex = 0;							// Ŭ�� ������(mClientInfos)�� �ε���
	bool mIsConnect = false;				// ���� ����
	UINT64 mLatestClosedTimeSec = 0;		// �ֱ� ������ �������� �� �ð�(��)
	SOCKET mSocket;							// Ŭ����� ���� ����
	stOverlappedEx m_stRecvOverlappedEx;	// Recv Overlapped I/O ����ü ����
	stOverlappedEx m_stSendOverlappedEx;	// Send Overlapped I/O ����ü ����
	stOverlappedEx m_stAcceptOverlappedEx;	// Accept Overlapped I/O ����ü ����

	char mRecvBuf[MAX_SOCKBUF];				// Recv ������ ����
	char mSendBuf[MAX_SOCK_SENDBUF];		// Send ������ ����
	char mAcceptBuf[64];					// Accept ������ ����
};