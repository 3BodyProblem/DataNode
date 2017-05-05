#ifndef __DATA_TABLE_PAINTER_H__
#define	__DATA_TABLE_PAINTER_H__


/**
 * @brief				DLL导出接口
 * @author				barry
 * @date				2017/4/1
 */
extern "C"
{
	/**
	 * @brief					运行节点服务器
	 * @return					==0				正常运行结束
								!=0				运行异常结束
	 */
	int							RunNodeServer();

	/**
	 * @brief					单元测试导出函数
	 */
	void						ExecuteUnitTest();
}




#endif





