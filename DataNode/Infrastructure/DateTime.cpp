#pragma warning(disable : 4996)
#include <time.h>
#include <assert.h>
#include "DateTime.h"


DateTime::DateTime(void)
{
	m_stRecordData = 0;
}

DateTime::DateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay)
{
	struct tm						sttime;
	
	sttime.tm_year = sYear - 1900;
	sttime.tm_mon = sMonth - 1;
	sttime.tm_mday = sDay;
	sttime.tm_hour = 0;  
	sttime.tm_min = 0;
	sttime.tm_sec = 0;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}

DateTime::DateTime(unsigned short sHour,unsigned short sMin,unsigned short sSec,unsigned short sMSec)
{
	struct tm						sttime;
	
	sttime.tm_year = 70;
	sttime.tm_mon = 0;
	sttime.tm_mday = 1;
	sttime.tm_hour = sHour;  
	sttime.tm_min = sMin;
	sttime.tm_sec = sSec;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}

DateTime::DateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay,unsigned short sHour,unsigned short sMin,unsigned short sSec)
{
	struct tm						sttime;
	
	sttime.tm_year = sYear - 1900;
	sttime.tm_mon = sMonth - 1;
	sttime.tm_mday = sDay;
	sttime.tm_hour = sHour;  
	sttime.tm_min = sMin;
	sttime.tm_sec = sSec;
	m_stRecordData = mktime(&sttime);
	assert(m_stRecordData != -1);
}

DateTime::DateTime(time_t stTime)
{
	m_stRecordData = stTime;
}

DateTime::DateTime(const DateTime & mTime)
{
	m_stRecordData = mTime.m_stRecordData;
}

DateTime::~DateTime()
{

}

void DateTime::SetCurDateTime(void)
{
	m_stRecordData = time(NULL);
}

int  DateTime::SetDatetimeByString(const char * strIn)
{
	int								iyear,imon,iday,ihour,imin,isec;
	struct tm						sttime;

	sscanf(strIn,"%04d-%02d-%02d %02d:%02d:%02d",&iyear,&imon,&iday,&ihour,&imin,&isec);

	sttime.tm_year = iyear - 1900;
	sttime.tm_mon = imon - 1;
	sttime.tm_mday = iday;
	sttime.tm_hour = ihour;  
	sttime.tm_min = imin;
	sttime.tm_sec = isec;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return -1;
	}

	return(1);
}

int  DateTime::SetDateByString(const char * strIn)
{
	int								iyear,imon,iday;
	struct tm						sttime;
	
	sscanf(strIn,"%04d-%02d-%02d",&iyear,&imon,&iday);
	
	sttime.tm_year = iyear - 1900;
	sttime.tm_mon = imon - 1;
	sttime.tm_mday = iday;
	sttime.tm_hour = 0;  
	sttime.tm_min = 0;
	sttime.tm_sec = 0;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return -1;
	}
	
	return(1);
}

int  DateTime::SetTimeByString(const char * strIn)
{
	int								ihour,imin,isec;
	struct tm						sttime;
	
	sscanf(strIn,"%02d:%02d:%02d",&ihour,&imin,&isec);
	
	sttime.tm_year = 70;
	sttime.tm_mon = 0;
	sttime.tm_mday = 1;
	sttime.tm_hour = ihour;  
	sttime.tm_min = imin;
	sttime.tm_sec = isec;
	if ( (m_stRecordData = mktime(&sttime)) == -1 )
	{
		assert(0);
		return -1;
	}
	
	return(1);
}

std::string DateTime::DateTimeToString(void)
{
	char							tempbuf[256];

	#ifndef LINUXCODE

		struct tm				*	lptemptime;
	
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}

		::sprintf(	tempbuf,
					"%04d-%02d-%02d %02d:%02d:%02d",
					lptemptime->tm_year + 1900,
					lptemptime->tm_mon + 1,
					lptemptime->tm_mday,
					lptemptime->tm_hour,
					lptemptime->tm_min,
					lptemptime->tm_sec	);
	#else

		struct tm					sttemptime;

		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}

		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d %02d:%02d:%02d",
						sttemptime.tm_year + 1900,
						sttemptime.tm_mon + 1,
						sttemptime.tm_mday,
						sttemptime.tm_hour,
						sttemptime.tm_min,
						sttemptime.tm_sec	);
	#endif

	return(tempbuf);
}

std::string DateTime::DateToString(void)
{
	char							tempbuf[256];
	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		::sprintf(	tempbuf,
					"%04d-%02d-%02d",
					lptemptime->tm_year + 1900,
					lptemptime->tm_mon + 1,
					lptemptime->tm_mday	);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%04d-%02d-%02d %02d:%02d:%02d",
						sttemptime.tm_year + 1900,
						sttemptime.tm_mon + 1,
						sttemptime.tm_mday	);
	#endif
		
	return(tempbuf);
}

std::string DateTime::TimeToString(void)
{
	char							tempbuf[256];
	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		::sprintf(	tempbuf,
					"%02d:%02d:%02d",
					lptemptime->tm_hour,
					lptemptime->tm_min,
					lptemptime->tm_sec	);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return("无效的日期或时间");
		}
		
		my_snprintf(	tempbuf,256,
						"%02d:%02d:%02d",
						sttemptime.tm_hour,
						sttemptime.tm_min,
						sttemptime.tm_sec	);
	#endif
	
	return(tempbuf);
}

time_t  DateTime::DateTimeToTimet(void)
{
	return(m_stRecordData);
}

unsigned long DateTime::DateToLong(void)
{	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(0);
		}

		return((lptemptime->tm_year + 1900) * 10000 + (lptemptime->tm_mon + 1) * 100 + lptemptime->tm_mday);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(0);
		}
		
		return((sttemptime.tm_year + 1900) * 10000 + (sttemptime.tm_mon + 1) * 100 + sttemptime.tm_mday);
	#endif
}

unsigned long DateTime::TimeToLong(void)
{	
	#ifndef LINUXCODE

		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return(0);
		}

		return(lptemptime->tm_hour * 10000 + lptemptime->tm_min * 100 + lptemptime->tm_sec);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(0);
		}
		
		return(sttemptime.tm_hour * 10000 + sttemptime.tm_min * 100 + sttemptime.tm_sec);
	#endif
}

DateTime & DateTime::operator = (const DateTime & mIn)
{
	m_stRecordData = mIn.m_stRecordData;

	return(* this);
}

int DateTime::operator - (DateTime & mIn)
{
	return((int)(m_stRecordData - mIn.m_stRecordData));
}

DateTime DateTime::operator - (int iIn)
{
	return(DateTime(m_stRecordData - iIn));
}

DateTime DateTime::operator + (int iIn)
{
	return(DateTime(m_stRecordData + iIn));
}

DateTime & DateTime::operator += (int iIn)
{
	m_stRecordData += iIn;

	return(* this);
}

DateTime & DateTime::operator -= (int iIn)
{
	m_stRecordData -= iIn;
	
	return(* this);
}

long DateTime::GetDuration(void)
{
	return((long)(time(NULL) - m_stRecordData));
}

std::string DateTime::GetDurationString(void)
{
	register long				errorcode;
	char						tempbuf[256];

	errorcode = GetDuration();
	::sprintf(tempbuf,"%02d:%02d:%02d",errorcode / 3500,(errorcode % 3600) / 60,errorcode % 60);

	return(tempbuf);
}

int  DateTime::GetDayOfWeek(void)
{	
	#ifndef LINUXCODE
	
		struct tm				*	lptemptime;

		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_wday);
	#else
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData, &sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_wday);
	#endif
}

int  DateTime::GetDayOfYear(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_yday);
	#else
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData, &sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_yday);
	#endif
}

int  DateTime::DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay)
{
	assert(sYear != NULL);
	assert(sMonth != NULL);
	assert(sDay != NULL);

	#ifndef LINUXCODE

		struct tm				*	lptemptime;

		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}

		* sYear = lptemptime->tm_year + 1900;
		* sMonth = lptemptime->tm_mon + 1;
		* sDay = lptemptime->tm_mday;

		return(1);
	#else

		struct tm					sttemptime;

		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}

		* sYear = sttemptime.tm_year + 1900;
		* sMonth = sttemptime.tm_mon + 1;
		* sDay = sttemptime.tm_mday;
		
		return(1);
	#endif
}

int  DateTime::DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec)
{
	assert(sHour != NULL);
	assert(sMin != NULL);
	assert(sSec != NULL);
	
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}

		* sHour = lptemptime->tm_hour;
		* sMin = lptemptime->tm_min;
		* sSec = lptemptime->tm_sec;
		
		return(1);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		* sHour = sttemptime.tm_hour;
		* sMin = sttemptime.tm_min;
		* sSec = sttemptime.tm_sec;
		
		return(1);
	#endif
}

int  DateTime::GetYear(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_year+1900);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_year+1900);
	#endif
}

int  DateTime::GetMonth(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_mon+1);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_mon+1);
	#endif
}

int  DateTime::GetDay(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_mday);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_mday);
	#endif
}

int  DateTime::GetHour(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_hour);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_hour);
	#endif
}

int  DateTime::GetMinute(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_min);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_min);
	#endif
}

int  DateTime::GetSecond(void)
{
	#ifndef LINUXCODE
		
		struct tm				*	lptemptime;
		
		if ( (lptemptime = localtime(&m_stRecordData)) == NULL )
		{
			assert(0);
			return -1;
		}
		
		return(lptemptime->tm_sec);
	#else
		
		struct tm					sttemptime;
		
		if ( localtime_r(&m_stRecordData,&sttemptime) == NULL )
		{
			assert(0);
			return(ERR_MDATETIME_INVALID);
		}
		
		return(sttemptime.tm_sec);
	#endif
}

bool DateTime::operator == (const DateTime & mIn)
{
	if ( m_stRecordData == mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::operator != (const DateTime & mIn)
{
	if ( m_stRecordData != mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::operator >  (const DateTime & mIn)
{
	if ( m_stRecordData > mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::operator >= (const DateTime & mIn)
{
	if ( m_stRecordData >= mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::operator <  (const DateTime & mIn)
{
	if ( m_stRecordData < mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::operator <= (const DateTime & mIn)
{
	if ( m_stRecordData <= mIn.m_stRecordData )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool DateTime::IsEmpty(void)
{
	if ( m_stRecordData == 0 )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

void DateTime::Empty(void)
{
	m_stRecordData = 0;
}

DateTime DateTime::Now(void)
{
	return(DateTime(time(NULL)));
}




