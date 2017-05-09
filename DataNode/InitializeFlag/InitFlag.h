#ifndef __INITIALIZE_FLAG_INIT_FLAG_H__
#define	__INITIALIZE_FLAG_INIT_FLAG_H__


/**
 * @class				InitializerFlag
 * @brief				��ʼ��(����)��ʶ
 * @detail				ͨ��TestFlag��ʶ���ڼ�����Ϣ��, ����ʱ��Σ�����ÿ��ʱ����Ƿ��ںϷ��Ĺ�����
 */
class InitializerFlag
{
public:
	/**
	 * @brief			��ʼ������ʱ��Σ��ڼ��ձ����Ա�ʶ
	 */
	void				Initialize( const std::vector<tagTradingPeriod>& refTradingPeriods, const std::string& sHolidayFilePath, bool bTestFlag );

public:
	bool				Get() const;

	void				Set( bool bNewFlag );

	bool				IsTradingPeriod() const;

private:
	MMutex				m_oLock;
	bool				m_bTestFlag;			///< ����ģʽ��ʶ
	Holiday				m_oHolidayTable;		///< �ڼ��ռ�¼
	std::vector<tagTradingPeriod>	m_vctTradingPeriod;		/// ����ʱ���б������ж��Ƿ�ó�ʼ����
};






#endif





