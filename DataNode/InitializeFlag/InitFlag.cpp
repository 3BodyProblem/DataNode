#include <time.h>
#include "InitFlag.h"
#include "../DataServer/NodeServer.h"
#include "../Infrastructure/IniFile.h"
#include "../Infrastructure/DateTime.h"


int MkHoliday::Initialize( std::string sHolidayFilePath, bool bTestFlag )
{
	m_bHolidayTestFlag = bTestFlag;
	m_sHolidayPath = sHolidayFilePath.c_str();

	SvrFramework::GetFramework().WriteInfo( "MkHoliday::Initialize()", "holiday configuration path : %s", m_sHolidayPath.c_str() );
	SvrFramework::GetFramework().WriteInfo( "MkHoliday::Initialize()", true==m_bHolidayTestFlag?"------ Test Model -----":"----------------" );

	return ReloadHoliday();
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
		if( false == bPrevious )	///< 向后判断
		{
			mDate += 24*60*60;		///< 一天是24*60*60
		}
		else						///< 向前判断
		{
			mDate -= 24*60*60;		///< 一天是24*60*60
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

	if( ini_file.load( m_sHolidayPath ) <= 0 )
	{
        SvrFramework::GetFramework().WriteInfo( "MOptionIO", "打开holiday.ini失败:%d", errno );
        return -1;
    }

    clear();
	for( inifile::IniFile::iterator it = ini_file.begin(); it != ini_file.end(); it++ )
	{
		std::string		prefix = it->first.substr( 0, 4 );///<ini_file.GetSectionName(i).Left(4);
        if (prefix != "cnf.") {
            continue;
        }

		std::string		str_year = it->first.substr( it->first.length() - 4, it->first.length() );///<ini_file.GetSectionName(i).Right(4);
        int year = atoi(str_year.c_str());
        if ((year >= 3000) || (year <= 2000)) {
            continue;
        }

		SvrFramework::GetFramework().WriteInfo( "MkHoliday::ReloadHoliday() : loading holiday: [%d]", year );

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


InitializerFlag::InitializerFlag()
{
}

int InitializerFlag::Initialize( const T_VECTOR_PERIODS& refTradingPeriods, std::string sHolidayFilePath, bool bTestFlag )
{
	SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Initialize() : initializing policy module : TradingPeriods Num=%d, TestFlag=%d", refTradingPeriods.size(), bTestFlag );

	int			nErrorCode = 0;

	m_vctTradingPeriod = refTradingPeriods;
	if( 0 != (nErrorCode=m_oHoliday.Initialize( sHolidayFilePath, bTestFlag )) )
	{
		SvrFramework::GetFramework().WriteError( "InitializerFlag::Initialize() : failed 2 initialize holiday table : %s", sHolidayFilePath.c_str() );
		return -1;
	}

	SvrFramework::GetFramework().WriteInfo( "InitializerFlag::Initialize() : policy module is initialized" );

	return 0;
}




