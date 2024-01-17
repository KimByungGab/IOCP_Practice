#pragma once

#include "NetworkDefine.h"
#include "ClientInfo.h"

const UINT16 SERVER_PORT = 12345;	// ���� ��Ʈ
const UINT16 MAX_CLIENT = 100;		// �� ���� ������ Ŭ�� ��

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
	virtual void OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData) = 0;

protected:
	bool SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen);
	ClientInfo* GetClientInfo(const unsigned int sessionIndex);

private:
	void CreateClient(const UINT32 maxClientCount);
	bool CreateWorkerThread();
	bool CreateAcceptThread();
	ClientInfo* GetEmptyClientInfo();
	bool BindIoCompletionPort(ClientInfo* pClientInfo);
	bool BindRecv(ClientInfo* pClientInfo);
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

