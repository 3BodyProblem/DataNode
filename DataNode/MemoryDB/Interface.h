#ifndef __MEM_DATABASE_INTERFACE_H__
#define	__MEM_DATABASE_INTERFACE_H__


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
	 * @param[out]					nDbSerialNo				数据库新增，更新操作流水号
	 * @return						>0						增加成功
									=0						记录已经存在，不需要增加
									<0						失败
	 */
	virtual int						InsertRecord( char* pRecord, unsigned int nRecordLen, unsigned __int64& nDbSerialNo ) = 0;

	/**
	 * @brief						更新数据
	 * @param[in]					pRecord					记录体地址
	 * @param[in]					nRecordLen				记录体长度
	 * @param[out]					nDbSerialNo				数据库新增，更新操作流水号
	 * @return						>0						增加成功
									=0						记录已经存在，不需要增加
									<0						失败
	 */
	virtual int						UpdateRecord( char* pRecord, unsigned int nRecordLen, unsigned __int64& nDbSerialNo ) = 0; 

	/**
	 * @brief						索引出记录对象
	 * @param[in]					pKeyStr					主键地址
	 * @param[in]					nKeyLen					主键长度
	 * @return						返回记录对象
	 */
	virtual RecordBlock				SelectRecord( char* pKeyStr, unsigned int nKeyLen ) = 0;

	/**
	 * @brief						删除某条记录
	 * @param[in]					pKeyStr					主键地址
	 * @param[in]					nKeyLen					主键长度
	 * @param[in,out]				nDbSerialNo				取出>nDbSerialNo的数据(若为0,则全部取出) & 将回填最新的流水号
	 * @return						>0						返回被操作到的记录数
									==0						未有记录被删除
									<0						删除时出现错误
	 */
	virtual int						DeleteRecord( char* pKeyStr, unsigned int nKeyLen, unsigned __int64& nDbSerialNo ) = 0;

	/**
	 * @brief						将数据表的数据原样copy到缓存
	 * @param[in]					pBuffer					缓存地址
	 * @param[in]					nBufferSize				缓存长度
	 * @param[in,out]				nDbSerialNo				取出>nDbSerialNo的数据(若为0,则全部取出) & 将回填最新的流水号
	 * @return						>=0						返回数据长度
									<						出错
	 */
	virtual int						CopyToBuffer( char* pBuffer, unsigned int nBufferSize, unsigned __int64& nDbSerialNo ) = 0;
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
	 * @brief						删除指定的数据表
	 * @param[in]					nBindID				数据表ID
	 * @return						true				删除成功
	 */
	virtual bool					DeleteTable( unsigned int nBindID ) = 0;

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
	virtual int						DeleteTables() = 0;

	/**
	 * @brief						从硬盘恢复所有数据
	 * @return						>0					数据库加载落盘文件日期
									<0					出错
	 */
	virtual int						LoadFromDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						将所有数据存盘
	 */
	virtual bool					SaveToDisk( const char* pszDataFile ) = 0;

	/**
	 * @brief						取得数据更新自增流水号
	 */
	virtual unsigned __int64		GetUpdateSequence() = 0;

	/**
	 * @brief						取得数据表的数量
	 * @return						返回统计值
	 */
	virtual unsigned int			GetTableCount() = 0;

	/**
	 * @brief						根据位置索引取得数据表元信息
	 * @param[in]					想获取数据表的位置
	 * @param[out]					nDataID				数据表ID
	 * @param[out]					nRecordLen			数据表记录长度
	 * @param[out]					nKeyStrLen			数据主键最大长度
	 * @return						true				获取成功
									false				获取出错
	 */
	virtual bool					GetTableMetaByPos( unsigned int nPos, unsigned int& nDataID, unsigned int& nRecordLen, unsigned int& nKeyStrLen ) = 0;
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









