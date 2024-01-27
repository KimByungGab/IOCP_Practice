#include "PacketManager.h"

// 패킷매니저 초기화
void PacketManager::Init(const UINT32 maxClient)
{
	// 딕셔너리에 각 함수 넣기
	// 순서는 상관없고 해시테이블의 역할을 해서 O(1)의 효과를 보고싶기 때문에 unordered_map
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_CREATE_REQUEST] = &PacketManager::CreateRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_ENTER_REQUEST] = &PacketManager::EnterRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_LEAVE_REQUEST] = &PacketManager::LeaveRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_CHAT_REQUEST] = &PacketManager::ChatReq;
	mRecvFunctionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::LogoutReq;

	// 룸 매니저 객체
	mRoomManager = new RoomManager();

	// 유저 매니저 객체
	// 일반적으로 불리는 세션의 역할이다.
	mUserManager = new UserManager();
	mUserManager->Init(maxClient);
}

// 실행
void PacketManager::Run()
{
	mIsRunProcessThread = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});
}

// 종료
void PacketManager::End()
{
	mIsRunProcessThread = false;

	if (mProcessThread.joinable())
		mProcessThread.join();
}

// 패킷 데이터 받기
void PacketManager::ReceivePacketData(const UINT32 clientIndex, const UINT32 size, char* pData)
{
	// 세션 정보 확인 후 세션에 패킷넣기
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);
	pUser->SetPacketData(size, pData);

	// 세션 인덱스 큐에 넣기
	EnqueuePacketData(clientIndex);
}

// 세션 인덱스 큐 삽입
void PacketManager::EnqueuePacketData(const UINT32 clientIndex)
{
	lock_guard<mutex> guard(mLock);
	mClientQueue.push_back(clientIndex);
}

// 큐에서 다음 기다리는 세션 인덱스의 패킷 가져오기
PacketInfo PacketManager::DequePacketData()
{
	// 다음 기다리는 세션 인덱스
	UINT32 clientIndex = 0;

	{
		lock_guard<mutex> guard(mLock);

		// 만약 기다리는 세션 인덱스가 없다면
		if (mClientQueue.empty())
		{
			return PacketInfo();
		}

		// 맨 앞의 세션 인덱스 정보 복사
		clientIndex = mClientQueue.front();

		// 클라이언트 큐에서 해당 정보 삭제
		mClientQueue.pop_front();
	}

	// 세션 인덱스로 세션 포인터 가져오기
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// 세션 인덱스의 현재 기다리는 패킷 가져오기
	auto packetData = pUser->GetPacket();
	packetData.ClientIndex = clientIndex;

	return packetData;
}

// 패킷 프로세스
void PacketManager::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		// 패킷 데이터 가져오기
		auto packetData = DequePacketData();

		// 만약 클라이언트가 보낸 명령이 SYS_END 이상의 명령이라면
		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			// 명령 실행
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}
		else
		{
			this_thread::sleep_for(1ms);
		}
	}
}

// 클라이언트가 보낸 명령 실행
void PacketManager::ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetId, const UINT16 packetSize, char* pPacket)
{
	// 명령에 관한 함수 찾기
	// map이기 때문에 알고리즘을 include하여 find를 사용했다.
	auto iter = mRecvFunctionDictionary.find(packetId);

	// 만약 도중에 찾았다면
	if (iter != mRecvFunctionDictionary.end())
	{
		// 해당 함수 실행
		// 사실 이 때 패킷 사이즈는 필요없다.
		(this->*(iter->second))(clientIndex, packetSize, pPacket);
	}
}

// 클라이언트 접속
void PacketManager::LoginUser(UINT32 clientIndex)
{
	cout << "[ProcessUserConnect] clientIndex: " << clientIndex << endl;

	// 클라이언트와 맵핑된 세션 불러오기
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// 세션 초기화
	pUser->Clear();
}

// 클라이언트 로그아웃
void PacketManager::LogoutUser(UINT32 clientIndex)
{	
	cout << "[ProcessUserDisconnect] clientIndex: " << clientIndex << endl;
	
	// 클라이언트와 맵핑된 세션 불러오기
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// 만약 로비에 있는 것이 아니었다면
	if (pUser->GetDomainState() == User::DOMAIN_STATE::ROOM)
	{
		// 방에서 나가기
		auto roomNum = pUser->GetCurrentRoom();
		mRoomManager->LeaveRoom(clientIndex, roomNum);
	}

	// 세션 초기화
	pUser->Clear();
}

// 로그아웃 요청
void PacketManager::LogoutReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	SYS_USER_DISCONNECT_RESPONSE_PACKET packet;
	packet.PacketId = (UINT16)PACKET_ID::SYS_USER_DISCONNECT;
	packet.PacketLength = sizeof(SYS_USER_DISCONNECT_RESPONSE_PACKET);
	
	SendPacketFunc(clientIndex, sizeof(SYS_USER_DISCONNECT_RESPONSE_PACKET), (char*)&packet);

	LogoutUser(clientIndex);
}

// 방 만들기 요청
void PacketManager::CreateRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pCreateRoomReqPacket = (ROOM_CREATE_REQUEST_PACKET*)pPacket;

	UINT32 roomIndex = mRoomManager->CreateRoom();

	ROOM_CREATE_RESPONSE_PACKET roomCreateResponsePacket;
	roomCreateResponsePacket.PacketId = (UINT16)PACKET_ID::ROOM_CREATE_RESPONSE;
	roomCreateResponsePacket.PacketLength = sizeof(ROOM_CREATE_RESPONSE_PACKET);
	roomCreateResponsePacket.RoomIndex = roomIndex;

	SendPacketFunc(clientIndex, sizeof(ROOM_CREATE_RESPONSE_PACKET), (char*)&roomCreateResponsePacket);
	
	ROOM_ENTER_REQUEST_PACKET enterPacket;
	enterPacket.RoomIndex = roomIndex;

	EnterRoomReq(clientIndex, 0, (char*)&enterPacket);
}

// 방 입장 요청
void PacketManager::EnterRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pEnterRoomReqPacket = (ROOM_ENTER_REQUEST_PACKET*)pPacket;
	UINT32 roomIndex = pEnterRoomReqPacket->RoomIndex;
	if (mRoomManager->EnterRoom(clientIndex, roomIndex))
	{
		ROOM_ENTER_RESPONSE_PACKET enterPacket;
		enterPacket.PacketId = (UINT16)PACKET_ID::ROOM_ENTER_RESPONSE;
		enterPacket.PacketLength = sizeof(ROOM_ENTER_RESPONSE_PACKET);

		SendPacketFunc(clientIndex, sizeof(ROOM_ENTER_RESPONSE_PACKET), (char*)&enterPacket);

		list<UINT32> members = mRoomManager->GetMember(roomIndex);
		string notifyStr = "[" + to_string(roomIndex) + "번방] " + to_string(clientIndex) + "번 님께서 입장하셨습니다.\0";
		for (UINT32 memberIndex : members)
		{
			ROOM_CHAT_RESPONSE_PACKET packet;
			packet.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
			packet.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
			memcpy(packet.chatData, notifyStr.c_str(), sizeof(notifyStr));
			
			SendPacketFunc(memberIndex, sizeof(ROOM_CHAT_RESPONSE_PACKET), (char*)&packet);
		}

		auto pUser = mUserManager->GetUserByConnIdx(clientIndex);
		pUser->EnterRoom(roomIndex);
	}
	else
	{
		SYS_ERROR cantEnterPacket;
		cantEnterPacket.PacketId = (UINT16)PACKET_ID::SYS_ERROR;
		cantEnterPacket.PacketLength = sizeof(SYS_ERROR);
		cantEnterPacket.errorCode = (unsigned int)ERROR_CODE::ENTER_ROOM_CANT_ENTER;

		SendPacketFunc(clientIndex, sizeof(SYS_ERROR), (char*)&cantEnterPacket);
	}
}

// 방 퇴장 요청
void PacketManager::LeaveRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pLeaveRoomReqPacket = (ROOM_LEAVE_REQUEST_PACKET*)pPacket;
	UINT32 roomIndex = pLeaveRoomReqPacket->RoomIndex;
	mRoomManager->LeaveRoom(clientIndex, roomIndex);

	ROOM_LEAVE_RESPONSE_PACKET packet;
	packet.PacketId = (UINT16)PACKET_ID::ROOM_LEAVE_RESPONSE;
	packet.PacketLength = sizeof(ROOM_LEAVE_RESPONSE_PACKET);

	SendPacketFunc(clientIndex, sizeof(ROOM_LEAVE_RESPONSE_PACKET), (char*)&packet);

	list<UINT32> members = mRoomManager->GetMember(roomIndex);
	string notifyStr = to_string(clientIndex) + "번 님께서 퇴장하셨습니다.\0";
	for (UINT32 memberIndex : members)
	{
		ROOM_CHAT_RESPONSE_PACKET packet;
		packet.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
		packet.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
		memcpy(packet.chatData, notifyStr.c_str(), sizeof(notifyStr));

		SendPacketFunc(memberIndex, sizeof(ROOM_CHAT_RESPONSE_PACKET), (char*)&packet);
	}

	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);
	pUser->EnterRoom(roomIndex);
}

// 채팅 요청
void PacketManager::ChatReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pChatReqPacket = (ROOM_CHAT_REQUEST_PACKET*)pPacket;
	
	UINT32 roomIndex = pChatReqPacket->RoomIndex;
	string chatData = pChatReqPacket->chatData;

	list<UINT32> members = mRoomManager->GetMember(roomIndex);
	string chatStr = "[" + to_string(clientIndex) + "번] " + chatData + "\0";
	for (UINT32 memberIndex : members)
	{
		ROOM_CHAT_RESPONSE_PACKET packet;
		packet.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
		packet.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
		memcpy(packet.chatData, chatStr.c_str(), sizeof(chatStr));

		SendPacketFunc(memberIndex, sizeof(ROOM_CHAT_RESPONSE_PACKET), (char*)&packet);
	}
}
