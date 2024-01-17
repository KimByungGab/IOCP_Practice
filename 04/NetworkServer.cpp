#include "NetworkServer.h"

// 연결 이벤트
void NetworkServer::OnConnect(const unsigned int clientIndex)
{
	cout << "[OnConnect] 인덱스: " << clientIndex << endl;
}

// 소켓 해제 이벤트
void NetworkServer::OnClose(const unsigned int clientIndex)
{
	cout << "[OnClose] 인덱스: " << clientIndex << endl;
}

// recv 이벤트
void NetworkServer::OnReceive(const unsigned int clientIndex, const unsigned int size, char* pData)
{
	cout << "[OnReceive] 인덱스: " << clientIndex << " / 데이터크기: " << size;

	// 패킷에 정보 넣어서 send 전담 큐에 삽입
	Packet packet;
	packet.Set(clientIndex, size, pData);

	lock_guard<mutex> guard(mLock);
	mPacketQueue.push_back(packet);
}

// 서버 구동
void NetworkServer::Run()
{
	// send 쓰레드 활성화
	mIsRunProcessThread = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});

	StartServer();
}

// 서버 종료
void NetworkServer::End()
{
	// send 쓰레드 비활성화
	mIsRunProcessThread = false;

	// send 쓰레드가 작동중이라면 종료
	if (mProcessThread.joinable())
		mProcessThread.join();

	// 네트워크 관련 쓰레드 전부 종료
	DestroyThread();
}

// 패킷 프로세스
void NetworkServer::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		// 가장 최근의 패킷 전달받기
		Packet packet = DequePacketData();

		// 패킷이 존재한다면
		if (packet.mDataSize != 0)
		{
			SendMsg(GetClientInfo(packet.mSessionIndex), packet.m_pPacketData, packet.mDataSize);
		}
		else
		{
			this_thread::sleep_for(1ms);
		}
	}
}

// 패킷 데이터 받기
Packet NetworkServer::DequePacketData()
{
	// 패킷
	Packet packet;

	// 락가드 적용
	lock_guard<mutex> guard(mLock);

	// 패킷 큐가 비었다면
	if (mPacketQueue.empty())
	{
		return packet;	// 빈 패킷 그대로 보내기
	}

	// 패킷 정보 삽입
	packet.Set(mPacketQueue.front());

	// 적용된 패킷은 필요없으므로 꺼내기
	mPacketQueue.front().Release();
	mPacketQueue.pop_front();

	return packet;
}
