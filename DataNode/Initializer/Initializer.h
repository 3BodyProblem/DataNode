#ifndef __DATA_NODE_INITIALIZER_H__
#define	__DATA_NODE_INITIALIZER_H__


#include <vector>
#include <string>
#include "../Infrastructure/Lock.h"


/**
 * @class				EngineInitializer
 * @brief				��ʼ��(����)��ʶ
 * @detail				ͨ��TestFlag��ʶ���ڼ�����Ϣ��, ����ʱ��Σ�����ÿ��ʱ����Ƿ��ںϷ��Ĺ�����
 * @date				2017/5/3
 * @author				barry
 */
class EngineInitializer
{
public:
	/**
	 * @brief						��ʼ������ʱ��Σ��ڼ��ձ����Ա�ʶ
	 * @param[in]					sHolidayFilePath		�ڼ����ļ�·��
	 * @param[in]					bTestFlag				�ڼ��ղ��Ա�ʶ
	 * @return						==0						��ʼ���ɹ�
									!=0						ʧ��
	 */
	int								Initialize( /*const std::vector<tagTradingPeriod>& refTradingPeriods,*/ const std::string& sHolidayFilePath, bool bTestFlag );

public:
	bool							Get() const;

	void							Set( bool bNewFlag );

	bool							IsTradingPeriod() const;

private:
	CriticalObject					m_oLock;
	bool							m_bTestFlag;			///< ����ģʽ��ʶ
//	Holiday							m_oHolidayTable;		///< �ڼ��ռ�¼
//	std::vector<tagTradingPeriod>	m_vctTradingPeriod;		/// ����ʱ���б������ж��Ƿ�ó�ʼ����
};







#endif








