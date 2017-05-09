#ifndef __INITIALIZE_FLAG_INIT_FLAG_H__
#define	__INITIALIZE_FLAG_INIT_FLAG_H__


#include <set>
#include <vector>
#include <string>
#include <utility>
#include "../Infrastructure/Lock.h"


/**
 * @class			MkHoliday
 * @brief			市场的假日信息
 * @detail			节假日信息，key的格式为yyyymmdd，存储在set中的表示该天为节假日
 * @note			包括节假日是否进行“测试的标识”TestFlag
 */
class MkHoliday : public std::set<int>
{
public:
	/**
	 * @brief						设置假日测试标识
	 * @param[in]					sHolidayFilePath	假日信息文件路径
	 * @param[in]					bTestFlag			本节假日是否测试标识
	 * @return						==0					成功
									!=0					失败
	 */
	int								Initialize( std::string sHolidayFilePath, bool bTestFlag );

	/**
	 * @brief						重新加载节假日配置文件
	 * @return						==0					成功
									!=0					失败
	 */
    int								ReloadHoliday();

	/**
	 * @brief						判断参数指定日期是否为节假日
	 */
    bool							IsHoliday( int nDate );

	/**
	 * @brief						判断是长节日前一天，还是后一天
	 * @param[in]					nDate			判断日期
	 * @param[in]					bPrevious		往前判断/往后判断标识
	 * @return						true			是长节假日
	 */
	bool							IsBeforeLongHoliday( unsigned int nDate, bool bPrevious );

protected:
	std::string						m_sHolidayPath;			///< 节假日文件路径
    bool							m_bHolidayTestFlag;		///< 假日测试模式
};


typedef std::pair<unsigned int,unsigned int>	T_TRADING_PERIOD;		///< 交易时段<starttime,endtime>
typedef std::vector<T_TRADING_PERIOD>			T_VECTOR_PERIODS;		///< 所有交易时段信息列表


/**
 * @class							InitializerFlag
 * @brief							初始化(策略)标识
 * @detail							通过TestFlag标识，节假日信息表, 交易时间段，定义每个时间点是否在合法的工作内
 */
class InitializerFlag
{
public:
	InitializerFlag();

public:
	/**
	 * @brief						初始化交易时间段，节假日表，测试标识
	 * @param[in]					refTradingPeriods	市场交易时段信息表
	 * @param[in]					sHolidayFilePath	假日信息文件路径
	 * @param[in]					bTestFlag			本节假日是否测试标识
	 * @return						==0					初始化成功
									!=0					出错
	 */
	int								Initialize( const T_VECTOR_PERIODS& refTradingPeriods, std::string sHolidayFilePath, bool bTestFlag );

public:
	bool							Get() const;

	void							Set( bool bNewFlag );

	/**
	 * @brief						判断是否在交易时段内
	 * @return						true				在交易时段内
									false				不在
	 */
	bool							InTradingPeriod() const;

private:
	CriticalObject					m_oLock;
	MkHoliday						m_oHoliday;				///< 节假日记录
	T_VECTOR_PERIODS				m_vctTradingPeriod;		/// 交易时段列表，用于判断是否该初始化了
};






#endif





