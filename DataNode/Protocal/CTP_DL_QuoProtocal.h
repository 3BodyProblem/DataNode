#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__


typedef struct
{
	char						Code[20];					//合约代码
	char						Name[40];					//合约名称
	unsigned long				LotFactor;					//手
	char						Type;						//分类
	//	unsigned long			PreClosePx;					//合约昨收(如遇除权除息则为调整后的收盘价格)(精确到厘)//[*放大倍数]
	//	unsigned long			PreSettlePx;				//合约昨结//[*放大倍数]
	char						Filler[8];
	unsigned char				ObjectMId;					//上海期货 0  大连期货 1  郑州期货 2 上海期权 3  大连期权 4  郑州期权 5
	char						UnderlyingCode[6];			//标的证券代码
	char						PriceLimitType;				//涨跌幅限制类型(N 有涨跌幅)(R 无涨跌幅)
	int							LotSize;					//一手等于几张合约
	unsigned long				ContractMult;				//合约乘数
	unsigned long				XqPrice;					//行权价格(精确到厘) //[*放大倍数] 
	int							StartDate;					//首个交易日(YYYYMMDD)
	int							EndDate;					//最后交易日(YYYYMMDD)
	int							XqDate;						//行权日(YYYYMM)
	int							DeliveryDate;				//交割日(YYYYMMDD)
	int							ExpireDate;					//到期日(YYYYMMDD)
	unsigned short				TypePeriodIdx;             // 分类交易时间段位置
	unsigned char				EarlyNightFlag;             //日盘or夜盘标志 1：日盘 2：夜盘 
} tagCTPReferenceData;


typedef struct
{
	char						Code[20];					//合约代码
	int							LeavesQty;					//未平仓合约数 = 昨持仓 单位是(张)
	unsigned long				UpLimit;					//当日期权涨停价格(精确到厘) //[*放大倍数]
	unsigned long				DownLimit;					//当日期权跌停价格(精确到厘) //[*放大倍数]
} tagCTPRefParameter;


typedef struct
{
	unsigned long					Price;					//委托价格[* 放大倍数]
	unsigned __int64				Volume;					//委托量[股]
} tagCcComm_BuySell;


typedef struct
{
    char						Code[20];				//商品代码<2009-1-21 Lumy更改>
    unsigned long				High;                   //最高价格[* 放大倍数]
    unsigned long				Open;                   //开盘价格[* 放大倍数]
    unsigned long				Low;                    //最低价格[* 放大倍数]
    unsigned long				PreClose;               //昨收价格[* 放大倍数]
    unsigned long				PreSettlePrice;         //昨日结算价格[* 放大倍数]
    unsigned long				Now;                    //最新价格[* 放大倍数]
    unsigned long				Close;                  //今日收盘价格[* 放大倍数]
    unsigned long				SettlePrice;            //今日结算价格[* 放大倍数]
    unsigned long				UpperPrice;             //涨停价格[* 放大倍数]
    unsigned long				LowerPrice;             //跌停价格[* 放大倍数]
    double						Amount;                 //总成交金额[元]
    unsigned __int64			Volume;                 //总成交量[股]
    unsigned __int64			PreOpenInterest;        //昨日持仓量[股]
    unsigned __int64			OpenInterest;           //持仓量[股]
    tagCcComm_BuySell			Buy[5];                 //委托买盘
    tagCcComm_BuySell			Sell[5];                //委托卖盘
	unsigned short              Date;                   //日期(mmdd)
	unsigned int				DataTimeStamp;			//时间戳(HHMMSSmmm)
} tagCTPSnapData;





#endif









