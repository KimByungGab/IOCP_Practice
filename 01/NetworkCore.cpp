#include "NetworkCore.h"

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
	int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
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

	return true;
}

// 서버 구동
bool NetworkCore::StartServer()
{
	// 클라이언트 풀 생성
	CreateClient(MAX_CLIENT);

	// IOCP 초기화, 쓰레드 개수 미리 선정
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, mMaxWorkerThread);
	if (mIOCPHandle == nullptr)
	{
		cout << "CreateIoCompletionPort() error: " << WSAGetLastError() << endl;
		return false;
	}

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
void NetworkCore::DestoryThread()
{
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
		mClientInfos.push_back(move(ClientInfo()));
	}
}

// 워커 쓰레드 생성
bool NetworkCore::CreateWorkerThread()
{
	// unsigned int uiThreadId = 0;

	// 워커 쓰레드 생성
	// MSDN에서 말한 쓰레드 권장개수는 '하드웨어 쓰레드 수 * 2 + 1'
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
		if (client.m_socketClient == INVALID_SOCKET)
			return &client;
	}

	return nullptr;
}

// Completion Port 객체와 소켓과 Completion Key를 연결
// 즉 IOCP 진짜 시작
bool NetworkCore::BindIoCompletionPort(ClientInfo* pClientInfo)
{
	// socket과 pClientInfo를 Completion Port 객체와 연결
	HANDLE hIOCP = CreateIoCompletionPort((HANDLE)pClientInfo->m_socketClient, mIOCPHandle, (ULONG_PTR)pClientInfo, 0);

	if (hIOCP == NULL || mIOCPHandle != hIOCP)
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
	pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = pClientInfo->m_stRecvOverlappedEx.m_szBuf;
	pClientInfo->m_stRecvOverlappedEx.m_eOperation = IOOperation::RECV;

	int nRet = WSARecv(pClientInfo->m_socketClient,
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
bool NetworkCore::SendMsg(ClientInfo* pClientInfo, char* pMsg, int nLen)
{
	DWORD dwRecvNumBytes = 0;

	// 전송될 메시지 복사
	CopyMemory(pClientInfo->m_stSendOverlappedEx.m_szBuf, pMsg, nLen);

	// Overlapped I/O를 위해 정보 세팅
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
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
			&dwIoSize, (PULONG_PTR)&pClientInfo, &lpOverlapped, INFINITE);

		// 클라가 쓰레드 종료 메시지 처리
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

		// 클라가 접속을 끊음
		if (bSuccess == FALSE || (bSuccess == TRUE && dwIoSize == 0))
		{
			cout << "클라 접속 종료! " << (int)pClientInfo->m_socketClient << endl;
			CloseSocket(pClientInfo);
			continue;
		}

		// overlapped 정보
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		// Overlapped I/O Recv 작업 결과 처리
		switch (pOverlappedEx->m_eOperation)
		{
			// Recv 라면
		case IOOperation::RECV:
			pOverlappedEx->m_szBuf[dwIoSize] = NULL;
			cout << "[수신] bytes: " << dwIoSize << ", msg: " << pOverlappedEx->m_szBuf << endl;

			// ======================================
			// 패킷에 따라 원하는 기능 수행하는 영역 시작
			// TODO: 에코가 아니라면 이제 여기서 패킷 분석하고 원하는 작업을 수행
			// 클라에게 메시지 에코
			SendMsg(pClientInfo, pOverlappedEx->m_szBuf, dwIoSize);
			// 패킷에 따라 원하는 기능 수행하는 영역 끝
			// ======================================

			// 한번 받았으면 낚싯대처럼 Recv는 다시 던져줘야된다.
			// 만약 던져주지 않으면 영영 못 받는다.
			BindRecv(pClientInfo);
			break;

			// Send 라면
			// 서버에서 단지 보내기만 하는것이니 넘어감
		case IOOperation::SEND:
			cout << "[송신] bytes: " << dwIoSize << ", msg: " << pOverlappedEx->m_szBuf << endl;
			break;

			// 예외 처리
		default:
			cout << "socket(" << (int)pClientInfo->m_socketClient << ")에서 예외상황" << endl;
			break;
		}
	}
}

// 사용자의 접속을 받는 쓰레드
void NetworkCore::AcceptThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);

	while (mIsAcceptRun)
	{
		// 접속받을 클라 구조체의 인덱스를 가져오기
		ClientInfo* pClientInfo = GetEmptyClientInfo();

		// 남은 클라 구조체가 없으면 return
		// 사실 여기서 만약 남은 구조체가 없으면 클라 구조체 vector(mClientInfos)에 넘기는 게 서버의 운영상으로는 좋은 듯.
		if (pClientInfo == nullptr)
		{
			cout << "Client Full" << endl;
			return;
		}

		// 클라 접속 요청이 들어올때까지 대기
		pClientInfo->m_socketClient = accept(mListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (pClientInfo->m_socketClient == INVALID_SOCKET)
			continue;

		// IOCP 객체와 소켓 연결
		bool bRet = BindIoCompletionPort(pClientInfo);
		if (bRet == false)
			return;

		// Recv Overlapped I/O 작업
		bRet = BindRecv(pClientInfo);
		if (bRet == false)
			return;

		// 단지 어떤 IP가 접속했는지 보여주기 위한 출력
		// 만약 운영서버였다면 접속 기록을 DB에 담아두는 작업을 진행했을 듯
		char clientIP[32] = { 0, };
		inet_ntop(AF_INET, (&stClientAddr.sin_addr), clientIP, 32 - 1);
		cout << "클라 접속: IP(" << clientIP << ") SOCKET(" << (int)pClientInfo->m_socketClient << ")" << endl;

		// 클라 접속 개수 증가
		mClientCnt++;
	}
}

// 소켓 연결 종료
void NetworkCore::CloseSocket(ClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0, 0 };		// SO_DONTLINGER로 설정함

	// bIsForce가 true면 SO_LINGER, timeout = 0으로 설정해서 강제 종료. 데이터 손실 될 수 있음
	if (bIsForce == true)
	{
		stLinger.l_onoff = 1;
	}

	// 소켓의 send recv 전부 중단
	shutdown(pClientInfo->m_socketClient, SD_BOTH);

	// 소켓 옵션을 지정해둔 linger로 설정
	setsockopt(pClientInfo->m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

	// 소켓 종료
	closesocket(pClientInfo->m_socketClient);

	// 소켓이 필요없어졌다고 마음대로 삭제하고 재배치하다간 나중에 큰일이 날 수 있음.
	// 때문에 pool의 용도로 활용하기 위하여 소켓만 죽여둠
	pClientInfo->m_socketClient = INVALID_SOCKET;
}

// mClientInfos(클라 vector)에 들어갈 클라 정보 초기화
ClientInfo::ClientInfo()
{
	memset(&m_stRecvOverlappedEx, 0, sizeof(stOverlappedEx));
	memset(&m_stSendOverlappedEx, 0, sizeof(stOverlappedEx));
	m_socketClient = INVALID_SOCKET;
}
