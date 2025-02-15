//AD初始化参数
#ifndef _USB9982_PARA_INIT// USB9982.h
#include <windows.h>  // 确保包含LONG的定义
typedef struct _USB9982_PARA_INIT    
{
	LONG    lSelADClk;       //预留
	LONG    lChCnt;          //通道选择
	LONG    ClkDeci;         //预留
	LONG	TriggerMode;     //触发模式
	LONG	TriggerSource;	 //触发源 
	LONG    TriggerDelay;    //触发延时
	LONG    TriggerLength;   //触发长度
	LONG    TriggerLevel;    //模拟触发电平
	LONG    lADGain;         //AD增益  
	LONG    bEnADD;          //累加功能选择
    LONG    m_bEn2G;         //2G功能
	LONG    lADDthd;         //累加门限，仅bEnAdd使能时有效，范围0x00~0xff
	LONG    lADDcnt;         //累加次数，仅bEnAdd使能时有效，范围1~65536
    LONG    m_bSelClk;       //时钟
    LONG    m_nClkdeci;      //分频因子
} USB9982_PARA_INIT,*PUSB9982_PARA_INIT;
#endif

//时钟选择
typedef enum EmADClkSel
{
	ADCLK_INT        = 0, //板上时钟
	ADCLK_EXT        = 1  //板外时钟
} ADCLK_SEL;

//触发模式
typedef enum EmTriggerMode
{
	TRIG_MODE_CONTINUE        = 0, //连续采集
	TRIG_MODE_POST            = 1, //后触发		
	TRIG_MODE_DELAY           = 2, //延时触发
	TRIG_MODE_PRE			  = 3, //前触发，USB9982不支持		
	TRIG_MODE_MIDDLE          = 4  //中触发，USB9982不支持		
} TRIGGER_MODE;

//触发源
typedef enum EmTriggerSource
{
	TRIG_SRC_EXT_RISING      = 0,  //外正沿触发
	TRIG_SRC_EXT_FALLING     = 1,  //外负沿触发	
	TRIG_SRC_SOFT            = 2,  //软件触发
    TRIG_SRC_CH1_RISING      = 3,  //AD通道1正沿触发
    TRIG_SRC_CH1_FALLING     = 4,  //AD通道1负沿触发
    TRIG_SRC_CH2_RISING      = 5,  //AD通道2正沿触发
    TRIG_SRC_CH2_FALLING     = 6,  //AD通道2负沿触发
    TRIG_SRC_INT_RISING      = 7,  //内正沿触发 仅USB9982C支持
    TRIG_SRC_INT_FALLING     = 8   //内负沿触发 仅USB9982C支持
} TRIGGER_SOURCE;

//波形发生器基准时钟
#define PULSE_BASE_FREQ 125000000L

//读/写零偏
#define WRITEOFFSET 0 //写零偏
#define READOFFSET  1 //读零偏

//触发长度单位
#define   TRIG_UNIT   128

//最大读取长度 3M
#define READ_MAX_LEN   0x300000

//2G使能标志
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

  
	//判断是否为高速USB设备
	DEVAPI BOOL FAR PASCAL USB9982_IsHighDevice(HANDLE hDevice,PUCHAR pDat);
	//根据设备号打开设备
	DEVAPI HANDLE FAR PASCAL USB9982_Link(UCHAR DeviceNO);
	//断开设备
	DEVAPI BOOL FAR PASCAL USB9982_UnLink(HANDLE hDevice);
	//初始化AD工作时钟
	DEVAPI BOOL FAR PASCAL USB9982_initADCLK(HANDLE hDevice, LONG lSelADClk, LONG ClkDeci);
	//初始化参数并开始采集
	DEVAPI BOOL FAR PASCAL USB9982_InitAD(HANDLE hDevice, PUSB9982_PARA_INIT para_init);
	//读取AD数据
	DEVAPI BOOL FAR PASCAL USB9982_ReadAD(HANDLE hDevice,PUCHAR pBuf, ULONG nCount);
	//结束采集
	DEVAPI BOOL FAR PASCAL USB9982_StopAD(HANDLE hDevice, UCHAR devNum);
	//设置DO
	DEVAPI BOOL FAR PASCAL USB9982_SetDO(HANDLE hDevice, LONG byDO);
	//读取DI
	DEVAPI BOOL FAR PASCAL USB9982_GetDI(HANDLE hDevice, PLONG pDI);
	//读取硬件FIFO溢出位
	DEVAPI BOOL FAR PASCAL USB9982_GetBufOver(HANDLE hDevice, PLONG pBufOver);
	//软件触发
	DEVAPI BOOL FAR PASCAL USB9982_ExeSoftTrig(HANDLE hDevice);
	//读取设备信息
	DEVAPI BOOL FAR PASCAL USB9982_GetDevInfo(HANDLE hdl,LONG* devFifoSize,LONG* devADbit,LONG* devVer);
	//波形发生器
	DEVAPI BOOL FAR PASCAL USB9982_SetPulGen(HANDLE hDevice, ULONG lAllcnt, ULONG lHighCnt, BOOL bEnable);
	//设置AD零偏 
	DEVAPI BOOL FAR PASCAL USB9982_ADoffset(HANDLE hDevice, LONG lselAD,BOOL bWtRd,PLONG plADoffset);

#ifdef __cplusplus
}
#endif


