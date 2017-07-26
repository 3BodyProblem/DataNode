#ifndef __QLX_UNIT_TEST_H__
#define __QLX_UNIT_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "gtest/gtest.h"
#include "../Interface.h"


///< --------------------- ��Ԫ�����ඨ�� --------------------------------


/**
 * @class							TestLogic
 * @brief							�����߼�ҵ��
 * @author							barry
 */
class TestLogic : public testing::Test
{
protected:
	static	void										SetUpTestCase();
	static	void										TearDownTestCase();
	void												SetUp();				///< Ԥ�潨��������Ҫ�����ݱ�
	void												TearDown();
};


#pragma pack(1)


typedef struct
{
	char						Key[20];					///< ������ֵ
	unsigned int				MarketID;					///< �г����
	unsigned int				SeqNo;						///< ���
	char						CnName[64];					///< ������
} tagTestMessageStruct;


#pragma pack()


/**
 * @class							TestLoopBuffer
 * @brief							���Ի����߼�
 * @author							barry
 */
class TestLoopBuffer : public testing::Test
{
protected:
	static	void										SetUpTestCase();
	static	void										TearDownTestCase();
	void												SetUp();				///< Ԥ�潨��������Ҫ�����ݱ�
	void												TearDown();

protected:
	static std::vector<tagTestMessageStruct>			m_vctMsg1;
	static std::vector<tagTestMessageStruct>			m_vctMsg2;
};



///< ------------ ��Ԫ���Գ�ʼ���ඨ�� ------------------------------------


/**
 * @class					QLXEnDeCodeTestEnv
 * @brief					ȫ���¼�(��ʼ������)
 * @author					barry
 */
class UnitTestEnv : public testing::Environment
{
public:
	void					SetUp();
	void					TearDown();

private:

};


///< ------------ ��Ԫ���Գ�ʼ���ඨ�� ------------------------------------


/**
 * @brief					�������еĲ�������
 * @date					2017/5/3
 * @author					barry
 */
void ExecuteTestCase();



#endif





