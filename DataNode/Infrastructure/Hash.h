#ifndef __INFRASTRUCTURE__HASH_H__
#define __INFRASTRUCTURE__HASH_H__


#include <exception>
#include <functional>
#include <algorithm>


/**
 * @class			UInt2UIntHash
 * @brief			整型值到位置值的哈希映射表
 * @note			默认值的情况下，数据表长度是哈希桶长度的3倍(无锁)
 * @author			barry
 * @date			2017/4/2
 */
template<typename T_KEY_TYPE, typename T_VALUE_TYPE, const unsigned int MAX_BUCKET_SIZE=1024, const unsigned int MAX_DATATABLE_NUM=MAX_BUCKET_SIZE*3>
class CollisionHash
{
	struct T_ListNode
	{
	public:
		T_ListNode() : nDataPos( -1 ), pNextNode( NULL ), nHashKey( 0 ) {};
		bool			IsNull()	{	return nDataPos < 0;	}		///< 节点是否有有效值
		bool			HasNext()	{	return pNextNode != NULL;	}	///< 节点是否有下一个值
		void			Clear()		{	nDataPos = -1; pNextNode = NULL; nHashKey = 0;	}
	public:
		T_KEY_TYPE		nHashKey;									///< 健值保存
		int				nDataPos;									///< 数据所在的索引位置
		void*			pNextNode;									///< 具有相同key的下一个值的索引位置
	};
public:
	CollisionHash();

	/**
	 * @brief			根据key取得对应的映射值
	 * @param[in]		nKey		索引值
	 * @return			返回映射值
	 * @note			如果key不存在，则抛出runtime异常
	 */
	const T_VALUE_TYPE	operator[]( T_KEY_TYPE nKey );

	/**
	 * @brief			设置键值对
	 * @param[in]		nKey		键值
	 * @param[in\		oData		数值
	 * @return			==1			设置成功
						==0			已经存在，不需要新建
						<0			失败
	 */
	int					NewKey( T_KEY_TYPE nKey, T_VALUE_TYPE oData );

	/**
	 * @brief			清空所有数据
	 */
	void				Clear();

private:
	T_ListNode			m_BucketOfHash[MAX_BUCKET_SIZE];			///< 哈稀桶
	T_ListNode			m_CollisionBucket[MAX_DATATABLE_NUM];		///< 哈稀碰撞桶
	T_VALUE_TYPE		m_ArrayOfData[MAX_DATATABLE_NUM];			///< 数据缓存表
	unsigned int		m_nUsedNumOfCollisionBucket;				///< 已经使用的节点数量(碰撞桶)
	unsigned int		m_nUsedNumOfArrayData;						///< 已经使用的数据缓存节点数据
};


template<typename T_KEY_TYPE, typename T_VALUE_TYPE, const unsigned int MAX_BUCKET_SIZE, const unsigned int MAX_DATATABLE_NUM>
CollisionHash<T_KEY_TYPE,T_VALUE_TYPE,MAX_BUCKET_SIZE,MAX_DATATABLE_NUM>::CollisionHash()
 : m_nUsedNumOfCollisionBucket( 0 ), m_nUsedNumOfArrayData( 0 )
{
	Clear();
}

template<typename T_KEY_TYPE, typename T_VALUE_TYPE, const unsigned int MAX_BUCKET_SIZE, const unsigned int MAX_DATATABLE_NUM>
void CollisionHash<T_KEY_TYPE,T_VALUE_TYPE,MAX_BUCKET_SIZE,MAX_DATATABLE_NUM>::Clear()
{
	m_nUsedNumOfArrayData = 0;
	m_nUsedNumOfCollisionBucket = 0;

	for( unsigned int n = 0; n < MAX_DATATABLE_NUM; n++ )
	{
		m_ArrayOfData[n] = T_VALUE_TYPE();
	}

	std::for_each( m_BucketOfHash, m_BucketOfHash+MAX_BUCKET_SIZE, std::mem_fun_ref(&T_ListNode::Clear) );
	std::for_each( m_CollisionBucket, m_CollisionBucket+MAX_DATATABLE_NUM, std::mem_fun_ref(&T_ListNode::Clear) );
}

template<typename T_KEY_TYPE, typename T_VALUE_TYPE, const unsigned int MAX_BUCKET_SIZE, const unsigned int MAX_DATATABLE_NUM>
int CollisionHash<T_KEY_TYPE,T_VALUE_TYPE,MAX_BUCKET_SIZE,MAX_DATATABLE_NUM>::NewKey( T_KEY_TYPE nKey, T_VALUE_TYPE oData )
{
	T_KEY_TYPE				nKeyPos = nKey % MAX_BUCKET_SIZE;
	struct T_ListNode*		pNode = m_BucketOfHash + nKeyPos;

	if( m_nUsedNumOfArrayData >= (MAX_DATATABLE_NUM-1) )
	{
		throw std::runtime_error( "CollisionHash::NewKey() : data buffer is full." );
	}

	if( true == pNode->IsNull() )
	{
		m_ArrayOfData[m_nUsedNumOfArrayData] = oData;
		pNode->nHashKey = nKey;
		pNode->nDataPos = m_nUsedNumOfArrayData++;
		return 1;
	}

	while( true )
	{
		if( nKey == pNode->nHashKey )
		{
			return 0;
		}

		if( false == pNode->HasNext() )
		{
			struct T_ListNode* pNewNodeOfCollision = m_CollisionBucket+m_nUsedNumOfCollisionBucket++;

			m_ArrayOfData[m_nUsedNumOfArrayData] = oData;
			pNewNodeOfCollision->nHashKey = nKey;
			pNewNodeOfCollision->nDataPos = m_nUsedNumOfArrayData++;
			pNode->pNextNode = pNewNodeOfCollision;

			return 1;
		}
		else
		{
			pNode = (struct T_ListNode*)(pNode->pNextNode);
		}
	}

	return -1;
}

template<typename T_KEY_TYPE, typename T_VALUE_TYPE, const unsigned int MAX_BUCKET_SIZE, const unsigned int MAX_DATATABLE_NUM>
const T_VALUE_TYPE CollisionHash<T_KEY_TYPE,T_VALUE_TYPE,MAX_BUCKET_SIZE,MAX_DATATABLE_NUM>::operator[]( T_KEY_TYPE nKey )
{
	T_KEY_TYPE				nKeyPos = nKey % MAX_BUCKET_SIZE;
	struct T_ListNode*		pNode = m_BucketOfHash + nKeyPos;

	while( false == pNode->IsNull() )
	{
		if( nKey == pNode->nHashKey )
		{
			return m_ArrayOfData[pNode->nDataPos];
		}
		else if( true == pNode->HasNext() )
		{
			pNode = (struct T_ListNode*)(pNode->pNextNode);
		}
		else
		{
			break;
		}
	}

	return T_VALUE_TYPE();
}

#endif






