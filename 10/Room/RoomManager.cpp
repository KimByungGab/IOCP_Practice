#include "RoomManager.h"

// 방 생성
UINT32 RoomManager::CreateRoom()
{
	bool isMake = false;	// 방 생성 여부
	UINT32 roomNumber;		// 방 번호

	// 방이 생성될때까지 무한반복
	while (isMake == false)
	{
		roomNumber = rand();	// 방 번호는 랜덤

		// 중복되는지 방 번호 찾기
		auto iter = rooms.find(roomNumber);

		// 만약 못 찾았다면 생성
		if (iter == rooms.end())
		{
			st_room roomInfos{ roomNumber };

			lock_guard<mutex> guard(mLock);
			rooms.insert(make_pair(roomNumber, roomInfos));

			isMake = true;
		}
	}

	return roomNumber;
}

// 방 입장
bool RoomManager::EnterRoom(UINT32 clientIndex, UINT32 roomNumber)
{
	// 방 찾기
	auto iter = rooms.find(roomNumber);

	// 방 못 찾으면 안 됨
	if (iter == rooms.end())
		return false;

	// 해당 방에 멤버 삽입
	lock_guard<mutex> guard(mLock);
	iter->second.roomMembers.push_back(clientIndex);
	
	return true;
}

// 방 퇴장
bool RoomManager::LeaveRoom(UINT32 clientIndex, UINT32 roomNumber)
{
	// 방 찾기
	auto roomsIter = rooms.find(roomNumber);
	if (roomsIter == rooms.end())
		return false;

	// 유저 리스트 포인터
	list<UINT32>* pList = &(roomsIter->second.roomMembers);

	// 유저 리스트 포인터로 퇴장하는 사람 찾기
	auto memberIter = find(pList->begin(), pList->end(), clientIndex);
	if (memberIter == pList->end())
		return false;

	// 퇴장하는 사람 정보 삭제

	/*
	* 왜 erase? remove말고?
	* 
	* 2가지의 이유가 있다.
	* 
	* 여기서 가장 중요한 것은 정확한 유저들의 인덱스 정보와 개수이다.
	* 때문에 remove를 사용하게되면 컨테이너의 크기가 실제로 줄어드는 것이 아니고 삭제되어야 할 원소들의 위치에 유지될 원소들의 값들로 덮어씌우는것이다.
	* 그래서 뒤에는 쓸데없는 데이터가 남게 된다. 즉 진짜 없애는 게 아니라 데이터만 덮어씌우는것이다.
	* 하지만 그 상황은 GetMember같이 전체순회를 하게 되는것과 맞지가 않게 된다.
	* 
	* 두번째로는 리스트의 삽입과 삭제에 유리하기 때문이다.
	* 동적배열처럼 메모리가 연속적으로 저장되어있는 것이 아니고 메모리 주소를 링크를 다는 형식이기 때문에 링크만 조정하면 된다.
	* 
	* 때문에 erase를 택했다.
	*/
	lock_guard<mutex> guard(mLock);
	pList->erase(memberIter);
	if (pList->size() == 0)
		rooms.erase(roomsIter);

	return true;
}

// 해당 방의 이용자들 반환
list<UINT32> RoomManager::GetMember(UINT32 roomNumber)
{
	// 해당 방의 멤버 리스트 찾기
	auto iter = rooms.find(roomNumber);
	if (iter == rooms.end())
		return list<UINT32>();

	// 리스트 뿌리기
	return iter->second.roomMembers;
}
