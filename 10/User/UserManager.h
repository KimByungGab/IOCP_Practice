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
	INT32 mMaxUserCount = 0;			// 최대 수용할 수 있는 사용자 수
	INT32 mCurrentUserCount = 0;		// 현재 유저

	vector<User*> mUserObjPool;			// 유저 오브젝트 풀
};