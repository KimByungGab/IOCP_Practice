#include "Packet.h"

// 패킷 세팅1
void Packet::Set(Packet& value)
{
	mSessionIndex = value.mSessionIndex;
	mDataSize = value.mDataSize;

	m_pPacketData = new char[value.mDataSize];
	memcpy(m_pPacketData, value.m_pPacketData, value.mDataSize);
}

// 패킷 세팅2
void Packet::Set(unsigned int sessionIndex, unsigned int dataSize, char* pPacketData)
{
	mSessionIndex = sessionIndex;
	mDataSize = dataSize;
	
	m_pPacketData = new char[dataSize];
	memcpy(m_pPacketData, pPacketData, dataSize);
}

// 패킷 동적할당 해제
void Packet::Release()
{
	delete m_pPacketData;
}
