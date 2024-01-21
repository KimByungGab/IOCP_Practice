#include "Packet.h"

// 패킷 세팅1
void Packet::Set(Packet& value)
{
	mSessionIndex = value.mSessionIndex;
	mDataSize = value.mDataSize;
	m_pPacketData = value.m_pPacketData;
}

// 패킷 세팅2
void Packet::Set(Packet* value)
{
	mSessionIndex = value->mSessionIndex;
	mDataSize = value->mDataSize;
	m_pPacketData = value->m_pPacketData;
}

// 패킷 세팅3
void Packet::Set(unsigned int sessionIndex, unsigned int dataSize, char* packetData)
{
	mSessionIndex = sessionIndex;
	mDataSize = dataSize;
	m_pPacketData = packetData;
}
