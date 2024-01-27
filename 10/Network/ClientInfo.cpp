#include "ClientInfo.h"

// mClientInfos(클라 vector)에 들어갈 클라 정보 초기화
ClientInfo::ClientInfo()
{
	// Overlapped I/O 구조체 초기화
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stAcceptOverlappedEx, 0, sizeof(stOverlappedEx));

	// 소켓 초기화
	mSocket = INVALID_SOCKET;

	// recv와 send 버퍼 초기화
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
	memset(mAcceptBuf, 0, sizeof(mAcceptBuf));
}

// 인덱스 반환
unsigned int ClientInfo::GetIndex()
{
	return mIndex;
}

// 인덱스 세팅
void ClientInfo::SetIndex(const unsigned int index)
{
	mIndex = index;
}

// 최근 접속종료 시간 반환
UINT64 ClientInfo::GetLatestClosedTimeSec()
{
	return mLatestClosedTimeSec;
}

// 최근 접속종료 시간 세팅
void ClientInfo::SetLatestClosedTimeSec(UINT64 timeSec)
{
	mLatestClosedTimeSec = timeSec;
}

// 연결 여부 확인
bool ClientInfo::IsConnected()
{
	return mIsConnect;
}

// 연결 여부 세팅
void ClientInfo::SetConnect(bool isConnect)
{
	mIsConnect = isConnect;
}

// 클라 소켓 반환
SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

// 클라 소켓 세팅
void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

// 버퍼 주소값(배열) 반환
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}