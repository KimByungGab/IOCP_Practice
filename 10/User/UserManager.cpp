#include "UserManager.h"

// 유저 매니저 초기화
void UserManager::Init(const INT32 maxUserCount)
{
	mMaxUserCount = maxUserCount;	// 유저 오브젝트 풀에 담겨질 총 크기
	mUserObjPool = vector<User*>(maxUserCount);		// 오브젝트 풀 벡터 생성

	// 동적할당 후 초기화
	for (INT32 i = 0; i < mMaxUserCount; i++)
	{
		mUserObjPool[i] = new User();
		mUserObjPool[i]->Init(i);
	}
}

// 최대 수용 가능한 사용자 수 반환
INT32 UserManager::GetMaxUserCount()
{
	return mMaxUserCount;
}

// 클라이언트 인덱스로 세션값 반환
User* UserManager::GetUserByConnIdx(INT32 clientIndex)
{
	return mUserObjPool[clientIndex];
}
