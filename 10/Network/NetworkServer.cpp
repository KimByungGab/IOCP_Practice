#include "NetworkServer.h"

// 연결 이벤트
void NetworkServer::OnConnect(const unsigned int clientIndex)
{
	cout << "[OnConnect] 인덱스: " << clientIndex << endl;
}

// 소켓 해제 이벤트
void NetworkServer::OnClose(const unsigned int clientIndex)
{
	cout << "[OnClose] 인덱스: " << clientIndex << endl;
}

// recv 이벤트
void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size)
{
	cout << "[OnReceive] 인덱스: " << clientIndex << " / 데이터크기: " << size;
}

// 서버 구동
void NetworkServer::Run()
{
	StartServer();
}

// 서버 종료
void NetworkServer::End()
{
	// 네트워크 관련 쓰레드 전부 종료
	DestroyThread();
}