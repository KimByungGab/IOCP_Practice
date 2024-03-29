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
	char m_szBuf[MAX_SOCKBUF];		// 데이터 버퍼
	IOOperation m_eOperation;		// 작업 동작 종류
};

// 클라이언트 정보를 담기 위한 구조체
class ClientInfo
{
public:
	ClientInfo();

public:
	SOCKET m_socketClient;
	stOverlappedEx m_stRecvOverlappedEx;
	stOverlappedEx m_stSendOverlappedEx;
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
	int mClientCnt = 0;												// 클라이언트 수
	vector<thread> mIOWorkerThreads;								// IO Worker 쓰레드
	thread mAcceptThread;											// Accept 쓰레드
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;						// Completion Port 객체 핸들
	bool mIsWorkerRun = true;										// 작업 쓰레드 동작 플래그
	bool mIsAcceptRun = true;										// Accept 동작 플래그
	char mSocketBuf[1024] = { 0, };									// 소켓 버퍼
	int mMaxWorkerThread = thread::hardware_concurrency() * 2 + 1;	// 쓰레드 풀에 넣을 쓰레드 수
};

