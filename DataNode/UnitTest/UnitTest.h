#ifndef __QLX_UNIT_TEST_H__
#define __QLX_UNIT_TEST_H__
#pragma warning(disable : 4996)
#pragma warning(disable : 4204)


#include <vector>
#include "gtest/gtest.h"
#include "../Interface.h"


///< --------------------- 单元测试类定义 --------------------------------


/**
 * @class							TestTableOperation
 * @brief							测试数据表操作业务
 * @author							barry
 */
class TestTableOperation : public testing::Test
{
public:

protected:
	static	void										SetUpTestCase();
	static	void										TearDownTestCase();
	void												SetUp();				///< 预告建立所有需要的数据表
	void												TearDown();
};


///< ------------ 单元测试初始化类定义 ------------------------------------


/**
 * @class					QLXEnDeCodeTestEnv
 * @brief					全局事件(初始化引擎)
 * @author					barry
 */
class UnitTestEnv : public testing::Environment
{
public:
	void					SetUp();
	void					TearDown();

private:

};


///< ------------ 单元测试初始化类定义 ------------------------------------


/**
 * @brief					运行所有的测试例程
 * @date					2017/5/3
 * @author					barry
 */
void ExecuteTestCase();



#endif





