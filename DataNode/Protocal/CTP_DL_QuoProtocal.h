#ifndef __CTP_QUOTATION_PROTOCAL_CTP_DL_H__
#define	__CTP_QUOTATION_PROTOCAL_CTP_DL_H__


typedef struct
{
	char						Code[20];					//��Լ����
	char						Name[40];					//��Լ����
	unsigned long				LotFactor;					//��
	char						Type;						//����
	//	unsigned long			PreClosePx;					//��Լ����(������Ȩ��Ϣ��Ϊ����������̼۸�)(��ȷ����)//[*�Ŵ���]
	//	unsigned long			PreSettlePx;				//��Լ���//[*�Ŵ���]
	char						Filler[8];
	unsigned char				ObjectMId;					//�Ϻ��ڻ� 0  �����ڻ� 1  ֣���ڻ� 2 �Ϻ���Ȩ 3  ������Ȩ 4  ֣����Ȩ 5
	char						UnderlyingCode[6];			//���֤ȯ����
	char						PriceLimitType;				//�ǵ�����������(N ���ǵ���)(R ���ǵ���)
	int							LotSize;					//һ�ֵ��ڼ��ź�Լ
	unsigned long				ContractMult;				//��Լ����
	unsigned long				XqPrice;					//��Ȩ�۸�(��ȷ����) //[*�Ŵ���] 
	int							StartDate;					//�׸�������(YYYYMMDD)
	int							EndDate;					//�������(YYYYMMDD)
	int							XqDate;						//��Ȩ��(YYYYMM)
	int							DeliveryDate;				//������(YYYYMMDD)
	int							ExpireDate;					//������(YYYYMMDD)
	unsigned short				TypePeriodIdx;             // ���ཻ��ʱ���λ��
	unsigned char				EarlyNightFlag;             //����orҹ�̱�־ 1������ 2��ҹ�� 
} tagCTPReferenceData;


typedef struct
{
	char						Code[20];					//��Լ����
	int							LeavesQty;					//δƽ�ֺ�Լ�� = ��ֲ� ��λ��(��)
	unsigned long				UpLimit;					//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
	unsigned long				DownLimit;					//������Ȩ��ͣ�۸�(��ȷ����) //[*�Ŵ���]
} tagCTPRefParameter;


typedef struct
{
	unsigned long					Price;					//ί�м۸�[* �Ŵ���]
	unsigned __int64				Volume;					//ί����[��]
} tagCcComm_BuySell;


typedef struct
{
    char						Code[20];				//��Ʒ����<2009-1-21 Lumy����>
    unsigned long				High;                   //��߼۸�[* �Ŵ���]
    unsigned long				Open;                   //���̼۸�[* �Ŵ���]
    unsigned long				Low;                    //��ͼ۸�[* �Ŵ���]
    unsigned long				PreClose;               //���ռ۸�[* �Ŵ���]
    unsigned long				PreSettlePrice;         //���ս���۸�[* �Ŵ���]
    unsigned long				Now;                    //���¼۸�[* �Ŵ���]
    unsigned long				Close;                  //�������̼۸�[* �Ŵ���]
    unsigned long				SettlePrice;            //���ս���۸�[* �Ŵ���]
    unsigned long				UpperPrice;             //��ͣ�۸�[* �Ŵ���]
    unsigned long				LowerPrice;             //��ͣ�۸�[* �Ŵ���]
    double						Amount;                 //�ܳɽ����[Ԫ]
    unsigned __int64			Volume;                 //�ܳɽ���[��]
    unsigned __int64			PreOpenInterest;        //���ճֲ���[��]
    unsigned __int64			OpenInterest;           //�ֲ���[��]
    tagCcComm_BuySell			Buy[5];                 //ί������
    tagCcComm_BuySell			Sell[5];                //ί������
	unsigned short              Date;                   //����(mmdd)
	unsigned int				DataTimeStamp;			//ʱ���(HHMMSSmmm)
} tagCTPSnapData;





#endif









