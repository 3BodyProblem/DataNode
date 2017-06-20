#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


typedef struct
{
	char						Key[32];					///< 索引键值
	unsigned int				MarketID;					///< 市场编号
	unsigned int				KindCount;					///< 类别数量
	unsigned int				WareCount;					///< 商品数量
	unsigned int				PeriodsCount;				///< 交易时段信息列表长度
	unsigned int				MarketPeriods[8][2];		///< 交易时段描述信息列表
} tagDLMarketInfo_LF1000;


typedef struct
{
	char						Key[20];					///< 索引键值
	char						KindName[64];				///< 类别的名称
	unsigned int				PriceRate;					///< 价格放大倍数[10的多少次方]
	unsigned int				LotFactor;					///< “手”比率
	unsigned int				WareCount;					///< 该分类的商品数量
} tagDLKindDetail_LF1001;


typedef struct
{
	char						Key[20];					///< 索引键值
	unsigned char				IsHoliday;					///< 日期是节假日标志，0表示非，1表示是
	unsigned int				Date;						///< 日期，由上面的标志控制该日期是否为节假日
} tagDLHolidayTable_LF1002;


typedef struct
{
	char						Code[20];					///< 合约代码
	char						Name[64];					///< 合约名称
	unsigned char				Kind;						///< 证券类型
	unsigned char				DerivativeType;				///< 衍生品类型：欧式美式+认购认沽
	unsigned int				LotSize;					///< 一手等于几张合约
	char						UnderlyingCode[20];			///< 标的证券代码
	unsigned int				ContractMult;				///< 合约乘数
	unsigned int				XqPrice;					///< 行权价格[*放大倍数]
	unsigned int				StartDate;					///< 首个交易日(YYYYMMDD)
	unsigned int				EndDate;					///< 最后交易日(YYYYMMDD)
	unsigned int				XqDate;						///< 行权日(YYYYMM)
	unsigned int				DeliveryDate;				///< 交割日(YYYYMMDD)
	unsigned int				ExpireDate;					///< 到期日(YYYYMMDD)
	unsigned short				TypePeriodIdx;				///< 分类交易时间段位置
	unsigned char				EarlyNightFlag;             ///< 日盘or夜盘标志 1：日盘 2：夜盘 
	unsigned int				PriceTick;					///< 最小变动价位
} tagDLReferenceData_LF1003;


typedef struct
{
	char						Code[20];					///< 合约代码
	unsigned int				Open;						///< 开盘价[*放大倍数]
	unsigned int				Close;						///< 今收价[*放大倍数]
	unsigned int				PreClose;					///< 昨收价[*放大倍数]
	unsigned int				UpperPrice;					///< 当日涨停价格[*放大倍数], 0表示无限制
	unsigned int				LowerPrice;					///< 当日跌停价格[*放大倍数], 0表示无限制
	unsigned int				SettlePrice;				///< 今结价[*放大倍数]
	unsigned int				PreSettlePrice;				///< 合约昨结[*放大倍数]
	unsigned __int64			PreOpenInterest;			///< 昨日持仓量(张)
} tagDLSnapData_LF1004;


typedef struct
{
	char						Code[20];					///< 合约代码
	unsigned int				Now;						///< 最新价[*放大倍数]
	unsigned int				High;						///< 最高价[*放大倍数]
	unsigned int				Low;						///< 最低价[*放大倍数]
	double						Amount;						///< 总成交金额[元]
	unsigned __int64			Volume;						///< 总成交量[股/张]
	unsigned __int64			Position;					///< 持仓量
} tagDLSnapData_HF1005;


typedef struct
{
	unsigned int				Price;						///< 委托价格[* 放大倍数]
	unsigned __int64			Volume;						///< 委托量[股]
} tagBuySellItem;


typedef struct
{
	char						Code[20];					///< 合约代码
	tagBuySellItem				Buy[5];						///< 买五档
	tagBuySellItem				Sell[5];					///< 卖五档
} tagDLSnapBuySell_HF1006;


typedef struct
{
	char						Key[32];					///< 索引键值
	unsigned int				MarketDate;					///< 市场日期
	unsigned int				MarketTime;					///< 市场时间
	unsigned char				MarketStatus;				///< 市场状态[0初始化 1行情中]
} tagDLMarketStatus_HF1007;




#pragma pack()
#endif









