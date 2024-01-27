#include "NetworkCore.h"

NetworkCore::NetworkCore()
{

}

NetworkCore::~NetworkCore()
{
	WSACleanup();
}

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

	cout << "���� �ʱ�ȭ ����" << endl;
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
	int nRet = ::bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
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

	// listen���� accept ���� ���ϰ� �ϱ�
	BOOL on = true;
	setsockopt(mListenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(on));

	// IOCP �ʱ�ȭ, ������ ���� �̸� ����
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	// listen socket�� IOCP �ڵ� ����
	auto hIOCPHandle = CreateIoCompletionPort((HANDLE)mListenSocket, mIOCPHandle, (UINT32)0, 0);
	if (hIOCPHandle == nullptr)
	{
		cout << "listen socket IOCP bind error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "���� ��� ����" << endl;
	return true;
}

// ���� ����
bool NetworkCore::StartServer()
{
	// ��Ŷ �Ŵ��� ����
	m_pPacketManager = make_unique<PacketManager>();
	m_pPacketManager->SendPacketFunc = [&](UINT32 clientIndex, UINT16 packetSize, char* pSendPacket)
		{
			SendMsg(GetClientInfo(clientIndex), pSendPacket, packetSize);
		};
	m_pPacketManager->Init(MAX_CLIENT);
	m_pPacketManager->Run();

	// Ŭ���̾�Ʈ Ǯ ����
	CreateClient(MAX_CLIENT);

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
void NetworkCore::DestroyThread()
{
	// ��Ŷ �Ŵ��� ����
	m_pPacketManager->End();

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
		ClientInfo* client = new ClientInfo();
		client->SetIndex(i);

		mClientInfos.push_back(client);
	}
}

// ��Ŀ ������ ����
bool NetworkCore::CreateWorkerThread()
{
	mIsWorkerRun = true;

	// ��Ŀ ������ ����
	// MSDN���� ���� ������ ���尳���� '�ϵ���� �ھ� �� * 2 + 1'
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
	mIsAcceptRun = true;
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
		if (client->IsConnected() == false)
			return client;
	}

	return nullptr;
}

// �ε����� Ŭ���̾�Ʈ ���� ����ü ��ȯ
ClientInfo* NetworkCore::GetClientInfo(const unsigned int sessionIdx)
{
	return mClientInfos[sessionIdx];
}

// Completion Port ��ü�� ���ϰ� Completion Key�� ����
// �� IOCP ��¥ ����
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// socket�� pClientInfo�� Completion Port ��ü�� ����
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->GetSocket(), mIOCPHandle, (ULONG_PTR)(pClientInfo), 0);

	if (hIOCP == INVALID_HANDLE_VALUE)
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
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->mRecvBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

	int nRet = WSARecv(pClientInfo->GetSocket(),
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
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// ���۵� �޽��� ����
	CopyMemory(pClientInfo->mSendBuf, pMsg, nLen);

	// Overlapped I/O�� ���� ���� ����
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.len = nLen;
	pClientInfo->m_stSendOverlappedEx.m_wsaBuf.buf = pClientInfo->mSendBuf;
	pClientInfo->m_stSendOverlappedEx.m_eOperation = IOOperation::SEND;

	int nRet = WSASend(pClientInfo->GetSocket(),
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
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);

		// ����� ������ ���� �޽��� ó��
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

		// overlapped ����
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// Ŭ�� ������ ����
		if (bSuccess == FALSE || (dwIoSize == 0 && (pOverlappedEx->m_eOperation != IOOperation::ACCEPT)))
		{
			cout << "Ŭ�� ���� ����! " << (int)pClientInfo->GetSocket() << endl;
			CloseSocket(pClientInfo);
			continue;
		}

		// Overlapped I/O Recv �۾� ��� ó��
		switch (pOverlappedEx->m_eOperation)
		{
			// Accept ���
		case IOOperation::ACCEPT:
			pClientInfo = GetClientInfo(pOverlappedEx->m_sessionIndex);
			if (AcceptCompletion(pClientInfo))
			{
				OnConnect(pClientInfo->mIndex);

				// Ŭ�� ���� ���� ����
				mClientCnt++;

				// �α��� �ߴٰ� ���� �ֱ�
				// ���⼭�� ������ ���� ���Ŀ� ������ �ϴ� ���̹Ƿ� �����̴�.
				// ������ ���� ������ �������� ������ ���� �ʾұ� ������
				// �α��� ���� ������ ������� index�� �־���.
				// ���� ������ �ٷ� �α����� �ؾߵǴ� ����� LoginUser ���Ŀ� ID�� PW�� �޾Ƽ� ������ ������ ���̴�.
				// ������ �װ͵� ������ �Ҽȷα��� �� ������ �α����� ó���� ���� ��ū�� ���� ������ ������ �޾ƿͼ� ��ū���� �����ϰ� �����Ұŷ� �����ȴ�.
				// �׷��� �ȴٸ� ���⿡ Ŭ���̾�Ʈ �����ε����� ����Ǵ� �� �ܿ� ��ū���� ���� ���� �� ���̴�.

				// ���� ���, �н����꿢���� ���� ��쿡�� �α��ι�ư�� �ѹ� �� �־ ������ �ѹ� �� ��ģ��.
				// ������ ������ �� ���� �� ��쿡 �α����� ������ �����߱� ������ ������ �־ ����ȭ������ ƨ��� �Ǹ� �α��� ��ư�� ������ �� ���ᰡ �ƴٰ� �ٽ� �α����� �����϶�� ���´�.
				m_pPacketManager->LoginUser(pClientInfo->GetIndex());
			}
			else
			{
				CloseSocket(pClientInfo, true);
			}
			break;

			// Recv ���
		case IOOperation::RECV:

			OnReceive(pClientInfo->GetIndex(), dwIoSize);

			// ��Ŷ �Ŵ����� ��Ŷ ���� ����
			m_pPacketManager->ReceivePacketData(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuffer());

			// �ѹ� �޾����� ���˴�ó�� Recv�� �ٽ� ������ߵȴ�.
			// ���� �������� ������ ���� �� �޴´�.
			BindRecv(pClientInfo);
			break;

			// Send ���
			// �������� ���� ��	���⸸ �ϴ°��̴� �Ѿ
		case IOOperation::SEND:
			cout << "[�۽�] bytes: " << dwIoSize << endl;
			break;

			// ���� ó��
		default:
			cout << "socket(" << (int)pClientInfo->GetSocket() << ")���� ���ܻ�Ȳ" << endl;
			break;
		}
	}
}

// ������� ������ �ٽ� �ް��ϴ� ������
void NetworkCore::AcceptThread()
{
	while (mIsAcceptRun)
	{
		// ���� �ð��� �ʴ����� �ޱ�
		UINT64 curTimeSec = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();

		// �� Ŭ���̾�Ʈ ��ȸ
		for (auto client : mClientInfos)
		{
			// ���� �Ǿ������� �Ѿ��
			if (client->IsConnected())
				continue;

			// ���� �ð��� �ֱ� ���� �ð����� ���ٸ� (ó�� �ʱ�ȭ�� ������ �ƹ��� �� ���� ���)
			if ((UINT64)curTimeSec < client->GetLatestClosedTimeSec())
				continue;

			// ���� ���
			auto diff = curTimeSec - client->GetLatestClosedTimeSec();

			// ���̰� ������ ����ð����� ���� ��� (���� ���� �ð��� ���� ���� ���)
			if (diff <= RE_USE_SESSION_WAIT_TIMESEC)
				continue;

			// AcceptEx ����
			PostAccept(client, curTimeSec);
		}

		this_thread::sleep_for(32ms);
	}
}

// AcceptEx
bool NetworkCore::PostAccept(ClientInfo* pClientInfo, const UINT64 curTimeSec)
{
	cout << "PostAccept. Client index: " << pClientInfo->GetIndex() << endl;

	// ù ������ ��, �ð��� �ƽ������� �ʱ�ȭ
	pClientInfo->SetLatestClosedTimeSec(UINT32_MAX);

	SOCKET newSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (newSocket == INVALID_SOCKET)
	{
		cout << "Client socket WSASocket error: " << WSAGetLastError() << endl;
		return false;
	}

	pClientInfo->SetSocket(newSocket);

	DWORD bytes = 0;
	DWORD flags = 0;
	pClientInfo->m_stAcceptOverlappedEx.m_wsaBuf.len = 0;
	pClientInfo->m_stAcceptOverlappedEx.m_wsaBuf.buf = nullptr;
	pClientInfo->m_stAcceptOverlappedEx.m_eOperation = IOOperation::ACCEPT;
	pClientInfo->m_stAcceptOverlappedEx.m_sessionIndex = pClientInfo->GetIndex();

	if(AcceptEx(mListenSocket, pClientInfo->GetSocket(), pClientInfo->mAcceptBuf, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) & (pClientInfo->m_stAcceptOverlappedEx)) == FALSE)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "AcceptEx Error: " << WSAGetLastError() << endl;
			return false;
		}
	}

	return true;
}

// Accept ���� ����
bool NetworkCore::AcceptCompletion(ClientInfo* pClientInfo)
{
	cout << "AcceptCompletion: SessionIndex(" << pClientInfo->GetIndex() << ")" << endl;

	// ���� ���� O
	pClientInfo->SetConnect(true);

	// ����� ������ IOCP ��ü�� ����
	if (BindIoCompletionPort(pClientInfo) == false)
	{
		return false;
	}

	// WSARecv
	if (BindRecv(pClientInfo) == false)
	{
		return false;
	}

	// ���� � IP�� �����ߴ��� �����ֱ� ���� ���
	// ���� ��������ٸ� ���� ����� DB�� ��Ƶδ� �۾��� �������� ��
	// ���ͼ� ���� ���ɼ��� �� �� ���� ��
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
	cout << "Ŭ�� ����: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->GetSocket() << ")" << endl;

	return true;
}

// ���� ���� ����
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	unsigned int clientIndex = pClientInfo->GetIndex();

	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER�� ������

	// bIsForce�� true�� SO_LINGER, timeout = 0���� �����ؼ� ���� ����. ������ �ս� �� �� ����
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// ������ send recv ���� �ߴ�
	shutdown(pClientInfo->GetSocket(), SD_BOTH);

	// ���� �ɼ��� �����ص� linger�� ����
	setsockopt(pClientInfo->GetSocket(), SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	pClientInfo->SetConnect(false);
	pClientInfo->SetLatestClosedTimeSec(chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count());

	// ���� ����
	closesocket(pClientInfo->GetSocket());

	// ������ �ʿ�������ٰ� ������� �����ϰ� ���ġ�ϴٰ� ���߿� ū���� �� �� ����.
	// ������ pool�� �뵵�� Ȱ���ϱ� ���Ͽ� ���ϸ� �׿���
	pClientInfo->SetSocket(INVALID_SOCKET);

	OnClose(clientIndex);

	// ��Ŷ �Ŵ����� ���� ���� �α׾ƿ� -> �ٽ� �ʱ�ȭ��
	m_pPacketManager->LogoutUser(clientIndex);
}
