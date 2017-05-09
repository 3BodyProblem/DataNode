#ifndef __MEngine_MDateTimeH__
#define __MEngine_MDateTimeH__


#include <string>
#ifdef LINUXCODE
#include <sys/timeb.h>
#endif


class DateTime
{
protected:
	time_t					m_stRecordData;
public:
	DateTime(void);
	DateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay);
	DateTime(unsigned short sHour,unsigned short sMin,unsigned short sSec,unsigned short sMSec);
	DateTime(unsigned short sYear,unsigned short sMonth,unsigned short sDay,unsigned short sHour,unsigned short sMin,unsigned short sSec);
	DateTime(time_t stTime);
	DateTime(const DateTime & mTime);
	virtual ~DateTime();
public:
	//获取当前的日期和时间
	void SetCurDateTime(void);
	//从字符串中获取日期和时间，字符串格式位2005-05-29 13:05:12
	int  SetDatetimeByString(const char * strIn);
	int  SetDateByString(const char * strIn);
	int  SetTimeByString(const char * strIn);
public:
	//转换字符串，格式为2005-05-29 13:05:12
	std::string DateTimeToString(void);
	std::string DateToString(void);
	std::string TimeToString(void);
	time_t  DateTimeToTimet(void);
	unsigned long DateToLong(void);
	unsigned long TimeToLong(void);
public:
	//重载运算符
	DateTime & operator = (const DateTime & mIn);
	//重载运算符，减去一个时间，得到两个时间之差，单位为秒
	int operator - (DateTime & mIn);
	//重载运算符，减去一个时间（单位为秒），得到另外一个时间
	DateTime operator - (int iIn);
	//重载运算符，加上一个时间（单位为秒），得到另外一个时间
	DateTime operator + (int iIn);
	//重载运算符，加上一个时间（单位为秒），得到另外一个时间
	DateTime & operator += (int iIn);
	//重载运算符，减去一个时间（单位为秒），得到另外一个时间
	DateTime & operator -= (int iIn);
public:
	//获取该时间和现在时间的差值，单位为秒
	long GetDuration(void);
	//获取该时间和现在时间的差值，用字符串表示，格式为HH:MM:SS
	std::string GetDurationString(void);
public:
	//获取当前时间是位于一周或一年的第几天，注意：星期天为0
	int  GetDayOfWeek(void);
	int  GetDayOfYear(void);
public:
	//分解出时间和日期
	int  DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay);
	int  DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec);
	int  GetYear(void);
	int  GetMonth(void);
	int  GetDay(void);
	int  GetHour(void);
	int  GetMinute(void);
	int  GetSecond(void);
public:
	//比较判断
	bool operator == (const DateTime & mIn);
	bool operator != (const DateTime & mIn);
	bool operator >  (const DateTime & mIn);
	bool operator >= (const DateTime & mIn);
	bool operator <  (const DateTime & mIn);
	bool operator <= (const DateTime & mIn);
public:
	//判断是否为空，为空表示从来没有设置过具体的时间
	bool IsEmpty(void);
	void Empty(void);
public:
	//获取当前时间的DateTime对象
	static DateTime Now(void);
};



#endif



