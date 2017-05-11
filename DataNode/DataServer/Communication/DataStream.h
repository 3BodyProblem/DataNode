#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"


/**
 * @class						QuotationStream
 * @brief						����������
 * @author						barry
 */
class QuotationStream : public SimpleTask
{
public:
	/**
	 * @brief					���캯��
	 */
	QuotationStream();
	~QuotationStream();

	/**
	 * @brief					��ʼ�����������ͻ���
	 * @param[in]				nNewBuffSize		Ҫ����Ļ����С
	 * @return					!= 0				ʧ��
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

protected:
	/**
	 * @brief					������(��ѭ��)
	 * @return					==0					�ɹ�
								!=0					ʧ��
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					����������ѹ������л���
	 * @param[in]				nMsgID				Message ID
	 * @param[in]				pData				��Ϣ���ݵ�ַ
	 * @param[in]				nLen				��Ϣ����
	 * @return					> 0					�ɹ����������ε����ۻ������л��ĳ���
								<= 0				ʧ��
	 */
	int							EncodeMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					������������
	 * @param[in]				uiLinkNo			��·��
	 * @param[in]				eFuncID				���󷵻ع��ܺ�
	 */
	void						ResponseData2Platform( unsigned int uiLinkNo, enum QuotationFunctionID eFuncID );

	/**
	 * @brief					������������, ���͹��ܺ� = FUNCTIONID_PUSH_QUOTATION
	 * @param[in]				lpLinkNoSet			��·�Ŷ��е�ַ����Ҫ���㲥��������·��ID����
	 * @param[in]				uiLinkNoCount		��·�Ŷ��г���
	 */
	void						FlushData2Platform( const unsigned int * lpLinkNoSet, unsigned int uiLinkNoCount );

protected:
	CriticalObject				m_oLock;				///< ��
	char*						m_pszBuffer;			///< ѹ������ռ�
	unsigned int				m_nBuffSize;			///< ����ռ��С
	unsigned int				m_nCurrSize;			///< ��ǰʹ�õĿռ��С
};


#endif



