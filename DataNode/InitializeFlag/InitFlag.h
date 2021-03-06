#ifndef __INITIALIZE_FLAG_INIT_FLAG_H__
#define	__INITIALIZE_FLAG_INIT_FLAG_H__


#include <set>
#include <vector>
#include <string>
#include <utility>
#include "../Infrastructure/Lock.h"
#include "../DataCollector/DataCollector.h"


/**
 * @class							MkHoliday
 * @brief							市场的假日信息
 * @detail							节假日信息，key的格式为yyyymmdd，存储在set中的表示该天为节假日
 * @note							包括节假日是否进行“测试的标识”TestFlag
 * @author							barry
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
	 * @brief						判断数据库加载日期是否有效(即为当天日期，或为前一个有效工作日）
	 * @param[in]					nDBDate				从文件加载的数据库日期
	 * @return						true				合法落盘文件
									false				非法落盘文件
	 * @note						测试标识testflag打开的情况下，是个例外，一直返回true
	 */
	bool							IsValidDatabaseDate( unsigned int nDBDate );

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


/**
 * @class							T_TRADING_PERIOD
 * @brief							交易时段描述
 * @author							barry
 */
typedef struct
{
	unsigned int					nBeginTime;				///< 交易段起时
	unsigned int					nEndTime;				///< 交易段止时
	bool							bInitializePoint;		///< 是否为初始化时段
} T_TRADING_PERIOD;


typedef std::vector<T_TRADING_PERIOD>			T_VECTOR_PERIODS;		///< 所有交易时段信息列表


/**
 * @class							InitializerFlag
 * @brief							初始化(策略)标识
 * @detail							通过TestFlag标识，节假日信息表, 交易时间段，定义每个时间点是否在合法的工作内
 * @author							barry
 */
class InitializerFlag
{
public:
	InitializerFlag( DataCollector& refDataCellector );

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
	/**
	 * @brief						判断当前时间是否需要初始化
	 * @detail						a) 对于市场接口行情采集插件： 在每个时段需要做一次初始化
									b) 对于传输接口行情采集插件： 在tcp断开重连后需要做一次初始化(保持一直连接)
	 * @return						true				需要初始化
	 */
	bool							GetFlag();

	/**
	 * @brief						配置成需要重新初始化
	 */
	void							RedoInitialize();

	/**
	 * @brief						判断是否在交易时段内
	 * @param[out]					bInitPoint			需要重新初始化标识
	 * @return						返回当前时间在配置交易时间段的哪个具体段的索引值
									&
									如果返回值为-1，则表示不在交易时间段内
	 */
	int								InTradingPeriod( bool& bInitPoint );

	/**
	 * @brief						获取节假日集合
	 */
	MkHoliday&						GetHoliday();

private:
	CriticalObject					m_oLock;
	MkHoliday						m_oHoliday;						///< 节假日记录
	DataCollector&					m_refDataCollector;				///< 行情采集插件
	T_VECTOR_PERIODS				m_vctTradingPeriod;				///< 交易时段列表，用于判断是否该初始化了
	int								m_nLastTradingTimeStatus;		///< 最后一次交易状态记录
};






#endif





