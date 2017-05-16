#ifndef __QUOTATION_STREAM_H__
#define __QUOTATION_STREAM_H__


#pragma warning(disable:4018)
#include <set>
#include "../../Infrastructure/Lock.h"
#include "../../Infrastructure/Thread.h"
#include "../../MemoryDB/MemoryDatabase.h"


/**
 * @class							SimpleLoopBuffer
 * @brief							循环缓存
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
	 * @brief						初始化缓存对象
	 * @param[in]					nMaxBufSize				将分配的缓存大小
	 * @return						==0						成功
	 */
	int								Initialize( unsigned long nMaxBufSize );

	/**
	 * @brief						释放缓存空间
	 */
	void							Release();

public:
	/**
	 * @brief						存储数据
	 * @param[in]					lpIn					数据指针
	 * @param[in]					lInSize					数据长度
	 * @return						==0						成功
	 */
	int								PutData( const tempclass* lpIn, unsigned long lInSize );

	/**
	 * @brief						取出数据
	 * @param[out]					lpOut					数据写缓存指针
	 * @param[in]					lInSize					数据写缓存长度
	 * @return						>0						返回取出数据大小
	 */
	int								GetData( tempclass* lpOut, unsigned long lInSize );

	/**
	 * @brief						查看数据（仅仅查看，不取出)
	 * @param[out]					lpOut					数据写缓存指针
	 * @param[in]					lInSize					数据写缓存长度
	 * @return						>0						返回取出数据大小
	 */
	int								LookData( tempclass * lpOut, unsigned long lInSize );

	/**
	 * @brief						移除数据
	 * @param[in]					lInSize					将移除的数据量
	 * @return						==0						成功
	 */
	int								MoveData( unsigned long lInSize );

public:
	/**
	 * @brief						清除数据
	 */
	void							Clear();

	/**
	 * @brief						判断是否为空或满
	 */
	bool							IsEmpty();

	/**
	 * @brief						是否队列满
	 */
	bool							IsFull();

public:
	/**
	 * @brief						获取数据数量
	 */
	int								GetRecordCount();

	/**
	 * @brief						获取当前空余（剩余）空间数量
	 */
	int								GetFreeRecordCount();

	/**
	 * @brief						获取数据最大空间
	 */
	int								GetMaxRecord();

	/**
	 * @brief						获取数据百分比
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

template<class tempclass>int  SimpleLoopBuffer<tempclass>::PutData( const tempclass * lpIn,unsigned long lInSize )
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
	if( icopysize >= lInSize )
	{
		memcpy( &m_lpRecordData[m_lLastRecord],(char *)lpIn,sizeof(tempclass) * lInSize );
	}
	else
	{
		memcpy( &m_lpRecordData[m_lLastRecord],lpIn,sizeof(tempclass) * icopysize );
		memcpy( &m_lpRecordData[0],lpIn + icopysize,sizeof(tempclass) * (lInSize - icopysize) );
	}

	m_lLastRecord = (m_lLastRecord + lInSize) % m_lMaxRecord;

	return 0;
}

template<class tempclass>int  SimpleLoopBuffer<tempclass>::GetData( tempclass* lpOut,unsigned long lInSize )
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
		memcpy( lpOut,&m_lpRecordData[m_lFirstRecord],sizeof(tempclass) * lInSize );
	}
	else
	{
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
 * @brief						行情流缓存
 * @author						barry
 */
class QuotationStream : public SimpleTask
{
public:
	/**
	 * @brief					构造函数
	 */
	QuotationStream();
	~QuotationStream();

	/**
	 * @brief					初始化行情流推送缓存
	 * @param[in]				nNewBuffSize			要分配的缓存大小
	 * @return					!= 0					失败
	 */
	int							Initialize( unsigned int nNewBuffSize = 1024*1024*10 );

	/**
	 * @brief					释放各资源
	 */
	void						Release();

protected:
	/**
	 * @brief					任务函数(内循环)
	 * @return					==0						成功
								!=0						失败
	 */
	virtual int					Execute();

public:
	/**
	 * @brief					将行情数据压缩后进行缓存
	 * @param[in]				nMsgID					Message ID
	 * @param[in]				pData					消息数据地址
	 * @param[in]				nLen					消息长度
	 * @return					> 0						成功，返回历次调用累积的序列化的长度
								<= 0					失败
	 */
	int							PutMessage( unsigned short nMsgID, const char *pData, unsigned int nLen );

	/**
	 * @brief					从缓存中取数据 & 推送给下级客户端
	 * @param[in]				lpLinkNoSet				链路号队列地址，需要被广播的所有链路的ID集合
	 * @param[in]				uiLinkNoCount			链路号队列长度
	 */
	void						FlushQuotation2Client();

protected:
	CriticalObject				m_oLock;				///< 锁
	WaitEvent					m_oWaitEvent;			///< 条件等待
	SimpleLoopBuffer<char>		m_oDataBuffer;			///< 数据缓存队列
	char*						m_pSendBuffer;			///< 数据发送缓存
	unsigned int				m_nMaxSendBufSize;		///< 发送缓存最大长度
};


/**
 * @class						ImageRebuilder
 * @brief						初始化行情流缓存
 * @author						barry
 */
class ImageRebuilder
{
private:
	ImageRebuilder();

public:
	static ImageRebuilder&		GetObj();

	/**
	 * @brief					初始化行情初始化流缓存
	 * @return					!= 0					失败
	 */
	int							Initialize();

	/**
	 * @brief					释放各资源
	 */
	void						Release();

public:
	/**
	 * @brief					将所有数据同步/初始化到所有客户端链路
	 * @param[in]				refDatabaseIO			数据库插件引用
	 * @param[in]				nSerialNo				推送查询序号(需要>nSerialNo)
	 * @return					>=0						同步的链路数
								<0						出错
	 */
	int							Flush2ReqSessions( DatabaseIO& refDatabaseIO, unsigned __int64 nSerialNo = 0 );

	/**
	 * @brief					获取待初始化的新链路数量
	 */
	unsigned int				GetReqSessionCount();

	/**
	 * @brief					增加一个新的待初始化推送的链路号
	 * @param[in]				nLinkNo					链路号
	 * @return					true					增加成功
								false					失败，有重复项
	 */
	bool						AddNewReqSession( unsigned int nLinkNo );

protected:
	CriticalObject				m_oBuffLock;			///< 初始化数据推送缓存锁
	std::set<unsigned int>		m_setNewReqLinkID;		///< 待初始化链路ID集合
	char*						m_pImageDataBuffer;		///< 初始化数据缓存
};




#endif



