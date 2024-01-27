#include "NetworkCore.h"

NetworkCore::NetworkCore()
{

}

NetworkCore::~NetworkCore()
{
	WSACleanup();
}

// 윈속 및 listen socket 초기화
bool NetworkCore::InitSocket()
{
	// 윈속 초기화
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "WSAStartup() error: " << WSAGetLastError() << endl;
		return false;
	}

	// listen socket 초기화
	// IOCP를 사용하기 위해 WSA_FLAG_OVERLAPPED 플래그 사용
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (mListenSocket == INVALID_SOCKET)
	{
		cout << "socket() error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "소켓 초기화 성공" << endl;
	return true;
}

// 서버 정보 bind 후 소켓 listen
bool NetworkCore::BindAndListen()
{
	// 서버 정보
	// 받는 IP는 모두 받기
	// 블랙리스트는 DB에서 가져와 처리하면 될 듯
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// 소켓 bind
	int nRet = ::bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (nRet != 0)
	{
		cout << "bind() error: " << WSAGetLastError() << endl;
		return false;
	}

	// 소켓 listen, 연결 큐는 최대길이로
	nRet = listen(mListenSocket, SOMAXCONN);
	if (nRet != 0)
	{
		cout << "listen() error: " << WSAGetLastError() << endl;
		return false;
	}

	// listen에서 accept 받지 못하게 하기
	BOOL on = true;
	setsockopt(mListenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(on));

	// IOCP 초기화, 쓰레드 개수 미리 선정
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	// listen socket에 IOCP 핸들 연결
	auto hIOCPHandle = CreateIoCompletionPort((HANDLE)mListenSocket, mIOCPHandle, (UINT32)0, 0);
	if (hIOCPHandle == nullptr)
	{
		cout << "listen socket IOCP bind error: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "서버 등록 성공" << endl;
	return true;
}

// 서버 구동
bool NetworkCore::StartServer()
{
	// 패킷 매니저 세팅
	m_pPacketManager = make_unique<PacketManager>();
	m_pPacketManager->SendPacketFunc = [&](UINT32 clientIndex, UINT16 packetSize, char* pSendPacket)
		{
			SendMsg(GetClientInfo(clientIndex), pSendPacket, packetSize);
		};
	m_pPacketManager->Init(MAX_CLIENT);
	m_pPacketManager->Run();

	// 클라이언트 풀 생성
	CreateClient(MAX_CLIENT);

	// 워커 쓰레드 생성
	bool bRet = CreateWorkerThread();
	if (bRet == false)
		return false;

	// Accept 하는 쓰레드 생성
	bRet = CreateAcceptThread();
	if (bRet == false)
		return false;

	cout << "서버 시작" << endl;
	return true;
}

// 생성되어있는 쓰레드 모조리 소멸
void NetworkCore::DestroyThread()
{
	// 패킷 매니저 종료
	m_pPacketManager->End();

	// 워커 쓰레드 반복문 종료
	mIsWorkerRun = false;

	// IOCP 객체 핸들러 종료
	CloseHandle(mIOCPHandle);

	// 워커 쓰레드 전부 join하여 종료
	for (auto& t : mIOWorkerThreads)
	{
		if (t.joinable())
			t.join();
	}

	//Accept 쓰레드 반복문 종료
	mIsAcceptRun = false;

	// listen socket 닫기
	closesocket(mListenSocket);

	// Accept 쓰레드 전부 join하여 종료
	if (mAcceptThread.joinable())
		mAcceptThread.join();
}

// 클라이언트 풀 생성
void NetworkCore::CreateClient(const UINT32 maxClientCount)
{
	for (UINT32 i = 0; i < maxClientCount; i++)
	{
		ClientInfo* client = new ClientInfo();
		client->SetIndex(i);

		mClientInfos.push_back(client);
	}
}

// 워커 쓰레드 생성
bool NetworkCore::CreateWorkerThread()
{
	mIsWorkerRun = true;

	// 워커 쓰레드 생성
	// MSDN에서 말한 쓰레드 권장개수는 '하드웨어 코어 수 * 2 + 1'
	// 이유: 쓰레드가 Suspend되어 대기상태로 빠지게 될 경우 새로운 쓰레드를 꺼내기 위함
	for (int i = 0; i < mMaxWorkerThread; i++)
	{
		mIOWorkerThreads.push_back(thread([this]()
			{
				WorkerThread();
			}));
	}

	cout << "WorkerThread 시작" << endl;

	return true;
}

// Accept 쓰레드 생성
bool NetworkCore::CreateAcceptThread()
{
	mIsAcceptRun = true;
	mAcceptThread = thread([this]()
		{
			AcceptThread();
		});

	cout << "AcceptThread 시작" << endl;
	return true;
}

// 사용하지 않는 클라이언트 구조체 반환
// 사용하지 않는 클라이언트 구조체가 하나라도 있으면 그 구조체를 반환
ClientInfo* NetworkCore::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (client->IsConnected() == false)
			return client;
	}

	return nullptr;
}

// 인덱스로 클라이언트 정보 구조체 반환
ClientInfo* NetworkCore::GetClientInfo(const unsigned int sessionIdx)
{
	return mClientInfos[sessionIdx];
}

// Completion Port 객체와 소켓과 Completion Key를 연결
// 즉 IOCP 진짜 시작
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// socket과 pClientInfo를 Completion Port 객체와 연결
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->GetSocket(), mIOCPHandle, (ULONG_PTR)(pClientInfo), 0);

	if (hIOCP == INVALID_HANDLE_VALUE)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// WSARecv Overlapped I/O 작업
bool NetworkCore::BindRecv(ClientInfo* pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// Overlapped I/O를 위해 정보 세팅
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

	// socket error면 client socket 끊어진거로 간주
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSARecv() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// WSASend Overlapped I/O 작업
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, const char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// 전송될 메시지 복사
	CopyMemory(pClientInfo->mSendBuf, pMsg, nLen);

	// Overlapped I/O를 위해 정보 세팅
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

	// socket error면 client socket 끊어진거로 간주
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		cout << "WSASend() error: " << WSAGetLastError() << endl;
		return false;
	}

	return true;
}

// Overlapped I/O 작업에 대한 완료 통보를 받아 해당 처리를 하는 함수
void NetworkCore::WorkerThread()
{
	ClientInfo* pClientInfo = nullptr;			// Completion Key를 받을 포인터 변수
	BOOL bSuccess = TRUE;						// 함수 호출 성공 여부
	DWORD dwIoSize = 0;							// Overlapped I/O 작업에서 전송된 데이터 크기
	LPOVERLAPPED lpOverlapped = nullptr;		// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터

	// 이 함수로 인해 쓰레드들은 Waiting Thread Queue에 대기상태로 들어감
	// 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue에서 완료된 작업을 가져와 나머지 처리
	// PostQueuedCompletionStatus() 함수에 의해 사용자 메시지가 도착되면 쓰레드를 종료
	while (mIsWorkerRun)
	{
		// 매개변수 정리
		// 1) IOCP 핸들러
		// 2) 실제로 전송된 바이트
		// 3) Completion Key
		// 4) Overlapped I/O 객체
		// 5) 타임아웃 시간
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle, &dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);

		// 사용자 쓰레드 종료 메시지 처리
		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == nullptr)
		{
			mIsWorkerRun = false;
			continue;
		}

		// I/O 정보가 없을때
		// 원래는 overlapped가 nullptr이라든가 데이터 크기(dwIoSize)가 이상하다면 연결을 종료시킨다고 함.
		// 참고: https://www.slideshare.net/namhyeonuk90/iocp
		if (lpOverlapped == nullptr)
		{
			continue;
		}

		// overlapped 정보
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// 클라가 접속을 끊음
		if (bSuccess == FALSE || (dwIoSize == 0 && (pOverlappedEx->m_eOperation != IOOperation::ACCEPT)))
		{
			cout << "클라 접속 종료! " << (int)pClientInfo->GetSocket() << endl;
			CloseSocket(pClientInfo);
			continue;
		}

		// Overlapped I/O Recv 작업 결과 처리
		switch (pOverlappedEx->m_eOperation)
		{
			// Accept 라면
		case IOOperation::ACCEPT:
			pClientInfo = GetClientInfo(pOverlappedEx->m_sessionIndex);
			if (AcceptCompletion(pClientInfo))
			{
				OnConnect(pClientInfo->mIndex);

				// 클라 접속 개수 증가
				mClientCnt++;

				// 로그인 했다고 정보 주기
				// 여기서는 유저가 들어온 이후에 세팅을 하는 것이므로 자유이다.
				// 하지만 나는 유저의 개인적인 정보를 넣지 않았기 때문에
				// 로그인 유저 정보에 상관없이 index만 넣었다.
				// 만약 실제로 바로 로그인을 해야되는 경우라면 LoginUser 이후에 ID와 PW를 받아서 인증을 거쳤을 것이다.
				// 하지만 그것도 요즘은 소셜로그인 등 웹에서 로그인을 처리한 이후 토큰과 같이 인증할 수단을 받아와서 토큰값을 저장하고 진행할거로 생각된다.
				// 그렇게 된다면 여기에 클라이언트 정보인덱스가 연결되는 것 외에 토큰값도 같이 들어가게 될 것이다.

				// 예를 들어, 패스오브엑자일 같은 경우에는 로그인버튼이 한번 더 있어서 인증을 한번 더 거친다.
				// 때문에 접속한 지 오래 된 경우에 로그인은 웹에서 진행했기 때문에 오류가 있어서 메인화면으로 튕기게 되면 로그인 버튼을 눌렀을 때 만료가 됐다고 다시 로그인을 진행하라고 나온다.
				m_pPacketManager->LoginUser(pClientInfo->GetIndex());
			}
			else
			{
				CloseSocket(pClientInfo, true);
			}
			break;

			// Recv 라면
		case IOOperation::RECV:

			OnReceive(pClientInfo->GetIndex(), dwIoSize);

			// 패킷 매니저에 패킷 정보 삽입
			m_pPacketManager->ReceivePacketData(pClientInfo->GetIndex(), dwIoSize, pClientInfo->GetRecvBuffer());

			// 한번 받았으면 낚싯대처럼 Recv는 다시 던져줘야된다.
			// 만약 던져주지 않으면 영영 못 받는다.
			BindRecv(pClientInfo);
			break;

			// Send 라면
			// 서버에서 단지 보	내기만 하는것이니 넘어감
		case IOOperation::SEND:
			cout << "[송신] bytes: " << dwIoSize << endl;
			break;

			// 예외 처리
		default:
			cout << "socket(" << (int)pClientInfo->GetSocket() << ")에서 예외상황" << endl;
			break;
		}
	}
}

// 사용자의 접속을 다시 받게하는 쓰레드
void NetworkCore::AcceptThread()
{
	while (mIsAcceptRun)
	{
		// 현재 시간을 초단위로 받기
		UINT64 curTimeSec = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();

		// 각 클라이언트 순회
		for (auto client : mClientInfos)
		{
			// 연결 되어있으면 넘어가기
			if (client->IsConnected())
				continue;

			// 현재 시간이 최근 종료 시간보다 적다면 (처음 초기화한 값에서 아무도 안 들어온 경우)
			if ((UINT64)curTimeSec < client->GetLatestClosedTimeSec())
				continue;

			// 차이 계산
			auto diff = curTimeSec - client->GetLatestClosedTimeSec();

			// 차이가 지정한 재사용시간보다 적을 경우 (아직 재사용 시간을 넘지 못한 경우)
			if (diff <= RE_USE_SESSION_WAIT_TIMESEC)
				continue;

			// AcceptEx 실행
			PostAccept(client, curTimeSec);
		}

		this_thread::sleep_for(32ms);
	}
}

// AcceptEx
bool NetworkCore::PostAccept(ClientInfo* pClientInfo, const UINT64 curTimeSec)
{
	cout << "PostAccept. Client index: " << pClientInfo->GetIndex() << endl;

	// 첫 세팅할 때, 시간을 맥스값으로 초기화
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

// Accept 진행 과정
bool NetworkCore::AcceptCompletion(ClientInfo* pClientInfo)
{
	cout << "AcceptCompletion: SessionIndex(" << pClientInfo->GetIndex() << ")" << endl;

	// 연결 여부 O
	pClientInfo->SetConnect(true);

	// 연결된 소켓을 IOCP 객체와 연결
	if (BindIoCompletionPort(pClientInfo) == false)
	{
		return false;
	}

	// WSARecv
	if (BindRecv(pClientInfo) == false)
	{
		return false;
	}

	// 단지 어떤 IP가 접속했는지 보여주기 위한 출력
	// 만약 운영서버였다면 접속 기록을 DB에 담아두는 작업을 진행했을 듯
	// 나와서 했을 가능성이 좀 더 높을 듯
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
	cout << "클라 접속: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->GetSocket() << ")" << endl;

	return true;
}

// 소켓 연결 종료
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	unsigned int clientIndex = pClientInfo->GetIndex();

	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER로 설정함

	// bIsForce가 true면 SO_LINGER, timeout = 0으로 설정해서 강제 종료. 데이터 손실 될 수 있음
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// 소켓의 send recv 전부 중단
	shutdown(pClientInfo->GetSocket(), SD_BOTH);

	// 소켓 옵션을 지정해둔 linger로 설정
	setsockopt(pClientInfo->GetSocket(), SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	pClientInfo->SetConnect(false);
	pClientInfo->SetLatestClosedTimeSec(chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count());

	// 소켓 종료
	closesocket(pClientInfo->GetSocket());

	// 소켓이 필요없어졌다고 마음대로 삭제하고 재배치하다간 나중에 큰일이 날 수 있음.
	// 때문에 pool의 용도로 활용하기 위하여 소켓만 죽여둠
	pClientInfo->SetSocket(INVALID_SOCKET);

	OnClose(clientIndex);

	// 패킷 매니저에 유저 정보 로그아웃 -> 다시 초기화함
	m_pPacketManager->LogoutUser(clientIndex);
}
