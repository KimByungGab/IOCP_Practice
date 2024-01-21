#pragma once

// Ư�� ��� ����, ���ʿ��� �ڵ� ���ŷ� ������ �ð��� ũ�� ����ȭ ��ũ��
// ���߿� �˾����� �̸� �����ϵ� ���(pch)�� �����ϸ� �ڵ������� ��ϵȴ�.
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

