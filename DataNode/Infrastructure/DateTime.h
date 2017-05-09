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
	//��ȡ��ǰ�����ں�ʱ��
	void SetCurDateTime(void);
	//���ַ����л�ȡ���ں�ʱ�䣬�ַ�����ʽλ2005-05-29 13:05:12
	int  SetDatetimeByString(const char * strIn);
	int  SetDateByString(const char * strIn);
	int  SetTimeByString(const char * strIn);
public:
	//ת���ַ�������ʽΪ2005-05-29 13:05:12
	std::string DateTimeToString(void);
	std::string DateToString(void);
	std::string TimeToString(void);
	time_t  DateTimeToTimet(void);
	unsigned long DateToLong(void);
	unsigned long TimeToLong(void);
public:
	//���������
	DateTime & operator = (const DateTime & mIn);
	//�������������ȥһ��ʱ�䣬�õ�����ʱ��֮���λΪ��
	int operator - (DateTime & mIn);
	//�������������ȥһ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	DateTime operator - (int iIn);
	//���������������һ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	DateTime operator + (int iIn);
	//���������������һ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	DateTime & operator += (int iIn);
	//�������������ȥһ��ʱ�䣨��λΪ�룩���õ�����һ��ʱ��
	DateTime & operator -= (int iIn);
public:
	//��ȡ��ʱ�������ʱ��Ĳ�ֵ����λΪ��
	long GetDuration(void);
	//��ȡ��ʱ�������ʱ��Ĳ�ֵ�����ַ�����ʾ����ʽΪHH:MM:SS
	std::string GetDurationString(void);
public:
	//��ȡ��ǰʱ����λ��һ�ܻ�һ��ĵڼ��죬ע�⣺������Ϊ0
	int  GetDayOfWeek(void);
	int  GetDayOfYear(void);
public:
	//�ֽ��ʱ�������
	int  DecodeDate(unsigned short * sYear,unsigned short * sMonth,unsigned short * sDay);
	int  DecodeTime(unsigned short * sHour,unsigned short * sMin,unsigned short * sSec);
	int  GetYear(void);
	int  GetMonth(void);
	int  GetDay(void);
	int  GetHour(void);
	int  GetMinute(void);
	int  GetSecond(void);
public:
	//�Ƚ��ж�
	bool operator == (const DateTime & mIn);
	bool operator != (const DateTime & mIn);
	bool operator >  (const DateTime & mIn);
	bool operator >= (const DateTime & mIn);
	bool operator <  (const DateTime & mIn);
	bool operator <= (const DateTime & mIn);
public:
	//�ж��Ƿ�Ϊ�գ�Ϊ�ձ�ʾ����û�����ù������ʱ��
	bool IsEmpty(void);
	void Empty(void);
public:
	//��ȡ��ǰʱ���DateTime����
	static DateTime Now(void);
};



#endif



