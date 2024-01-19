#pragma once

#include "NetworkDefine.h"

#define MAX_SOCKBUF 1024						// 버퍼 크기
#define MAX_SOCK_SENDBUF 4096					// send 버퍼 크기

// 클라이언트 정보 클래스
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
	int mIndex = 0;							// 클라 정보들(mClientInfos)의 인덱스
	SOCKET mSocket;							// 클라와의 연결 소켓
	stOverlappedEx m_stRecvOverlappedEx;	// Recv Overlapped I/O 구조체 변수
	stOverlappedEx m_stSendOverlappedEx;	// Send Overlapped I/O 구조체 변수

	char mRecvBuf[MAX_SOCKBUF];				// Recv 데이터 버퍼
	char mSendBuf[MAX_SOCK_SENDBUF];		// Send 데이터 버퍼
};