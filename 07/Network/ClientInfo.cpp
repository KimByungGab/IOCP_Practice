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

UINT64 ClientInfo::GetLatestClosedTimeSec()
{
	return mLatestClosedTimeSec;
}

void ClientInfo::SetLatestClosedTimeSec(UINT64 timeSec)
{
	mLatestClosedTimeSec = timeSec;
}

// 연결 여부 확인
bool ClientInfo::IsConnected()
{
	return mIsConnect;
}

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

// 현재는 아직 용도를 확인할 수 없음.
// 아마 초기화의 정보가 아닐까 싶긴 한데 쓸모가 있으리라 판단하여 남겨둔다.
void ClientInfo::Clear()
{
}