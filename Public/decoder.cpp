#include "decoder.h"

CDecoder::CDecoder()
{

}

double CDecoder::GetTimebase()
{
	return m_timebase;
}

double CDecoder::GetRate()
{
	return m_rate;
}

double CDecoder::GetDuration()
{
	return m_duration;
}
