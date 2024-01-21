#pragma once

// 특정 기능 제외, 불필요한 코드 제거로 컴파일 시간과 크기 최적화 매크로
// 나중에 알았지만 미리 컴파일된 헤더(pch)를 적용하면 자동적으로 등록된다.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

using namespace std;

class Packet
{
public:
	void Set(Packet& value);
	void Set(Packet* value);
	void Set(unsigned int sessionIndex, unsigned int dataSize, char* pPacketData);

public:
	unsigned int mSessionIndex = 0;
	unsigned int mDataSize = 0;
	string m_pPacketData;
};

