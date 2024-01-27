#include "PacketManager.h"

// ��Ŷ�Ŵ��� �ʱ�ȭ
void PacketManager::Init(const UINT32 maxClient)
{
	// ��ųʸ��� �� �Լ� �ֱ�
	// ������ ������� �ؽ����̺��� ������ �ؼ� O(1)�� ȿ���� ����ͱ� ������ unordered_map
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_CREATE_REQUEST] = &PacketManager::CreateRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_ENTER_REQUEST] = &PacketManager::EnterRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_LEAVE_REQUEST] = &PacketManager::LeaveRoomReq;
	mRecvFunctionDictionary[(int)PACKET_ID::ROOM_CHAT_REQUEST] = &PacketManager::ChatReq;
	mRecvFunctionDictionary[(int)PACKET_ID::SYS_USER_DISCONNECT] = &PacketManager::LogoutReq;

	// �� �Ŵ��� ��ü
	mRoomManager = new RoomManager();

	// ���� �Ŵ��� ��ü
	// �Ϲ������� �Ҹ��� ������ �����̴�.
	mUserManager = new UserManager();
	mUserManager->Init(maxClient);
}

// ����
void PacketManager::Run()
{
	mIsRunProcessThread = true;
	mProcessThread = thread([this]()
		{
			ProcessPacket();
		});
}

// ����
void PacketManager::End()
{
	mIsRunProcessThread = false;

	if (mProcessThread.joinable())
		mProcessThread.join();
}

// ��Ŷ ������ �ޱ�
void PacketManager::ReceivePacketData(const UINT32 clientIndex, const UINT32 size, char* pData)
{
	// ���� ���� Ȯ�� �� ���ǿ� ��Ŷ�ֱ�
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);
	pUser->SetPacketData(size, pData);

	// ���� �ε��� ť�� �ֱ�
	EnqueuePacketData(clientIndex);
}

// ���� �ε��� ť ����
void PacketManager::EnqueuePacketData(const UINT32 clientIndex)
{
	lock_guard<mutex> guard(mLock);
	mClientQueue.push_back(clientIndex);
}

// ť���� ���� ��ٸ��� ���� �ε����� ��Ŷ ��������
PacketInfo PacketManager::DequePacketData()
{
	// ���� ��ٸ��� ���� �ε���
	UINT32 clientIndex = 0;

	{
		lock_guard<mutex> guard(mLock);

		// ���� ��ٸ��� ���� �ε����� ���ٸ�
		if (mClientQueue.empty())
		{
			return PacketInfo();
		}

		// �� ���� ���� �ε��� ���� ����
		clientIndex = mClientQueue.front();

		// Ŭ���̾�Ʈ ť���� �ش� ���� ����
		mClientQueue.pop_front();
	}

	// ���� �ε����� ���� ������ ��������
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// ���� �ε����� ���� ��ٸ��� ��Ŷ ��������
	auto packetData = pUser->GetPacket();
	packetData.ClientIndex = clientIndex;

	return packetData;
}

// ��Ŷ ���μ���
void PacketManager::ProcessPacket()
{
	while (mIsRunProcessThread)
	{
		// ��Ŷ ������ ��������
		auto packetData = DequePacketData();

		// ���� Ŭ���̾�Ʈ�� ���� ����� SYS_END �̻��� ����̶��
		if (packetData.PacketId > (UINT16)PACKET_ID::SYS_END)
		{
			// ��� ����
			ProcessRecvPacket(packetData.ClientIndex, packetData.PacketId, packetData.DataSize, packetData.pDataPtr);
		}
		else
		{
			this_thread::sleep_for(1ms);
		}
	}
}

// Ŭ���̾�Ʈ�� ���� ��� ����
void PacketManager::ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetId, const UINT16 packetSize, char* pPacket)
{
	// ��ɿ� ���� �Լ� ã��
	// map�̱� ������ �˰����� include�Ͽ� find�� ����ߴ�.
	auto iter = mRecvFunctionDictionary.find(packetId);

	// ���� ���߿� ã�Ҵٸ�
	if (iter != mRecvFunctionDictionary.end())
	{
		// �ش� �Լ� ����
		// ��� �� �� ��Ŷ ������� �ʿ����.
		(this->*(iter->second))(clientIndex, packetSize, pPacket);
	}
}

// Ŭ���̾�Ʈ ����
void PacketManager::LoginUser(UINT32 clientIndex)
{
	cout << "[ProcessUserConnect] clientIndex: " << clientIndex << endl;

	// Ŭ���̾�Ʈ�� ���ε� ���� �ҷ�����
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// ���� �ʱ�ȭ
	pUser->Clear();
}

// Ŭ���̾�Ʈ �α׾ƿ�
void PacketManager::LogoutUser(UINT32 clientIndex)
{	
	cout << "[ProcessUserDisconnect] clientIndex: " << clientIndex << endl;
	
	// Ŭ���̾�Ʈ�� ���ε� ���� �ҷ�����
	auto pUser = mUserManager->GetUserByConnIdx(clientIndex);

	// ���� �κ� �ִ� ���� �ƴϾ��ٸ�
	if (pUser->GetDomainState() == User::DOMAIN_STATE::ROOM)
	{
		// �濡�� ������
		auto roomNum = pUser->GetCurrentRoom();
		mRoomManager->LeaveRoom(clientIndex, roomNum);
	}

	// ���� �ʱ�ȭ
	pUser->Clear();
}

// �α׾ƿ� ��û
void PacketManager::LogoutReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	SYS_USER_DISCONNECT_RESPONSE_PACKET packet;
	packet.PacketId = (UINT16)PACKET_ID::SYS_USER_DISCONNECT;
	packet.PacketLength = sizeof(SYS_USER_DISCONNECT_RESPONSE_PACKET);
	
	SendPacketFunc(clientIndex, sizeof(SYS_USER_DISCONNECT_RESPONSE_PACKET), (char*)&packet);

	LogoutUser(clientIndex);
}

// �� ����� ��û
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

// �� ���� ��û
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
		string notifyStr = "[" + to_string(roomIndex) + "����] " + to_string(clientIndex) + "�� �Բ��� �����ϼ̽��ϴ�.\0";
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

// �� ���� ��û
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
	string notifyStr = to_string(clientIndex) + "�� �Բ��� �����ϼ̽��ϴ�.\0";
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

// ä�� ��û
void PacketManager::ChatReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket)
{
	auto pChatReqPacket = (ROOM_CHAT_REQUEST_PACKET*)pPacket;
	
	UINT32 roomIndex = pChatReqPacket->RoomIndex;
	string chatData = pChatReqPacket->chatData;

	list<UINT32> members = mRoomManager->GetMember(roomIndex);
	string chatStr = "[" + to_string(clientIndex) + "��] " + chatData + "\0";
	for (UINT32 memberIndex : members)
	{
		ROOM_CHAT_RESPONSE_PACKET packet;
		packet.PacketId = (UINT16)PACKET_ID::ROOM_CHAT_RESPONSE;
		packet.PacketLength = sizeof(ROOM_CHAT_RESPONSE_PACKET);
		memcpy(packet.chatData, chatStr.c_str(), sizeof(chatStr));

		SendPacketFunc(memberIndex, sizeof(ROOM_CHAT_RESPONSE_PACKET), (char*)&packet);
	}
}
