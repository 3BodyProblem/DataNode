#ifndef __DATE_NODE_INTERFACE_H__
#define	__DATE_NODE_INTERFACE_H__


/**
 * @class					I_DataHandle
 * @brief					数据回调接口
 * @date					2017/5/3
 * @author					barry
 */
class I_DataHandle
{
public:
	/**
 	 * @brief				初始化性质的行情数据回调
	 * @note				只是更新构造好行情数据的内存初始结构，不推送
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @param[in]			bLastFlag			是否所有初始化数据已经发完，本条为最后一条的，标识
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnImage( unsigned int nDataID, char* pData, unsigned int nDataLen, bool bLastFlag ) = 0;

	/**
	 * @brief				实行行情数据回调
	 * @note				更新行情内存块，并推送
	 * @param[in]			nDataID				消息ID
	 * @param[in]			pData				数据内容
	 * @param[in]			nDataLen			长度
	 * @return				==0					成功
							!=0					错误
	 */
	virtual int				OnData( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;

	/**
	 * @brief				根据消息ID/Code查询某数据内容(通过内存数据插件接口)
	 * @param[in]			nDataID				消息ID
	 * @param[in,out]		pData				商品Code[in],数据查询返回[out]
	 * @param[in]			nDataLen			缓存长度
	 * @return				true				查询成功
	 */
	virtual bool			OnQuery( unsigned int nDataID, char* pData, unsigned int nDataLen ) = 0;
};

















/**
 * @classs							RecordBlock
 * @brief							记录数据块类
 * @author							barry
 */
class RecordBlock
{
public:
	RecordBlock();
	RecordBlock( const RecordBlock& record );
	RecordBlock( char* pRecord, unsigned int nRecordLen );

public:
	/**
	 * @brief						记录对象为无效
	 * @return						true							无效对象
	 */
	bool							IsNone() const;

	/**
	 * @brief						记录体比较函数
	 * @param[in]					refRecord						比较标的对象
	 * @return						true							相同
									false							不同
	 */
	bool							Compare( const RecordBlock& refRecord );

	/**
	 * @brief						记录体deeply copy
	 * @param[in]					refRecord						内存copy的源头
	 * @return						==1								影响的数量
									==0								不需要copy，目标和源头数据完全一样
									<0								出现错误
	 */
	int								CloneFrom( const RecordBlock& refRecord );

public:
	/**
	 * @brief						取得数据地址
	 */
	const char*						GetPtr() const;

	/**
	 * @brief						数据长度
	 */
	unsigned int					Length() const;

protected:
	char*							m_pRecordData;					///< 数据结构地址
	unsigned int					m_nRecordLen;					///< 数据结构长度
};


/**
 * @class							I_Table
 * @brief							数据表操作接口
 * @author							barry
 */
class I_Table
{
public:
	/**
	 * @brief						追加新数据
	 * @param[in]					pRecord					记录体地址
	 * @param[in]					nRecordLen				记录体长度
	 * @return						>0						增加成功
									=0						记录已经存在，不需要增加
									<0						失败
	 */
	virtual int						InsertRecord( char* pRecord, unsigned int nRecordLen ) = 0; 

	/**
	 * @brief						更新数据
	 * @param[in]					pRecord					记录体地址
	 * @param[in]					nRecordLen				记录体长度
	 * @return						>0						增加成功
									=0						记录已经存在，不需要增加
									<0						失败
	 */
	virtual int						UpdateRecord( char* pRecord, unsigned int nRecordLen ) = 0; 

	/**
	 * @brief						索引出记录对象
	 * @param[in]					pKeyStr					主键地址
	 * @param[in]					nKeyLen					主键长度
	 * @return						返回记录对象
	 */
	virtual RecordBlock				SelectRecord( char* pKeyStr, unsigned int nKeyLen ) = 0;
};


/**
 * @class							I_Database
 * @brief							数据库操作接口
 * @author							barry
 */
class I_Database
{
public:
	virtual ~I_Database(){};

	/**
	 * @brief						根据消息id和消息长度，进行合适的数据表配置（在预备表中配置对应的占用关系）
	 * @param[in]					nBindID				数据类形标识号
	 * @param[in]					nRecordWidth		数据长度
	 * @param[in]					nKeyStrLen			主键长度
	 * @return						=0					配置成功
									>0					忽略（成功）
									<0					配置出错
	 */
	virtual bool					CreateTable( unsigned int nBindID, unsigned int nRecordWidth, unsigned int nKeyStrLen ) = 0;

	/**
	 * @brief						根据MessageID取得已经存在的或者分配一个新的内存表的引用
	 * @detail						本函数对每个messageid维护一个唯一且对应的内存表，根据nBindID值返回已经存在的，或新建后返回
	 * @param[in]					nBindID				MessageID
	 * @return						返回已经存在的内存表或新建的内存表
	 */
	virtual I_Table*				QueryTable( unsigned int nBindID ) = 0;

	/**
	 * @brief						清理所有数据表
	 */
	virtual void					DeleteTables() = 0;

	/**
	 * @brief						从硬盘恢复所有数据
	 */
	virtual bool					LoadFromDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						将所有数据存盘
	 */
	virtual bool					SaveToDisk( const char* pszDataFile ) = 0;
};


/**
 * @class							IDBFactory
 * @brief							内存数据分配管理工厂类接口
 * @author							barry
 */
class IDBFactory
{
public:
	/**
	 * @brief						创建并返回数据库对象指针
	 * @return						返回数据库指针的地址
	 */
	virtual I_Database*				GrapDatabaseInterface() = 0;

	/**
	 * @brief						释放分配的所有数据库对象
	 */
	virtual bool					ReleaseAllDatabase() = 0;

};





#endif









