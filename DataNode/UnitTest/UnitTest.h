#ifndef __QLX_UNIT_TEST_H__
#define __QLX_UNIT_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "gtest/gtest.h"
#include "../Interface.h"


///< --------------------- ��Ԫ�����ඨ�� --------------------------------


/**
 * @class							TestTableOperation
 * @brief							�������ݱ����ҵ��
 * @author							barry
 */
class TestTableOperation : public testing::Test
{
public:

protected:
	static	void										SetUpTestCase();
	static	void										TearDownTestCase();
	void												SetUp();				///< Ԥ�潨��������Ҫ�����ݱ�
	void												TearDown();
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





