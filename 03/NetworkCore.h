#pragma once

#include <iostream>
#include <thread>
#include <vector>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_SOCKBUF 1024			// ��Ŷ ũ��
const UINT16 SERVER_PORT = 12345;	// ���� ��Ʈ
const UINT16 MAX_CLIENT = 100;		// �� ���� ������ Ŭ�� ��

// �۾� ���� ����
enum class IOOperation
{
	RECV,
	SEND
};

// WSAOVERLAPPED�� Ȯ�� ����ü
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;	// Overlapped I/O ����ü
	SOCKET m_socketClient;			// Ŭ���̾�Ʈ ����
	WSABUF m_wsaBuf;				// Overlapped I/O �۾� ����
	IOOperation m_eOperation;		// �۾� ���� ����
};

// Ŭ���̾�Ʈ ������ ��� ���� ����ü
class ClientInfo
{
public:
	ClientInfo(const unsigned int index);

public:
	int mIndex = 0;							// Ŭ�� ������(mClientInfos)�� �ε���
	SOCKET mClientSocket;					// Ŭ����� ���� ����
	stOverlappedEx m_stRecvOverlappedEx;	// Recv Overlapped I/O ����ü ����
	stOverlappedEx m_stSendOverlappedEx;	// Send Overlapped I/O ����ü ����

	char mRecvBuf[MAX_SOCKBUF];		// Recv ������ ����
	char mSendBuf[MAX_SOCKBUF];		// Send ������ ����
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

	// ��Ʈ��ũ �̺�Ʈ�� ó���� �Լ���
	// �̺�Ʈ�� ���� �и��Ͽ� �����ҰŶ� ������ �����Ǹ� �ϰ� �ϱ� ���� ���������Լ� ����
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
	vector<ClientInfo> mClientInfos;								// Ŭ���̾�Ʈ ������
	SOCKET mListenSocket = INVALID_SOCKET;							// Listen socket
	int mClientCnt = 0;												// ���ӵ� Ŭ���̾�Ʈ ��
	vector<thread> mIOWorkerThreads;								// IO Worker ������
	thread mAcceptThread;											// Accept ������
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;						// Completion Port ��ü �ڵ�
	bool mIsWorkerRun = true;										// �۾� ������ ���� �÷���
	bool mIsAcceptRun = true;										// Accept ���� �÷���
	char mSocketBuf[1024] = { 0, };									// ���� ����
	int mMaxWorkerThread = thread::hardware_concurrency() * 2 + 1;	// ������ Ǯ�� ���� ������ ��
};

