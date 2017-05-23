#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"
#include "../../MemoryDB/MemoryDatabase.h"


#pragma pack(1)

/**
 * @class							tagPackageHead
 * @brief							���ݰ��İ�ͷ�ṹ����
 * @author							barry
 */
typedef struct
{
	unsigned __int64				nSeqNo;				///< �������
	unsigned int					nMsgCount;			///< ���ڵ�Message����
} tagPackageHead;


/**
 * @class							tagBlockHead
 * @brief							���ݿ��ͷ���Ķ���
 * @author							barry
 */
typedef struct
{
	unsigned int					nDataType;			///< ���ݿ�����
	unsigned int					nDataLen;			///< ���ݿ鳤��
} tagBlockHead;

#pragma pack()


/**
 * @class							PackagesBuffer
 * @brief							���ݰ����л���
 * @detail							struct PkgHead + data block1 + data block2 + ...
 * @author							barry
 */
class PackagesBuffer
{
public:
	PackagesBuffer();
	~PackagesBuffer();

public:
	/**
	 * @brief						��ʼ���������
	 * @param[in]					nMaxBufSize				������Ļ����С
	 * @return						==0						�ɹ�
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						�ͷŻ���ռ�
	 */
	void							Release();

public:
	/**
	 * @brief						�洢����
	 * @param[in]					nDataID					����ID
	 * @param[in]					pData					����ָ��
	 * @param[in]					nDataSize				���ݳ���
	 * @param[in]					nSeqNo					��ǰ���ݿ�ĸ������
	 * @param[in]					bEnclosePkg				�Ƿ��װһ����ͷ
	 * @return						==0						�ɹ�
	 */
	int								PushBlock( unsigned int nDataID, const char* pData, unsigned int nDataSize, unsigned __int64 nSeqNo, bool bEnclosePkg );

	/**
	 * @brief						��ȡһ�����ݰ�
	 * @param[out]					pBuff					������ݻ����ַ
	 * @param[in]					nBuffSize				���ݻ��泤��
	 * @return						>0						���ݳ���
									==0						������
									<0						����
	 */
	int								GetOnePkg( char* pBuff, unsigned int nBuffSize );

protected:
	char*							m_pPkgBuffer;			///< ���ݰ������ַ
	unsigned int					m_nMaxPkgBufSize;		///< ���ݰ������С
	unsigned long					m_nFirstPosition;		///< ��ʼλ������
	unsigned long					m_nLastPosition;		///< ����λ������
};


/**
 * @class							SimpleLoopBuffer
 * @brief							ѭ������
 * @author							barry
 */
template<class tempclass>
class SimpleLoopBuffer
{
public:
	SimpleLoopBuffer();
	virtual ~SimpleLoopBuffer();
public:
	/**
	 * @brief						��ʼ���������
	 * @param[in]					nMaxBufSize				������Ļ����С
	 * @return						==0						�ɹ�
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						�ͷŻ���ռ�
	 */
	void							Release();

public:
	/**
	 * @brief						�洢����
	 * @param[in]					nDataID					����ID
	 * @param[in]					lpIn					����ָ��
	 * @param[in]					lInSize					���ݳ���
	 * @return						==0						�ɹ�
	 */
	int								PutData( unsigned int nDataID, const tempclass* lpIn, unsigned long lInSize );

	/**
	 * @brief						ȡ������
	 * @param[out]					nDataID					����ID
	 * @param[out]					lpOut					����д����ָ��
	 * @param[in]					lInSize					����д���泤��
	 * @return						>0						����ȡ�����ݴ�С
	 */
	int								GetData( unsigned int& nDataID, tempclass* lpOut, unsigned int lInSize );

	/**
	 * @brief						�鿴���ݣ������鿴����ȡ��)
	 * @param[out]					lpOut					����д����ָ��
	 * @param[in]					lInSize					����д���泤��
	 * @return						>0						����ȡ�����ݴ�С
	 */
	int								LookData( tempclass * lpOut, unsigned long lInSize );

	/**
	 * @brief						�Ƴ�����
	 * @param[in]					lInSize					���Ƴ���������
	 * @return						==0						�ɹ�
	 */
	int								MoveData( unsigned long lInSize );

public:
	/**
	 * @brief						�������
	 */
	void							Clear();

	/**
	 * @brief						�ж��Ƿ�Ϊ�ջ���
	 */
	bool							IsEmpty();

	/**
	 * @brief						�Ƿ������
	 */
	bool							IsFull();

public:
	/**
	 * @brief						��ȡ��������
	 */
	int								GetRecordCount();

	/**
	 * @brief						��ȡ��ǰ���ࣨʣ�ࣩ�ռ�����
	 */
	int								GetFreeRecordCount();

	/**
	 * @brief						��ȡ�������ռ�
	 */
	int								GetMaxRecord();

	/**
	 * @brief						��ȡ���ݰٷֱ�
	 */
	int								GetPercent();

protected:
	tempclass*						m_lpRecordData;
	unsigned long					m_lMaxRecord;
	unsigned long					m_lFirstRecord;
	unsigned long					m_lLastRecord;

};


template<class tempclass> SimpleLoopBuffer<tempclass>::SimpleLoopBuffer()
: m_lpRecordData( NULL ), m_lMaxRecord( 0 ), m_lFirstRecord( 0 ), m_lLastRecord( 0 )
{
}

template<class tempclass> SimpleLoopBuffer<tempclass>::~SimpleLoopBuffer()
{
	Release();
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::Initialize( unsigned long nMaxBufSize )
{
	assert( nMaxBufSize != 0 );
	Release();

	try
	{
		m_lpRecordData = new tempclass[nMaxBufSize];
	}
	catch( ... )
	{
		m_lpRecordData = NULL;
	}

	if( NULL == m_lpRecordData )
	{
		return -1;
	}

	m_lMaxRecord = nMaxBufSize;
	m_lFirstRecord = 0;
	m_lLastRecord = 0;

	return 0;
}

template<class tempclass>void SimpleLoopBuffer<tempclass>::Release()
{
	if( m_lpRecordData != NULL )
	{
		delete []m_lpRecordData;
		m_lpRecordData = NULL;
	}

	m_lMaxRecord = 0;
	m_lFirstRecord = 0;
	m_lLastRecord = 0;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::PutData( unsigned int nDataID, const tempclass * lpIn,unsigned long lInSize )
{
	register int				errorcode;
	register int				icopysize;

	assert( lpIn != NULL );
	if( lInSize == 0 )
	{
		return 0;
	}

	if( m_lpRecordData == NULL || m_lMaxRecord == 0 )
	{
		assert( 0 );
		return -1;
	}

	errorcode = (m_lFirstRecord + m_lMaxRecord - m_lLastRecord - 1) % m_lMaxRecord;
	if ( lInSize > errorcode )
	{
		return -2;
	}

	icopysize = m_lMaxRecord - m_lLastRecord;
	if( icopysize >= (lInSize+sizeof(unsigned int)*2) )
	{
		*((unsigned int*)(m_lpRecordData + m_lLastRecord)) = nDataID;
		m_lLastRecord += sizeof(unsigned int);
		*((unsigned int*)(m_lpRecordData + m_lLastRecord)) = lInSize;
		m_lLastRecord += sizeof(unsigned int);
		memcpy( &m_lpRecordData[m_lLastRecord],(char *)lpIn,sizeof(tempclass) * lInSize );
	}
	else
	{
		*((unsigned int*)(m_lpRecordData + m_lLastRecord)) = nDataID;
		m_lLastRecord += sizeof(unsigned int);
		*((unsigned int*)(m_lpRecordData + m_lLastRecord)) = lInSize;
		m_lLastRecord += sizeof(unsigned int);
		icopysize -= sizeof(unsigned int)*2;
		memcpy( &m_lpRecordData[m_lLastRecord],lpIn,sizeof(tempclass) * icopysize );
		memcpy( &m_lpRecordData[0],lpIn + icopysize,sizeof(tempclass) * (lInSize - icopysize) );
	}

	m_lLastRecord = (m_lLastRecord + lInSize) % m_lMaxRecord;

	return 0;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetData( unsigned int& nDataID, tempclass* lpOut,unsigned int lInSize )
{
	register int				errorcode;
	register int				icopysize;

	assert( lpOut != NULL );

	if( m_lpRecordData == NULL || m_lMaxRecord == 0 )
	{
		assert( 0 );
		return -1;
	}

	errorcode = (m_lLastRecord + m_lMaxRecord - m_lFirstRecord) % m_lMaxRecord;
	if( errorcode == 0 )
	{
		return -2;
	}
	else if( lInSize > errorcode )
	{
		lInSize = errorcode;
	}

	icopysize = m_lMaxRecord - m_lFirstRecord;
	if( icopysize >= lInSize )
	{
		nDataID = *((unsigned int*)(m_lpRecordData + m_lLastRecord));
		m_lFirstRecord += sizeof(unsigned int);
		lInSize = *((unsigned int*)(m_lpRecordData + m_lLastRecord));
		m_lFirstRecord += sizeof(unsigned int);
		memcpy( lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * lInSize );
	}
	else
	{
		nDataID = *((unsigned int*)(m_lpRecordData + m_lLastRecord));
		m_lFirstRecord += sizeof(unsigned int);
		lInSize = *((unsigned int*)(m_lpRecordData + m_lLastRecord));
		m_lFirstRecord += sizeof(unsigned int);
		memcpy( lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * icopysize );
		memcpy( lpOut + icopysize,&m_lpRecordData[0],sizeof(tempclass) * (lInSize - icopysize) );
	}

	m_lFirstRecord = (m_lFirstRecord + lInSize) % m_lMaxRecord;
	
	return lInSize;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::LookData( tempclass * lpOut,unsigned long lInSize )
{
	register int				errorcode;
	register int				icopysize;
	
	assert( lpOut != NULL );
	
	if( m_lpRecordData == NULL || m_lMaxRecord == 0 )
	{
		assert( 0 );
		return -1;
	}
	
	errorcode = (m_lLastRecord + m_lMaxRecord - m_lFirstRecord) % m_lMaxRecord;
	if( errorcode <= 0 )
	{
		return -2;
	}
	else if ( lInSize > errorcode )
	{
		lInSize = errorcode;
	}
	
	icopysize = m_lMaxRecord - m_lFirstRecord;
	if( icopysize >= lInSize )
	{
		memcpy(lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * lInSize);
	}
	else
	{
		memcpy(lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * icopysize);
		memcpy(lpOut + icopysize,&m_lpRecordData[0],sizeof(tempclass) * (lInSize - icopysize));
	}

	return lInSize;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::MoveData( unsigned long lInSize )
{
	register unsigned long				errorcode;
	
	if( m_lpRecordData == NULL || m_lMaxRecord == 0 )
	{
		assert( 0 );
		return -1;
	}
	
	errorcode = (m_lLastRecord + m_lMaxRecord - m_lFirstRecord) % m_lMaxRecord;
	if( lInSize > errorcode )
	{
		lInSize = errorcode;
	}

	m_lFirstRecord = (m_lFirstRecord + lInSize) % m_lMaxRecord;

	return 0;
}

template<class tempclass>void SimpleLoopBuffer<tempclass>::Clear()
{
	m_lLastRecord = 0;
	m_lFirstRecord = 0;
}

template<class tempclass>bool SimpleLoopBuffer<tempclass>::IsEmpty()
{
	if( m_lLastRecord == m_lFirstRecord )
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<class tempclass>bool SimpleLoopBuffer<tempclass>::IsFull()
{
	if( m_lMaxRecord == 0 )
	{
		assert( 0 );
		return true;
	}
	else if( ((m_lLastRecord + 1) % m_lMaxRecord) == m_lFirstRecord )
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetRecordCount()
{
	if( m_lMaxRecord == 0 )
	{
		assert( 0 );
		return 0;
	}

	return (m_lLastRecord + m_lMaxRecord - m_lFirstRecord) % m_lMaxRecord;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetFreeRecordCount()
{
	if( m_lMaxRecord == 0 )
	{
		assert( 0 );
		return 0;
	}

	return m_lMaxRecord - GetRecordCount() - 1;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetMaxRecord()
{
	return m_lMaxRecord;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetPercent()
{
	if( m_lMaxRecord == 0 )
	{
		assert 0;
		return 100;
	}

	return ((m_lLastRecord + m_lMaxRecord - m_lFirstRecord) % m_lMaxRecord) * 100 / m_lMaxRecord;
}


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
	 * @param[in]				nNewBuffSize			Ҫ����Ļ����С
	 * @return					!= 0					ʧ��
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

protected:
	/**
	 * @brief					������(��ѭ��)
	 * @return					==0						�ɹ�
								!=0						ʧ��
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					����������ѹ������л���
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					��Ϣ���ݵ�ַ
	 * @param[in]				nLen					��Ϣ����
	 * @return					> 0						�ɹ����������ε����ۻ������л��ĳ���
								<= 0					ʧ��
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					�ӻ�����ȡ���� & ���͸��¼��ͻ���
	 * @param[in]				lpLinkNoSet				��·�Ŷ��е�ַ����Ҫ���㲥��������·��ID����
	 * @param[in]				uiLinkNoCount			��·�Ŷ��г���
	 */
	void						FlushQuotation2Client();

protected:
	CriticalObject				m_oLock;				///< ��
	WaitEvent					m_oWaitEvent;			///< �����ȴ�
	SimpleLoopBuffer<char>		m_oDataBuffer;			///< ���ݻ������
	char*						m_pSendBuffer;			///< ���ݷ��ͻ���
	unsigned int				m_nMaxSendBufSize;		///< ���ͻ�����󳤶�
};


/**
 * @class						ImageRebuilder
 * @brief						��ʼ������������
 * @author						barry
 */
class ImageRebuilder
{
private:
	ImageRebuilder();

public:
	/**
	 * @brief					��ȡ�������������
	 */
	static ImageRebuilder&		GetRebuilder();

	/**
	 * @brief					��ʼ�������ʼ��������
	 * @return					!= 0					ʧ��
	 */
	int							Initialize();

	/**
	 * @brief					�ͷŸ���Դ
	 */
	void						Release();

public:
	/**
	 * @brief					����������ͬ��/��ʼ�������пͻ�����·
	 * @param[in]				refDatabaseIO			���ݿ�������
	 * @param[in]				nSerialNo				���Ͳ�ѯ���(��Ҫ>nSerialNo)
	 * @return					>=0						ͬ������·��
								<0						����
	 */
	int							Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					��ȡ����ʼ��������·����
	 */
	unsigned int				GetReqSessionCount();

	/**
	 * @brief					����һ���µĴ���ʼ�����͵���·��
	 * @param[in]				nLinkNo					��·��
	 * @return					true					���ӳɹ�
								false					ʧ�ܣ����ظ���
	 */
	bool						AddNewReqSession( unsigned int nLinkNo );

protected:
	CriticalObject				m_oBuffLock;			///< ��ʼ���������ͻ�����
	std::set<unsigned int>		m_setNewReqLinkID;		///< ����ʼ����·ID����
	char*						m_pImageDataBuffer;		///< ��ʼ�����ݻ���
};




#endif



