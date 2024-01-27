#pragma once

#include "NetworkDefine.h"
#include "ClientInfo.h"
#include "Packet.h"
#include "PacketManager.h"

#define SERVER_PORT 12345				// 서버 포트
#define MAX_CLIENT 100					// 총 접속 가능한 클라 수
#define RE_USE_SESSION_WAIT_TIMESEC 3	// 세션 재연결 할 수 있는 시간(초)

class NetworkCore
{
public:
	NetworkCore();
	virtual ~NetworkCore();
	bool InitSocket();
	bool BindAndListen();
	bool StartServer();
	void DestroyThread();

	bool SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen);
	ClientInfo* GetClientInfo(const unsigned int sessionIndex);

	// 네트워크 이벤트를 처리할 함수들
	// 이벤트를 따로 분리하여 구현할거라 무조건 재정의를 하게 하기 위해 순수가상함수 구현
	virtual void OnConnect(const unsigned int clientIndex) = 0;
	virtual void OnClose(const unsigned int clientIndex) = 0;
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size) = 0;

private:
	void CreateClient(const UINT32 maxClientCount);
	bool CreateWorkerThread();
	bool CreateAcceptThread();
	ClientInfo* GetEmptyClientInfo();
	bool BindIoCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
	void WorkerThread();
	void AcceptThread();
	bool PostAccept(ClientInfo* pClientInfo, const UINT64 curTimeSec);
	bool AcceptCompletion(ClientInfo* pClientInfo);
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

private:
	vector<ClientInfo*> mClientInfos;								// 클라이언트 정보들
	SOCKET mListenSocket = INVALID_SOCKET;							// Listen socket
	int mClientCnt = 0;												// 접속된 클라이언트 수
	vector<thread> mIOWorkerThreads;								// IO Worker 쓰레드
	bool mIsWorkerRun = false;										// 작업 쓰레드 동작 플래그
	thread mAcceptThread;											// Accept 쓰레드
	bool mIsAcceptRun = false;										// Accept 동작 플래그
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;						// Completion Port 객체 핸들
	int mMaxWorkerThread = thread::hardware_concurrency() * 2 + 1;	// 쓰레드 풀에 넣을 쓰레드 수

	unique_ptr<PacketManager> m_pPacketManager;						// 패킷 매니저
};

