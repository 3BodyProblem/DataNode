#pragma warning(disable : 4996)
#include <time.h>
#include "InitFlag.h"
#include "../DataServer/SvrConfig.h"
#include "../DataServer/NodeServer.h"
#include "../Infrastructure/IniFile.h"
#include "../Infrastructure/DateTime.h"


int MkHoliday::Initialize( std::string sHolidayFilePath, bool bTestFlag )
{
	m_bHolidayTestFlag = bTestFlag;
	m_sHolidayPath = sHolidayFilePath;

	return ReloadHoliday();
}

bool MkHoliday::IsValidDatabaseDate( unsigned int nDBDate )
{
	DateTime		oTodayDate = DateTime::Now();

	///< ����ģʽ�£�һֱ������Ч(true)
	if( m_bHolidayTestFlag )
	{
		return true;
	}

	///< ���ڼ�Ϊ�������ڣ�������Ч(true)
	if( oTodayDate.DateToLong() == nDBDate )
	{
		return true;
	}

	///< ��ǰ��ǰһ���Ϸ�������
	for( int n = 0; n < 30; n++ )
	{
		oTodayDate -= 24*60*60;		///< �����ǰһ�죬һ����24*60*60

		unsigned int	nPrevDate = oTodayDate.DateToLong();

		if( false == IsHoliday( nPrevDate ) || nPrevDate < nDBDate )
		{
			if( nPrevDate == nDBDate )
			{
				return true;
			}

			break;
		}
	}

	return false;
}

bool MkHoliday::IsHoliday( int nDate )
{
	if( m_bHolidayTestFlag )
	{
		return false;
	}

	tm t = { 0 };
	t.tm_year = nDate / 10000 - 1900;
	t.tm_mon = nDate % 10000 / 100 - 1;
	t.tm_mday = nDate % 100;
	mktime(&t);

    if( (t.tm_wday == 0) || (t.tm_wday == 6) )
	{
        return true;
    }

    if( find(nDate) != end() )
	{
        return true;
    }

    return false;
}

bool MkHoliday::IsBeforeLongHoliday( unsigned int nDate, bool bPrevious )
{
	int				nLastSeq = -1;
	unsigned int	nHolidayNum = 0;
	DateTime		mDate( nDate/10000, nDate%10000/100, nDate%100 );

	for( int n = 0; n < 30; n++ )
	{
		if( false == bPrevious )	///< ����ж�
		{
			mDate += 24*60*60;		///< һ����24*60*60
		}
		else						///< ��ǰ�ж�
		{
			mDate -= 24*60*60;		///< һ����24*60*60
		}

		unsigned int nCalDate = mDate.DateToLong();
		if( IsHoliday( nCalDate ) )
		{
			if( n - nLastSeq > 1 )
			{
				break;
			}

			nLastSeq = n;
			nHolidayNum++;
		}
		else
		{
			break;
		}
	}

	if( nHolidayNum >= 3 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int MkHoliday::ReloadHoliday()
{
	inifile::IniFile	ini_file;
	int					nErrorCode = 0;
    bool				new_file = false;

	if( 0 != (nErrorCode=ini_file.load( m_sHolidayPath )) )
	{
        DataNodeService::GetSerivceObj().WriteInfo( "MkHoliday::ReloadHoliday() ��failed 2 open holiday.ini %d", nErrorCode );
        return -1;
    }

    clear();
	for( inifile::IniFile::iterator it = ini_file.begin(); it != ini_file.end(); it++ )
	{
		int				nSepIndex = it->first.find( "." );
		std::string		prefix = it->first.substr( 0, nSepIndex );///<ini_file.GetSectionName(i).Left(4);
		if( prefix != Configuration::GetConfigObj().GetHolidayNodeName() )
		{
            continue;
        }

		std::string		str_year = it->first.substr( it->first.length()-nSepIndex+1, it->first.length() );///<ini_file.GetSectionName(i).Right(4);
        int year = atoi(str_year.c_str());
        if ((year >= 3000) || (year <= 2000)) {
            continue;
        }

		DataNodeService::GetSerivceObj().WriteInfo( "MkHoliday::ReloadHoliday() : loading holiday: [%d]", year );

        for (int j = 1; j <= 12; ++j) {
            char month[3];
            _snprintf(month, sizeof(month), "%02d", j);
            month[sizeof(month) - 1] = '\0';
			std::string		days = ini_file.getStringValue( it->first, month, nErrorCode );///<.ReadString(ini_file.GetSectionName(i), month, "");
            char buffer[1024 * 4];
            strncpy(buffer, days.c_str(), sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            char *pch;
            pch = strtok(buffer, ", ");
            while(pch != NULL) {
                int day = atoi(pch);
                int date = year * 10000 + j * 100 + day;
                insert(date);
                pch = strtok(NULL, ", ");
			}
		}
	}

	return 0;
}


InitializerFlag::InitializerFlag( DataCollector& refDataCellector )
 : m_nLastTradingTimeStatus( -1 ), m_refDataCellector( refDataCellector )
{
}

int InitializerFlag::Initialize( const T_VECTOR_PERIODS& refTradingPeriods, std::string sHolidayFilePath, bool bTestFlag )
{
	DataNodeService::GetSerivceObj().WriteInfo( "InitializerFlag::Initialize() : initializing policy module : TradingPeriods Num=%d, TestFlag=%d", refTradingPeriods.size(), bTestFlag );

	int					nErrorCode = 0;
	CriticalLock		guard( m_oLock );

	m_vctTradingPeriod = refTradingPeriods;
	if( 0 != (nErrorCode=m_oHoliday.Initialize( sHolidayFilePath, bTestFlag )) )
	{
		DataNodeService::GetSerivceObj().WriteError( "InitializerFlag::Initialize() : failed 2 initialize holiday table : %s", sHolidayFilePath.c_str() );
		return -1;
	}

	return 0;
}

void InitializerFlag::RedoInitialize()
{
	CriticalLock		guard( m_oLock );

	m_nLastTradingTimeStatus = -1;
}

MkHoliday& InitializerFlag::GetHoliday()
{
	CriticalLock		guard( m_oLock );

	return m_oHoliday;
}

bool InitializerFlag::GetFlag()
{
	CriticalLock		guard( m_oLock );
	bool				bInitFlag = false;
	int					nTradingTimeStatus = InTradingPeriod( bInitFlag );

	if( true == m_refDataCellector.IsProxy() )						///< �ػ���������ɼ�����Ǵ����������
	{
		static char			s_pszTmp[2048] = { 0 };
		unsigned int		nBufLen = sizeof(s_pszTmp);
		enum E_SS_Status	eStatus = m_refDataCellector.InquireDataCollectorStatus( s_pszTmp, nBufLen );

		if( ET_SS_DISCONNECTED == eStatus )							///< �ڴ���Ͽ���ʱ����Ҫ�����������󲢶�������
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if( nTradingTimeStatus < 0 )									///< ���ڽ���ʱ����
	{
		return false;
	}

	if( m_nLastTradingTimeStatus != nTradingTimeStatus )			///< ����ʱ��ѭ���У��״ν����ʱ�� & Ϊ��Ҫ�����³�ʼ����ʱ��
	{
		m_nLastTradingTimeStatus = nTradingTimeStatus;

		return bInitFlag;
	}

	return false;													///< �Ѿ����״��жϱ��ֽ���ʱ��, ����Ҫ���³�ʼ��
}

int InitializerFlag::InTradingPeriod( bool& bInitPoint )
{
	CriticalLock		guard( m_oLock );
	unsigned int		nPeriodsIndex = 0;
	unsigned int		nTime = DateTime::Now().TimeToLong();
	unsigned int		nToday = DateTime::Now().DateToLong();
	bool				bTodayIsHoliday = m_oHoliday.IsHoliday( nToday );

	bInitPoint = false;
	for( T_VECTOR_PERIODS::iterator it = m_vctTradingPeriod.begin(); it != m_vctTradingPeriod.end(); it++, nPeriodsIndex++ )
	{
		unsigned int			nBeginTime = it->nBeginTime;
		unsigned int			nEndTime = it->nEndTime;

		///< ��ֹ����ʱ�䶼��24:00:00ǰ�����
		if( nBeginTime < nEndTime && nTime >= nBeginTime && nTime <= nEndTime )
		{	///< �����̽���ʱ��
			if( true == bTodayIsHoliday )	{
				return -1;
			}

			bInitPoint = it->bInitializePoint;
			return nPeriodsIndex;
		}

		///< ��ֹ����ʱ����24:00:00ǰ��,����������
		if( nBeginTime > nEndTime )
		{
			if( nTime >= nBeginTime )	///< ����֮ǰ
			{
				if( true == m_oHoliday.IsBeforeLongHoliday( nToday, false ) )	{	///< �����ǳ���
					return -1;
				}

				if( true == bTodayIsHoliday )	{
					return -1;
				}

				bInitPoint = it->bInitializePoint;
				return nPeriodsIndex;
			}

			if( nTime <= nEndTime )		///< ����֮��
			{
				if( true == m_oHoliday.IsBeforeLongHoliday( nToday, true ) )	{	///< ǰ���ǳ���
					return -1;
				}
				else
				{
					if( true == bTodayIsHoliday )	{			///< �ǳ��٣������㳿��ҹ��
						tm t = { 0 };
						t.tm_year = nToday / 10000 - 1900;
						t.tm_mon = nToday % 10000 / 100 - 1;
						t.tm_mday = nToday % 100;
						mktime( &t );

						if( (t.tm_wday == 6) )	{
							DateTime		mDate( nToday/10000, nToday%10000/100, nToday%100 );
							mDate -= 24*60*60;		///< һ����24*60*60
							if( true == m_oHoliday.IsBeforeLongHoliday( mDate.DateToLong(), false ) )	///< ��ǰ��һ�죬���㵱��(����)�Ƿ�Ϊ������
							{
								return -1;
							}

							bInitPoint = it->bInitializePoint;
							return nPeriodsIndex;
						}

						return -1;
					}
					else
					{
						tm t = { 0 };							///< �ǳ��٣���һ�㳿��ҹ��
						t.tm_year = nToday / 10000 - 1900;
						t.tm_mon = nToday % 10000 / 100 - 1;
						t.tm_mday = nToday % 100;
						mktime( &t );

						if( (t.tm_wday == 1) )	{
							return false;
						}
					}

					bInitPoint = it->bInitializePoint;
					return nPeriodsIndex;
				}
			}
		}
	}

	return -1;
}




