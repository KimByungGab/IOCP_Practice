#pragma once

#include <unordered_map>

#include "ErrorCode.h"
#include "User.h"

class UserManager
{
public:
	UserManager() = default;
	~UserManager() = default;

	void Init(const INT32 maxUserCount);
	INT32 GetMaxUserCount();
	User* GetUserByConnIdx(INT32 clientIndex);

private:
	INT32 mMaxUserCount = 0;			// �ִ� ������ �� �ִ� ����� ��
	INT32 mCurrentUserCount = 0;		// ���� ����

	vector<User*> mUserObjPool;			// ���� ������Ʈ Ǯ
};