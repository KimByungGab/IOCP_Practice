#include "RoomManager.h"

// �� ����
UINT32 RoomManager::CreateRoom()
{
	bool isMake = false;	// �� ���� ����
	UINT32 roomNumber;		// �� ��ȣ

	// ���� �����ɶ����� ���ѹݺ�
	while (isMake == false)
	{
		roomNumber = rand();	// �� ��ȣ�� ����

		// �ߺ��Ǵ��� �� ��ȣ ã��
		auto iter = rooms.find(roomNumber);

		// ���� �� ã�Ҵٸ� ����
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

// �� ����
bool RoomManager::EnterRoom(UINT32 clientIndex, UINT32 roomNumber)
{
	// �� ã��
	auto iter = rooms.find(roomNumber);

	// �� �� ã���� �� ��
	if (iter == rooms.end())
		return false;

	// �ش� �濡 ��� ����
	lock_guard<mutex> guard(mLock);
	iter->second.roomMembers.push_back(clientIndex);
	
	return true;
}

// �� ����
bool RoomManager::LeaveRoom(UINT32 clientIndex, UINT32 roomNumber)
{
	// �� ã��
	auto roomsIter = rooms.find(roomNumber);
	if (roomsIter == rooms.end())
		return false;

	// ���� ����Ʈ ������
	list<UINT32>* pList = &(roomsIter->second.roomMembers);

	// ���� ����Ʈ �����ͷ� �����ϴ� ��� ã��
	auto memberIter = find(pList->begin(), pList->end(), clientIndex);
	if (memberIter == pList->end())
		return false;

	// �����ϴ� ��� ���� ����

	/*
	* �� erase? remove����?
	* 
	* 2������ ������ �ִ�.
	* 
	* ���⼭ ���� �߿��� ���� ��Ȯ�� �������� �ε��� ������ �����̴�.
	* ������ remove�� ����ϰԵǸ� �����̳��� ũ�Ⱑ ������ �پ��� ���� �ƴϰ� �����Ǿ�� �� ���ҵ��� ��ġ�� ������ ���ҵ��� ����� �����°��̴�.
	* �׷��� �ڿ��� �������� �����Ͱ� ���� �ȴ�. �� ��¥ ���ִ� �� �ƴ϶� �����͸� �����°��̴�.
	* ������ �� ��Ȳ�� GetMember���� ��ü��ȸ�� �ϰ� �Ǵ°Ͱ� ������ �ʰ� �ȴ�.
	* 
	* �ι�°�δ� ����Ʈ�� ���԰� ������ �����ϱ� �����̴�.
	* �����迭ó�� �޸𸮰� ���������� ����Ǿ��ִ� ���� �ƴϰ� �޸� �ּҸ� ��ũ�� �ٴ� �����̱� ������ ��ũ�� �����ϸ� �ȴ�.
	* 
	* ������ erase�� ���ߴ�.
	*/
	lock_guard<mutex> guard(mLock);
	pList->erase(memberIter);
	if (pList->size() == 0)
		rooms.erase(roomsIter);

	return true;
}

// �ش� ���� �̿��ڵ� ��ȯ
list<UINT32> RoomManager::GetMember(UINT32 roomNumber)
{
	// �ش� ���� ��� ����Ʈ ã��
	auto iter = rooms.find(roomNumber);
	if (iter == rooms.end())
		return list<UINT32>();

	// ����Ʈ �Ѹ���
	return iter->second.roomMembers;
}
