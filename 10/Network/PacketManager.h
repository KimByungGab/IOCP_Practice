#pragma once

#include "Packet.h"
#include "RoomManager.h"
#include "UserManager.h"
#include "ErrorCode.h"

#include <iostream>
#include <utility>
#include <string>
#include <unordered_map>
#include <deque>
#include <functional>
#include <thread>
#include <mutex>

using namespace std;

class PacketManager
{
public:
	PacketManager() = default;
	~PacketManager() = default;

	void Init(const UINT32 maxClient);
	void Run();
	void EnqueuePacketData(const UINT32 clientIndex);
	void End();
	void ReceivePacketData(const UINT32 clientIndex, const UINT32 size, char* pData);
	void LoginUser(UINT32 clientIndex);
	void LogoutUser(UINT32 clientIndex);

public:
	function<void(UINT32, UINT32, char*)> SendPacketFunc;		// SendMsg �Լ� ������

private:
	PacketInfo DequePacketData();

	void ProcessPacket();
	void ProcessRecvPacket(const UINT32 clientIndex, const UINT16 packetId, const UINT16 packetSize, char* pPacket);

	void LogoutReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void CreateRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void EnterRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void LeaveRoomReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket);
	void ChatReq(UINT32 clientIndex, UINT16 packetSize, char* pPacket);

	typedef void(PacketManager::* PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT16, char*);
	unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> mRecvFunctionDictionary;		// �Լ� ������ ��ųʸ�

	deque<UINT32> mClientQueue;				// ��û�� ���� Ŭ���̾�Ʈ ť
	bool mIsRunProcessThread = false;		// ��Ŷ �Ŵ��� ������ onoff
	thread mProcessThread;					// ��Ŷ �Ŵ��� ������
	mutex mLock;							// ��

	RoomManager* mRoomManager;				// Room Manager ��ü
	UserManager* mUserManager;				// User Manager ��ü
};

