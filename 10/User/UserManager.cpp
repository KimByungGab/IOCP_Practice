#include "UserManager.h"

// ���� �Ŵ��� �ʱ�ȭ
void UserManager::Init(const INT32 maxUserCount)
{
	mMaxUserCount = maxUserCount;	// ���� ������Ʈ Ǯ�� ����� �� ũ��
	mUserObjPool = vector<User*>(maxUserCount);		// ������Ʈ Ǯ ���� ����

	// �����Ҵ� �� �ʱ�ȭ
	for (INT32 i = 0; i < mMaxUserCount; i++)
	{
		mUserObjPool[i] = new User();
		mUserObjPool[i]->Init(i);
	}
}

// �ִ� ���� ������ ����� �� ��ȯ
INT32 UserManager::GetMaxUserCount()
{
	return mMaxUserCount;
}

// Ŭ���̾�Ʈ �ε����� ���ǰ� ��ȯ
User* UserManager::GetUserByConnIdx(INT32 clientIndex)
{
	return mUserObjPool[clientIndex];
}
