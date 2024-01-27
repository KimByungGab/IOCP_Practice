#pragma once

#include <Windows.h>
#include <map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include <mutex>

using namespace std;

struct st_room
{
	UINT32 roomNumber;
	list<UINT32> roomMembers;
};

class RoomManager
{
public:
	RoomManager() = default;
	~RoomManager() = default;

	UINT32 CreateRoom();
	bool EnterRoom(UINT32 clientIndex, UINT32 roomNumber);
	bool LeaveRoom(UINT32 clientIndex, UINT32 roomNumber);
	list<UINT32> GetMember(UINT32 roomNumber);

private:
	map<UINT32, st_room> rooms;		// 현재 만들어진 방 정보
	mutex mLock;					// 락
};