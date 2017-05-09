#ifndef __INITIALIZE_FLAG_INIT_FLAG_H__
#define	__INITIALIZE_FLAG_INIT_FLAG_H__


/**
 * @class				InitializerFlag
 * @brief				初始化(策略)标识
 * @detail				通过TestFlag标识，节假日信息表, 交易时间段，定义每个时间点是否在合法的工作内
 */
class InitializerFlag
{
public:
	/**
	 * @brief			初始化交易时间段，节假日表，测试标识
	 */
	void				Initialize( const std::vector<tagTradingPeriod>& refTradingPeriods, const std::string& sHolidayFilePath, bool bTestFlag );

public:
	bool				Get() const;

	void				Set( bool bNewFlag );

	bool				IsTradingPeriod() const;

private:
	MMutex				m_oLock;
	bool				m_bTestFlag;			///< 测试模式标识
	Holiday				m_oHolidayTable;		///< 节假日记录
	std::vector<tagTradingPeriod>	m_vctTradingPeriod;		/// 交易时段列表，用于判断是否该初始化了
};






#endif





