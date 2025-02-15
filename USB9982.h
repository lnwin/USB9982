//AD��ʼ������
#ifndef _USB9982_PARA_INIT// USB9982.h
#include <windows.h>  // ȷ������LONG�Ķ���
typedef struct _USB9982_PARA_INIT    
{
	LONG    lSelADClk;       //Ԥ��
	LONG    lChCnt;          //ͨ��ѡ��
	LONG    ClkDeci;         //Ԥ��
	LONG	TriggerMode;     //����ģʽ
	LONG	TriggerSource;	 //����Դ 
	LONG    TriggerDelay;    //������ʱ
	LONG    TriggerLength;   //��������
	LONG    TriggerLevel;    //ģ�ⴥ����ƽ
	LONG    lADGain;         //AD����  
	LONG    bEnADD;          //�ۼӹ���ѡ��
    LONG    m_bEn2G;         //2G����
	LONG    lADDthd;         //�ۼ����ޣ���bEnAddʹ��ʱ��Ч����Χ0x00~0xff
	LONG    lADDcnt;         //�ۼӴ�������bEnAddʹ��ʱ��Ч����Χ1~65536
    LONG    m_bSelClk;       //ʱ��
    LONG    m_nClkdeci;      //��Ƶ����
} USB9982_PARA_INIT,*PUSB9982_PARA_INIT;
#endif

//ʱ��ѡ��
typedef enum EmADClkSel
{
	ADCLK_INT        = 0, //����ʱ��
	ADCLK_EXT        = 1  //����ʱ��
} ADCLK_SEL;

//����ģʽ
typedef enum EmTriggerMode
{
	TRIG_MODE_CONTINUE        = 0, //�����ɼ�
	TRIG_MODE_POST            = 1, //�󴥷�		
	TRIG_MODE_DELAY           = 2, //��ʱ����
	TRIG_MODE_PRE			  = 3, //ǰ������USB9982��֧��		
	TRIG_MODE_MIDDLE          = 4  //�д�����USB9982��֧��		
} TRIGGER_MODE;

//����Դ
typedef enum EmTriggerSource
{
	TRIG_SRC_EXT_RISING      = 0,  //�����ش���
	TRIG_SRC_EXT_FALLING     = 1,  //�⸺�ش���	
	TRIG_SRC_SOFT            = 2,  //�������
    TRIG_SRC_CH1_RISING      = 3,  //ADͨ��1���ش���
    TRIG_SRC_CH1_FALLING     = 4,  //ADͨ��1���ش���
    TRIG_SRC_CH2_RISING      = 5,  //ADͨ��2���ش���
    TRIG_SRC_CH2_FALLING     = 6,  //ADͨ��2���ش���
    TRIG_SRC_INT_RISING      = 7,  //�����ش��� ��USB9982C֧��
    TRIG_SRC_INT_FALLING     = 8   //�ڸ��ش��� ��USB9982C֧��
} TRIGGER_SOURCE;

//���η�������׼ʱ��
#define PULSE_BASE_FREQ 125000000L

//��/д��ƫ
#define WRITEOFFSET 0 //д��ƫ
#define READOFFSET  1 //����ƫ

//�������ȵ�λ
#define   TRIG_UNIT   128

//����ȡ���� 3M
#define READ_MAX_LEN   0x300000

//2Gʹ�ܱ�־
#define EN_AD2G 0x10

/***********************************************************/
#ifndef DEFINING
#define DEVAPI __declspec(dllimport)
#else
#define DEVAPI __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

  
	//�ж��Ƿ�Ϊ����USB�豸
	DEVAPI BOOL FAR PASCAL USB9982_IsHighDevice(HANDLE hDevice,PUCHAR pDat);
	//�����豸�Ŵ��豸
	DEVAPI HANDLE FAR PASCAL USB9982_Link(UCHAR DeviceNO);
	//�Ͽ��豸
	DEVAPI BOOL FAR PASCAL USB9982_UnLink(HANDLE hDevice);
	//��ʼ��AD����ʱ��
	DEVAPI BOOL FAR PASCAL USB9982_initADCLK(HANDLE hDevice, LONG lSelADClk, LONG ClkDeci);
	//��ʼ����������ʼ�ɼ�
	DEVAPI BOOL FAR PASCAL USB9982_InitAD(HANDLE hDevice, PUSB9982_PARA_INIT para_init);
	//��ȡAD����
	DEVAPI BOOL FAR PASCAL USB9982_ReadAD(HANDLE hDevice,PUCHAR pBuf, ULONG nCount);
	//�����ɼ�
	DEVAPI BOOL FAR PASCAL USB9982_StopAD(HANDLE hDevice, UCHAR devNum);
	//����DO
	DEVAPI BOOL FAR PASCAL USB9982_SetDO(HANDLE hDevice, LONG byDO);
	//��ȡDI
	DEVAPI BOOL FAR PASCAL USB9982_GetDI(HANDLE hDevice, PLONG pDI);
	//��ȡӲ��FIFO���λ
	DEVAPI BOOL FAR PASCAL USB9982_GetBufOver(HANDLE hDevice, PLONG pBufOver);
	//�������
	DEVAPI BOOL FAR PASCAL USB9982_ExeSoftTrig(HANDLE hDevice);
	//��ȡ�豸��Ϣ
	DEVAPI BOOL FAR PASCAL USB9982_GetDevInfo(HANDLE hdl,LONG* devFifoSize,LONG* devADbit,LONG* devVer);
	//���η�����
	DEVAPI BOOL FAR PASCAL USB9982_SetPulGen(HANDLE hDevice, ULONG lAllcnt, ULONG lHighCnt, BOOL bEnable);
	//����AD��ƫ 
	DEVAPI BOOL FAR PASCAL USB9982_ADoffset(HANDLE hDevice, LONG lselAD,BOOL bWtRd,PLONG plADoffset);

#ifdef __cplusplus
}
#endif


