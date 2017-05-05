#ifndef __INFRASTRUCTURE__HASH_H__
#define __INFRASTRUCTURE__HASH_H__


#include <exception>
#include <functional>
#include <algorithm>


/**
 * @class			UInt2UIntHash
 * @brief			����ֵ��λ��ֵ�Ĺ�ϣӳ���
 * @note			Ĭ��ֵ������£����ݱ����ǹ�ϣͰ���ȵ�3��(����)
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
		bool			IsNull()	{	return nDataPos < 0;	}		///< �ڵ��Ƿ�����Чֵ
		bool			HasNext()	{	return pNextNode != NULL;	}	///< �ڵ��Ƿ�����һ��ֵ
		void			Clear()		{	nDataPos = -1; pNextNode = NULL; nHashKey = 0;	}
	public:
		T_KEY_TYPE		nHashKey;									///< ��ֵ����
		int				nDataPos;									///< �������ڵ�����λ��
		void*			pNextNode;									///< ������ͬkey����һ��ֵ������λ��
	};
public:
	CollisionHash();

	/**
	 * @brief			����keyȡ�ö�Ӧ��ӳ��ֵ
	 * @param[in]		nKey		����ֵ
	 * @return			����ӳ��ֵ
	 * @note			���key�����ڣ����׳�runtime�쳣
	 */
	const T_VALUE_TYPE	operator[]( T_KEY_TYPE nKey );

	/**
	 * @brief			���ü�ֵ��
	 * @param[in]		nKey		��ֵ
	 * @param[in\		oData		��ֵ
	 * @return			==1			���óɹ�
						==0			�Ѿ����ڣ�����Ҫ�½�
						<0			ʧ��
	 */
	int					NewKey( T_KEY_TYPE nKey, T_VALUE_TYPE oData );

	/**
	 * @brief			�����������
	 */
	void				Clear();

private:
	T_ListNode			m_BucketOfHash[MAX_BUCKET_SIZE];			///< ��ϡͰ
	T_ListNode			m_CollisionBucket[MAX_DATATABLE_NUM];		///< ��ϡ��ײͰ
	T_VALUE_TYPE		m_ArrayOfData[MAX_DATATABLE_NUM];			///< ���ݻ����
	unsigned int		m_nUsedNumOfCollisionBucket;				///< �Ѿ�ʹ�õĽڵ�����(��ײͰ)
	unsigned int		m_nUsedNumOfArrayData;						///< �Ѿ�ʹ�õ����ݻ���ڵ�����
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






