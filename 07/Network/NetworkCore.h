#pragma once

#include "NetworkDefine.h"
#include "ClientInfo.h"
#include "Packet.h"

#define SERVER_PORT 12345				// ���� ��Ʈ
#define MAX_CLIENT 100					// �� ���� ������ Ŭ�� ��
#define RE_USE_SESSION_WAIT_TIMESEC 3	// ���� �翬�� �� �� �ִ� �ð�(��)

class NetworkCore
{
public:
	NetworkCore();
	virtual ~NetworkCore();
	bool InitSocket();
	bool BindAndListen();
	bool StartServer();
	void DestroyThread();

	// ��Ʈ��ũ �̺�Ʈ�� ó���� �Լ���
	// �̺�Ʈ�� ���� �и��Ͽ� �����ҰŶ� ������ �����Ǹ� �ϰ� �ϱ� ���� ���������Լ� ����
	virtual void OnConnect(const unsigned int clientIndex) = 0;
	virtual void OnClose(const unsigned int clientIndex) = 0;
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size) = 0;

private:
	void CreateClient(const UINT32 maxClientCount);
	bool CreateWorkerThread();
	bool CreateAcceptThread();
	bool CreateSendThread();
	ClientInfo* GetEmptyClientInfo();
	ClientInfo* GetClientInfo(const unsigned int sessionIndex);
	bool BindIoCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
	bool SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen);
	void WorkerThread();
	void AcceptThread();
	bool PostAccept(ClientInfo* pClientInfo, const UINT64 curTimeSec);
	bool AcceptCompletion(ClientInfo* pClientInfo);
	void SendThread();
	Packet GetPacketData();
	void PushPacketData(const unsigned int clientIndex, const unsigned int size, char* pData);
	void CloseSocket(ClientInfo* pClientInfo, bool bIsForce = false);

private:
	vector<ClientInfo*> mClientInfos;								// Ŭ���̾�Ʈ ������
	SOCKET mListenSocket = INVALID_SOCKET;							// Listen socket
	int mClientCnt = 0;												// ���ӵ� Ŭ���̾�Ʈ ��
	vector<thread> mIOWorkerThreads;								// IO Worker ������
	bool mIsWorkerRun = true;										// �۾� ������ ���� �÷���
	thread mAcceptThread;											// Accept ������
	bool mIsAcceptRun = true;										// Accept ���� �÷���
	thread mSendThread;												// Send ������
	bool mIsSendRun = true;											// Send ���� �÷���
	HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;						// Completion Port ��ü �ڵ�
	int mMaxWorkerThread = thread::hardware_concurrency() * 2 + 1;	// ������ Ǯ�� ���� ������ ��

	mutex mLock;													// ��Ŷ ���� ���ؽ�
	queue<Packet> mPacketQueue;									// ��Ŷ ť
};

