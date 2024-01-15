#include "NetworkCore.h"

// ���� �� listen socket �ʱ�ȭ
bool NetworkCore::InitSocket()
{
	// ���� �ʱ�ȭ
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return false;
	}

	// listen socket �ʱ�ȭ
	// IOCP�� ����ϱ� ���� WSA_FLAG_OVERLAPPED �÷��� ���
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (mListenSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// ���� ���� bind �� ���� listen
bool NetworkCore::BindAndListen()
{
	// ���� ����
	// �޴� IP�� ��� �ޱ�
	// ������Ʈ�� DB���� ������ ó���ϸ� �� ��
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// ���� bind
	int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (nRet != 0)
	{
		cout << "bind() error: " << WSAGetLastError() << endl;
		return false;
	}

	// ���� listen, ���� ť�� �ִ���̷�
	nRet = listen(mListenSocket, SOMAXCONN);
	if (nRet != 0)
	{
		cout << "listen() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// ���� ����
bool NetworkCore::StartServer()
{
	// Ŭ���̾�Ʈ Ǯ ����
	CreateClient(MAX_CLIENT);

	// IOCP �ʱ�ȭ, ������ ���� �̸� ����
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	// ��Ŀ ������ ����
	bool bRet = CreateWorkerThread();
	if (bRet == false)
		return false;

	// Accept �ϴ� ������ ����
	bRet = CreateAcceptThread();
	if (bRet == false)
		return false;

	cout << "���� ����" << endl;
	return true;
}

// �����Ǿ��ִ� ������ ������ �Ҹ�
void NetworkCore::DestoryThread()
{
	// ��Ŀ ������ �ݺ��� ����
	mIsWorkerRun = false;

	// IOCP ��ü �ڵ鷯 ����
	CloseHandle(mIOCPHandle);

	// ��Ŀ ������ ���� join�Ͽ� ����
	for (auto& t : mIOWorkerThreads)
	{
		if (t.joinable())
			t.join();
	}

	//Accept ������ �ݺ��� ����
	mIsAcceptRun = false;

	// listen socket �ݱ�
	closesocket(mListenSocket);

	// Accept ������ ���� join�Ͽ� ����
	if (mAcceptThread.joinable())
		mAcceptThread.join();
}

// Ŭ���̾�Ʈ Ǯ ����
void NetworkCore::CreateClient(const UINT32 maxClientCount)
{
	for (UINT32 i = 0; i < maxClientCount; i++)
	{
		mClientInfos.push_back(move(ClientInfo()));
	}
}

// ��Ŀ ������ ����
bool NetworkCore::CreateWorkerThread()
{
	// unsigned int uiThreadId = 0;

	// ��Ŀ ������ ����
	// MSDN���� ���� ������ ���尳���� '�ϵ���� ������ �� * 2 + 1'
	// ����: �����尡 Suspend�Ǿ� �����·� ������ �� ��� ���ο� �����带 ������ ����
	for (int i = 0; i < mMaxWorkerThread; i++)
	{
		mIOWorkerThreads.push_back(thread([this]()
			{
				WorkerThread();
			}));
	}

	cout << "WorkerThread ����" << endl;

	return true;
}

// Accept ������ ����
bool NetworkCore::CreateAcceptThread()
{
	mAcceptThread = thread([this]()
		{
			AcceptThread();
		});

	cout << "AcceptThread ����" << endl;
	return true;
}

// ������� �ʴ� Ŭ���̾�Ʈ ����ü ��ȯ
// ������� �ʴ� Ŭ���̾�Ʈ ����ü�� �ϳ��� ������ �� ����ü�� ��ȯ
ClientInfo* NetworkCore::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (client.m_socketClient == INVALID_SOCKET)
			return &client;
	}

	return nullptr;
}

// Completion Port ��ü�� ���ϰ� Completion Key�� ����
// �� IOCP ��¥ ����
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// socket�� pClientInfo�� Completion Port ��ü�� ����
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, mIOCPHandle, (ULONG_PTR)pClientInfo, 0);

	if (hIOCP == NULL || mIOCPHandle != hIOCP)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// WSARecv Overlapped I/O �۾�
bool NetworkCore::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// Overlapped I/O�� ���� ���� ����
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

	int nRet = WSARecv(pClientInfo->m_socketClient,
		&(pClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		&dwFlag,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stRecvOverlappedEx),
		NULL);

	// socket error�� client socket �������ŷ� ����
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSARecv() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// WSASend Overlapped I/O �۾�
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// ���۵� �޽��� ����
	CopyMemory(pClientInfo->m_stSendOverlappedEx.m_szBuf, pMsg, nLen);

	// Overlapped I/O�� ���� ���� ����
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stSendOverlappedEx.m_szBuf;
	pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

	int nRet = WSASend(pClientInfo->m_socketClient,
		&(pClientInfo->m_stSendOverlappedEx.m_wsaBuf),
		1,
		&dwRecvNumBytes,
		0,
		(LPWSAOVERLAPPED) & (pClientInfo->m_stSendOverlappedEx),
		NULL);

	// socket error�� client socket �������ŷ� ����
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// Overlapped I/O �۾��� ���� �Ϸ� �뺸�� �޾� �ش� ó���� �ϴ� �Լ�
void NetworkCore::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;			// Completion Key�� ���� ������ ����
	BOOL bSuccess = TRUE;						// �Լ� ȣ�� ���� ����
	DWORD dwIoSize = 0;							// Overlapped I/O �۾����� ���۵� ������ ũ��
	LPOVERLAPPED lpOverlapped = nullptr;		// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������

	// �� �Լ��� ���� ��������� Waiting Thread Queue�� �����·� ��
	// �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue���� �Ϸ�� �۾��� ������ ������ ó��
	// PostQueuedCompletionStatus() �Լ��� ���� ����� �޽����� �����Ǹ� �����带 ����
	while (mIsWorkerRun)
	{
		// �Ű����� ����
		// 1) IOCP �ڵ鷯
		// 2) ������ ���۵� ����Ʈ
		// 3) Completion Key
		// 4) Overlapped I/O ��ü
		// 5) Ÿ�Ӿƿ� �ð�
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
			&dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);

		// Ŭ�� ������ ���� �޽��� ó��
		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			mIsWorkerRun = false;
			continue;
		}

		// I/O ������ ������
		// ������ overlapped�� nullptr�̶�簡 ������ ũ��(dwIoSize)�� �̻��ϴٸ� ������ �����Ų�ٰ� ��.
		// ����: https://www.slideshare.net/namhyeonuk90/iocp
		if (lpOverlapped == nullptr)
		{
			continue;
		}

		// Ŭ�� ������ ����
		if (bSuccess == FALSE || (bSuccess == TRUE && dwIoSize == 0))
		{
			cout << "Ŭ�� ���� ����! " << (int)pClientInfo->m_socketClient << endl;
			CloseSocket(pClientInfo);
			continue;
		}

		// overlapped ����
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// Overlapped I/O Recv �۾� ��� ó��
		switch (pOverlappedEx->m_eOperation)
		{
			// Recv ���
		case IOOperation::RECV:
			pOverlappedEx->m_szBuf[dwIoSize] = NULL;
			cout << "[����] bytes: " << dwIoSize << ", msg: " << pOverlappedEx->m_szBuf << endl;

			// ======================================
			// ��Ŷ�� ���� ���ϴ� ��� �����ϴ� ���� ����
			// TODO: ���ڰ� �ƴ϶�� ���� ���⼭ ��Ŷ �м��ϰ� ���ϴ� �۾��� ����
			// Ŭ�󿡰� �޽��� ����
			SendMsg(pClientInfo, pOverlappedEx->m_szBuf, dwIoSize);
			// ��Ŷ�� ���� ���ϴ� ��� �����ϴ� ���� ��
			// ======================================

			// �ѹ� �޾����� ���˴�ó�� Recv�� �ٽ� ������ߵȴ�.
			// ���� �������� ������ ���� �� �޴´�.
			BindRecv(pClientInfo);
			break;

			// Send ���
			// �������� ���� �����⸸ �ϴ°��̴� �Ѿ
		case IOOperation::SEND:
			cout << "[�۽�] bytes: " << dwIoSize << ", msg: " << pOverlappedEx->m_szBuf << endl;
			break;

			// ���� ó��
		default:
			cout << "socket(" << (int)pClientInfo->m_socketClient << ")���� ���ܻ�Ȳ" << endl;
			break;
		}
	}
}

// ������� ������ �޴� ������
void NetworkCore::AcceptThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (mIsAcceptRun)
	{
		// ���ӹ��� Ŭ�� ����ü�� �ε����� ��������
		ClientInfo* pClientInfo = GetEmptyClientInfo();

		// ���� Ŭ�� ����ü�� ������ return
		// ��� ���⼭ ���� ���� ����ü�� ������ Ŭ�� ����ü vector(mClientInfos)�� �ѱ�� �� ������ ������δ� ���� ��.
		if (pClientInfo == nullptr)
		{
			cout << "Client Full" << endl;
			return;
		}

		// Ŭ�� ���� ��û�� ���ö����� ���
		pClientInfo->m_socketClient = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (pClientInfo->m_socketClient == INVALID_SOCKET)
			continue;

		// IOCP ��ü�� ���� ����
		bool bRet = BindIoCompletionPort(pClientInfo);
		if (bRet == false)
			return;

		// Recv Overlapped I/O �۾�
		bRet = BindRecv(pClientInfo);
		if (bRet == false)
			return;

		// ���� � IP�� �����ߴ��� �����ֱ� ���� ���
		// ���� ��������ٸ� ���� ����� DB�� ��Ƶδ� �۾��� �������� ��
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
		cout << "Ŭ�� ����: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->m_socketClient << ")" << endl;

		// Ŭ�� ���� ���� ����
		mClientCnt++;
	}
}

// ���� ���� ����
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER�� ������

	// bIsForce�� true�� SO_LINGER, timeout = 0���� �����ؼ� ���� ����. ������ �ս� �� �� ����
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// ������ send recv ���� �ߴ�
	shutdown(pClientInfo->m_socketClient, SD_BOTH);

	// ���� �ɼ��� �����ص� linger�� ����
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// ���� ����
	closesocket(pClientInfo->m_socketClient);

	// ������ �ʿ�������ٰ� ������� �����ϰ� ���ġ�ϴٰ� ���߿� ū���� �� �� ����.
	// ������ pool�� �뵵�� Ȱ���ϱ� ���Ͽ� ���ϸ� �׿���
	pClientInfo->m_socketClient = INVALID_SOCKET;
}

// mClientInfos(Ŭ�� vector)�� �� Ŭ�� ���� �ʱ�ȭ
ClientInfo::ClientInfo()
{
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));
	m_socketClient = INVALID_SOCKET;
}
