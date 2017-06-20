#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#pragma pack(1)


typedef struct
{
	char						Key[32];					///< ������ֵ
	unsigned int				MarketID;					///< �г����
	unsigned int				KindCount;					///< �������
	unsigned int				WareCount;					///< ��Ʒ����
	unsigned int				PeriodsCount;				///< ����ʱ����Ϣ�б���
	unsigned int				MarketPeriods[8][2];		///< ����ʱ��������Ϣ�б�
} tagDLMarketInfo_LF1000;


typedef struct
{
	char						Key[20];					///< ������ֵ
	char						KindName[64];				///< ��������
	unsigned int				PriceRate;					///< �۸�Ŵ���[10�Ķ��ٴη�]
	unsigned int				LotFactor;					///< ���֡�����
	unsigned int				WareCount;					///< �÷������Ʒ����
} tagDLKindDetail_LF1001;


typedef struct
{
	char						Key[20];					///< ������ֵ
	unsigned char				IsHoliday;					///< �����ǽڼ��ձ�־��0��ʾ�ǣ�1��ʾ��
	unsigned int				Date;						///< ���ڣ�������ı�־���Ƹ������Ƿ�Ϊ�ڼ���
} tagDLHolidayTable_LF1002;


typedef struct
{
	char						Code[20];					///< ��Լ����
	char						Name[64];					///< ��Լ����
	unsigned char				Kind;						///< ֤ȯ����
	unsigned char				DerivativeType;				///< ����Ʒ���ͣ�ŷʽ��ʽ+�Ϲ��Ϲ�
	unsigned int				LotSize;					///< һ�ֵ��ڼ��ź�Լ
	char						UnderlyingCode[20];			///< ���֤ȯ����
	unsigned int				ContractMult;				///< ��Լ����
	unsigned int				XqPrice;					///< ��Ȩ�۸�[*�Ŵ���]
	unsigned int				StartDate;					///< �׸�������(YYYYMMDD)
	unsigned int				EndDate;					///< �������(YYYYMMDD)
	unsigned int				XqDate;						///< ��Ȩ��(YYYYMM)
	unsigned int				DeliveryDate;				///< ������(YYYYMMDD)
	unsigned int				ExpireDate;					///< ������(YYYYMMDD)
	unsigned short				TypePeriodIdx;				///< ���ཻ��ʱ���λ��
	unsigned char				EarlyNightFlag;             ///< ����orҹ�̱�־ 1������ 2��ҹ�� 
	unsigned int				PriceTick;					///< ��С�䶯��λ
} tagDLReferenceData_LF1003;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				Open;						///< ���̼�[*�Ŵ���]
	unsigned int				Close;						///< ���ռ�[*�Ŵ���]
	unsigned int				PreClose;					///< ���ռ�[*�Ŵ���]
	unsigned int				UpperPrice;					///< ������ͣ�۸�[*�Ŵ���], 0��ʾ������
	unsigned int				LowerPrice;					///< ���յ�ͣ�۸�[*�Ŵ���], 0��ʾ������
	unsigned int				SettlePrice;				///< ����[*�Ŵ���]
	unsigned int				PreSettlePrice;				///< ��Լ���[*�Ŵ���]
	unsigned __int64			PreOpenInterest;			///< ���ճֲ���(��)
} tagDLSnapData_LF1004;


typedef struct
{
	char						Code[20];					///< ��Լ����
	unsigned int				Now;						///< ���¼�[*�Ŵ���]
	unsigned int				High;						///< ��߼�[*�Ŵ���]
	unsigned int				Low;						///< ��ͼ�[*�Ŵ���]
	double						Amount;						///< �ܳɽ����[Ԫ]
	unsigned __int64			Volume;						///< �ܳɽ���[��/��]
	unsigned __int64			Position;					///< �ֲ���
} tagDLSnapData_HF1005;


typedef struct
{
	unsigned int				Price;						///< ί�м۸�[* �Ŵ���]
	unsigned __int64			Volume;						///< ί����[��]
} tagBuySellItem;


typedef struct
{
	char						Code[20];					///< ��Լ����
	tagBuySellItem				Buy[5];						///< ���嵵
	tagBuySellItem				Sell[5];					///< ���嵵
} tagDLSnapBuySell_HF1006;


typedef struct
{
	char						Key[32];					///< ������ֵ
	unsigned int				MarketDate;					///< �г�����
	unsigned int				MarketTime;					///< �г�ʱ��
	unsigned char				MarketStatus;				///< �г�״̬[0��ʼ�� 1������]
} tagDLMarketStatus_HF1007;




#pragma pack()
#endif









