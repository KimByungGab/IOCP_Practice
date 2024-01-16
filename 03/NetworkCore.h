#pragma once

#include <iostream>
#include <thread>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_SOCKBUF 1024			// 패킷 크기
const UINT16 SERVER_PORT = 12345;	// 서버 포트
const UINT16 MAX_CLIENT = 100;		// 총 접속 가능한 클라 수

// 작업 동작 종류
enum class IOOperation
{
	RECV,
	SEND
};

// WSAOVERLAPPED의 확장 구조체
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped I/O 구조체
	SOCKET m_socketClient;			// 클라이언트 소켓
	WSABUF m_wsaBuf;				// Overlapped I/O 작업 버퍼
	IOOperation m_eOperation;		// 작업 동작 종류
};

// 클라이언트 정보를 담기 위한 구조체
class ClientInfo
{
public:
	ClientInfo(const unsigned int index);

public:
	int mIndex = 0;							// 클라 정보들(mClientInfos)의 인덱스
	SOCKET mClientSocket;					// 클라와의 연결 소켓
	stOverlappedEx m_stRecvOverlappedEx;	// Recv Overlapped I/O 구조체 변수
	stOverlappedEx m_stSendOverlappedEx;	// Send Overlapped I/O 구조체 변수

	char mRecvBuf[MAX_SOCKBUF];		// Recv 데이터 버퍼
	char mSendBuf[MAX_SOCKBUF];		// Send 데이터 버퍼
};

class NetworkCore
{
public:
	NetworkCore() {};
	~NetworkCore() { WSACleanup(); }
	bool InitSocket();
	bool BindAndListen();
	bool StartServer();
	void DestoryThread();

	// 네트워크 이벤트를 처리할 함수들
	// 이벤트를 따로 분리하여 구현할거라 무조건 재정의를 하게 하기 위해 순수가상함수 구현
	virtual void OnConnect(const unsigned int clientIndex) = 0;
	virtual void OnClose(const unsigned int clientIndex) = 0;
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData) = 0;

private:
	void CreateClient(const UINT32 maxClientCount);
	bool CreateWorkerThread();
	bool CreateAcceptThread();
	ClientInfo* GetEmptyClientInfo();
	bool BindIoCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen);
	void WorkerThread();
	void AcceptThread();
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

private:
	vector<ClientInfo> mClientInfos;								// 클라이언트 정보들
	SOCKET mListenSocket = INVALID_SOCKET;							// Listen socket
	int mClientCnt = 0;												// 접속된 클라이언트 수
	vector<thread> mIOWorkerThreads;								// IO Worker 쓰레드
	thread mAcceptThread;											// Accept 쓰레드
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;						// Completion Port 객체 핸들
	bool mIsWorkerRun = true;										// 작업 쓰레드 동작 플래그
	bool mIsAcceptRun = true;										// Accept 동작 플래그
	char mSocketBuf[1024] = { 0, };									// 소켓 버퍼
	int mMaxWorkerThread = thread::hardware_concurrency() * 2 + 1;	// 쓰레드 풀에 넣을 쓰레드 수
};

