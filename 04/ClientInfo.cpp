#include "ClientInfo.h"

// mClientInfos(클라 vector)에 들어갈 클라 정보 초기화
ClientInfo::ClientInfo(const unsigned int index)
{
	// Overlapped I/O 구조체 초기화
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));

	// 소켓 초기화
	mSocket = INVALID_SOCKET;

	// 인덱스 정의
	mIndex = index;

	// recv와 send 버퍼 초기화
	memset(mRecvBuf, 0, sizeof(mRecvBuf));
	memset(mSendBuf, 0, sizeof(mSendBuf));
}

// 인덱스 반환
unsigned int ClientInfo::GetIndex()
{
	return mIndex;
}

// 연결 여부 확인
bool ClientInfo::IsConnected()
{
	return mSocket != INVALID_SOCKET;
}

// 클라 소켓 반환
SOCKET ClientInfo::GetSocket()
{
	return mSocket;
}

void ClientInfo::SetSocket(SOCKET socket)
{
	mSocket = socket;
}

// 버퍼 주소값(배열) 반환
char* ClientInfo::GetRecvBuffer()
{
	return mRecvBuf;
}

// 클라이언트 종료
void ClientInfo::Close(bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER로 설정함

	// bIsForce가 true면 SO_LINGER, timeout = 0으로 설정해서 강제 종료. 데이터 손실 될 수 있음
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// 소켓의 send recv 전부 중단
	shutdown(mSocket, SD_BOTH);

	// 소켓 옵션을 지정해둔 linger로 설정
	setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// 소켓 종료
	closesocket(mSocket);

	// 소켓이 필요없어졌다고 마음대로 삭제하고 재배치하다간 나중에 큰일이 날 수 있음.
	// 때문에 pool의 용도로 활용하기 위하여 소켓만 죽여둠
	mSocket = INVALID_SOCKET;
}

// 현재는 아직 용도를 확인할 수 없음.
// 아마 초기화의 정보가 아닐까 싶긴 한데 쓸모가 있으리라 판단하여 남겨둔다.
void ClientInfo::Clear()
{
}