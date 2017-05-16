#ifndef __INITIALIZE_FLAG_INIT_FLAG_H__
#define	__INITIALIZE_FLAG_INIT_FLAG_H__


#include <set>
#include <vector>
#include <string>
#include <utility>
#include "../Infrastructure/Lock.h"


/**
 * @class							MkHoliday
 * @brief							�г��ļ�����Ϣ
 * @detail							�ڼ�����Ϣ��key�ĸ�ʽΪyyyymmdd���洢��set�еı�ʾ����Ϊ�ڼ���
 * @note							�����ڼ����Ƿ���С����Եı�ʶ��TestFlag
 * @author							barry
 */
class MkHoliday : public std::set<int>
{
public:
	/**
	 * @brief						���ü��ղ��Ա�ʶ
	 * @param[in]					sHolidayFilePath	������Ϣ�ļ�·��
	 * @param[in]					bTestFlag			���ڼ����Ƿ���Ա�ʶ
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
	int								Initialize( std::string sHolidayFilePath, bool bTestFlag );

	/**
	 * @brief						���¼��ؽڼ��������ļ�
	 * @return						==0					�ɹ�
									!=0					ʧ��
	 */
    int								ReloadHoliday();

	/**
	 * @brief						�жϲ���ָ�������Ƿ�Ϊ�ڼ���
	 */
    bool							IsHoliday( int nDate );

	/**
	 * @brief						�ж��ǳ�����ǰһ�죬���Ǻ�һ��
	 * @param[in]					nDate			�ж�����
	 * @param[in]					bPrevious		��ǰ�ж�/�����жϱ�ʶ
	 * @return						true			�ǳ��ڼ���
	 */
	bool							IsBeforeLongHoliday( unsigned int nDate, bool bPrevious );

protected:
	std::string						m_sHolidayPath;			///< �ڼ����ļ�·��
    bool							m_bHolidayTestFlag;		///< ���ղ���ģʽ
};


/**
 * @class							T_TRADING_PERIOD
 * @brief							����ʱ������
 * @author							barry
 */
typedef struct
{
	unsigned int					nBeginTime;				///< ���׶���ʱ
	unsigned int					nEndTime;				///< ���׶�ֹʱ
	bool							bInitializePoint;		///< �Ƿ�Ϊ��ʼ��ʱ��
} T_TRADING_PERIOD;


typedef std::vector<T_TRADING_PERIOD>			T_VECTOR_PERIODS;		///< ���н���ʱ����Ϣ�б�


/**
 * @class							InitializerFlag
 * @brief							��ʼ��(����)��ʶ
 * @detail							ͨ��TestFlag��ʶ���ڼ�����Ϣ��, ����ʱ��Σ�����ÿ��ʱ����Ƿ��ںϷ��Ĺ�����
 * @author							barry
 */
class InitializerFlag
{
public:
	InitializerFlag();

	/**
	 * @brief						��ʼ������ʱ��Σ��ڼ��ձ����Ա�ʶ
	 * @param[in]					refTradingPeriods	�г�����ʱ����Ϣ��
	 * @param[in]					sHolidayFilePath	������Ϣ�ļ�·��
	 * @param[in]					bTestFlag			���ڼ����Ƿ���Ա�ʶ
	 * @return						==0					��ʼ���ɹ�
									!=0					����
	 */
	int								Initialize( const T_VECTOR_PERIODS& refTradingPeriods, std::string sHolidayFilePath, bool bTestFlag );

public:
	/**
	 * @brief						�жϵ�ǰʱ���Ƿ���Ҫ��ʼ��
	 * @return						true				��Ҫ��ʼ��
	 */
	bool							GetFlag();

	/**
	 * @brief						���ó���Ҫ���³�ʼ��
	 */
	void							RedoInitialize();

	/**
	 * @brief						�ж��Ƿ��ڽ���ʱ����
	 * @param[out]					bInitPoint			��Ҫ���³�ʼ����ʶ
	 * @return						���ص�ǰʱ�������ý���ʱ��ε��ĸ�����ε�����ֵ
									&
									�������ֵΪ-1�����ʾ���ڽ���ʱ�����
	 */
	int								InTradingPeriod( bool& bInitPoint );

private:
	CriticalObject					m_oLock;
	MkHoliday						m_oHoliday;						///< �ڼ��ռ�¼
	T_VECTOR_PERIODS				m_vctTradingPeriod;				///< ����ʱ���б������ж��Ƿ�ó�ʼ����
	int								m_nLastTradingTimeStatus;		///< ���һ�ν���״̬��¼
};






#endif





