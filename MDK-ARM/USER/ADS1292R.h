#ifndef __ADS1292R_H
#define __ADS1292R_H	 
#include "main.h" 
#include "spi.h"
#include "usart.h"
#include "key.h"

////////////////////////////////////////////////////////////////////////////////
//引脚宏定义	
#define ADS_CS_HIGH   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET)
#define ADS_CS_LOW   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET)

// #define ADS_DRDY_HIGH   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET)
// #define ADS_DRDY_LOW   HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET)
		
//	#define ADS_RESET 	PAout(0)
//	#define ADS_START		PAout(2)	
//	#define ADS_CLKSEL	PAout(3)	

/////////////////////////////////////////////////////////////////////////////////////////////
//声明变量
extern	volatile uint8_t ads1292_Cache[9];		//ads1292数据缓存
extern  volatile uint8_t ads1292_recive_flag;	//数据读取完成标志



/////////////////////////////////////////////////////////////////////////////////////////////
		//ADS1292R 具有呼吸和导联脱落检测的功能
		//ID
		#define	ADS1292_DEVICE	DEVICE_ID_ADS1292R		//注意1292芯片不能使用呼吸相关的功能，会导致数据不正常
		//1		CONFIG1	
		#define DATA_RATE  				DATA_RATE_500SPS		//由于时钟原因，采样率会有误差
		//2		CONFIG2		
		#define	PDB_LOFF_COMP 		PDB_LOFF_COMP_ON		//导联脱落比较器
		#define	PDB_REFBUF				PDB_REFBUF_ON				//基准电压缓冲器
		#define	VREF							VREF_242V						//设置内部参考电压
		#define	CLK_EN						CLK_EN_OFF					//使用内部时钟时，CLK引脚是否输出时钟信号
		#define	INT_TEST					INT_TEST_OFF				//是否打开内部测试信号
		//4	5		CHSET	
		#define	CNNNLE1_POWER			PD_ON								//通道电源
		#define	CNNNLE1_GAIN			GAIN_2							//增益
		#define	CNNNLE1_MUX				MUX_Normal_input		//输入方式
		#define	CNNNLE2_POWER			PD_ON								
		#define	CNNNLE2_GAIN			GAIN_6							
		#define	CNNNLE2_MUX				MUX_Normal_input		
		//6		RLD_SENS	
		#define	PDB_RLD						PDB_RLD_ON					//RLD缓冲电源		
		#define	RLD_LOFF_SENSE		RLD_LOFF_SENSE_OFF	//RLD导联脱落功能（测试的时候发现右腿导联脱落检测和右腿驱动输出好像不能同时工作）
		#define	RLD2N							RLD_CANNLE_ON				//通道的右腿驱动输出
		#define	RLD2P							RLD_CANNLE_ON
		#define	RLD1N							RLD_CANNLE_OFF			//呼吸通道不需要右腿驱动
		#define	RLD1P							RLD_CANNLE_OFF
		//7		LOFF_SENS	
		#define	FLIP2							FLIP2_OFF						//这个位用于控制导联脱落检测通道1的电流的方向
		#define	FLIP1							FLIP1_OFF						//这个位用于控制导联脱落检测通道2的电流的方向
		#define	LOFF2N						RLD_CANNLE_ON			//通道导联脱落检测功能
		#define	LOFF2P						RLD_CANNLE_ON
		#define	LOFF1N						RLD_CANNLE_OFF			//呼吸通道不需要导联脱落检测
		#define	LOFF1P						RLD_CANNLE_OFF
		//9		RSP1	
		#define	RESP_DEMOD_EN1		RESP_DEMOD_ON		//启用通道1解调电路
		#define	RESP_MOD_EN				RESP_MOD_ON			//启用通道1调制电路
		#define	RESP_PH						0X0D					//控制解调信号的相位	135																					
		#define	RESP_CTRL					RESP_CTRL_CLOCK_INTERNAL//内部呼吸内部时钟
		//10		RSP2	
		#define	CALIB							CALIB_OFF								//通道偏移校正 //运行过程中如果改变增益需要打开通道偏移校正，并执行 OFFSETCAL 指令
		#define	FREQ							FREQ_32K								//呼吸调制频率	64K/32K
		#define	RLDREF_INT				RLDREF_INT_INTERNALLY		//RLD参考 内部馈电  右腿驱动的参考电压可以选择内部产生(AVDD + AVSS) / 2，也可以选择外部提供




//ADS1292R命令定义
//系统命令
	#define WAKEUP	0X02	//从待机模式唤醒
	#define STANDBY	0X04	//进入待机模式
	#define RESET	0X06	//复位ADS1292R
	#define START	0X08	//启动或转换
	#define STOP	0X0A	//停止转换
	#define OFFSETCAL	0X1A	//通道偏移校准

//数据读取命令
	#define RDATAC	0X10	//启用连续的数据读取模式,默认使用此模式
	#define SDATAC	0X11	//停止连续的数据读取模式
	#define RDATA		0X12	//通过命令读取数据;支持多种读回。
//寄存器读取命令
	//r rrrr=要读、写的寄存器首地址	 //	n nnnn=要读写的寄存器数量
	#define RREG	0X20	//读取  001r rrrr(首字节) 000n nnnn(2字节)
	#define WREG	0X40	//写入  010r rrrr(首字节) 000n nnnn(2字节)
	
//ADS1292R内部寄存器地址定义
	#define ID					0	//ID控制寄存器
	#define CONFIG1			1	//配置寄存器1
	#define CONFIG2			2	//配置寄存器2
	#define LOFF				3	//导联脱落控制寄存器
	#define CH1SET			4	//通道1设置寄存器
	#define CH2SET			5	//通道2设置寄存器
	#define RLD_SENS		6	//右腿驱动选择寄存器
	#define LOFF_SENS		7	//导联脱落检测选择寄存器
	#define LOFF_STAT		8	//导联脱落检测状态寄存器
	#define	RESP1				9	//呼吸检测控制寄存器1
	#define	RESP2				10//呼吸检测控制寄存器2
	#define	GPIO				11//GPIO控制寄存器


/////////////////////////////////////////////////////////////////////////////////////////////
//寄存器设置
//ID
#define	DEVICE_ID_ADS1292		0X53
#define	DEVICE_ID_ADS1292R	0X73
//CONFIG1
#define	DATA_RATE_125SPS	0X00	//采样率
#define	DATA_RATE_250SPS	0X01
#define	DATA_RATE_500SPS	0X02
#define	DATA_RATE_1kSPS		0X03
#define	DATA_RATE_2kSPS		0X04
#define	DATA_RATE_4kSPS		0X05
#define	DATA_RATE_8kSPS		0X06
//CONFIG2
#define	PDB_LOFF_COMP_OFF	0	//导联脱落比较器掉电（默认）
#define	PDB_LOFF_COMP_ON	1
#define	PDB_REFBUF_OFF	0		//基准电压缓冲器掉电（默认）
#define	PDB_REFBUF_ON		1
#define	VREF_242V		0	//内部参考电压2.42V（默认）
#define	VREF_4V			1	//内部参考电压4.033V
#define	CLK_EN_OFF	0	//振荡器时钟输出禁用（默认）
#define	CLK_EN_ON		1
#define	INT_TEST_OFF	0	//关闭内部测试信号（默认）
#define	INT_TEST_ON		1
//CHSET
#define	PD_ON			0	//通道正常运行（默认）	
#define	PD_OFF		1
#define	GAIN_6		0//增益6（默认）
#define	GAIN_1		1
#define	GAIN_2		2
#define	GAIN_3		3
#define	GAIN_4		4
#define	GAIN_8		5
#define	GAIN_12		6
#define	MUX_Normal_input 	0		//普通电极输入（默认）
#define	MUX_input_shorted 1		//输入短路
#define	MUX_Test_signal 	5		//测试信号输入
#define	MUX_RLD_DRP				6	
#define	MUX_RLD_DRM				7		
#define	MUX_RLD_DRPM			8	
#define	MUX_RSP_IN3P			9		//呼吸道通道1
//RLD_SENS
#define	PDB_RLD_OFF					0//（默认）
#define	PDB_RLD_ON					1
#define	RLD_LOFF_SENSE_OFF	0//（默认）
#define	RLD_LOFF_SENSE_ON		1
#define	RLD_CANNLE_OFF			0//（默认）
#define	RLD_CANNLE_ON				1
//LOFF_SENS
#define	FLIP2_OFF			0//（默认）
#define	FLIP2_ON			1							
#define	FLIP1_OFF			0//（默认）
#define	FLIP1_ON			1			
#define	LOFF_CANNLE_OFF		0//（默认）
#define	LOFF_CANNLE_ON		1	
//RSP1
#define	RESP_DEMOD_OFF	0	//（默认）
#define	RESP_DEMOD_ON		1	
#define	RESP_MOD_OFF 		0	//（默认）
#define	RESP_MOD_ON			1
#define	RESP_CTRL_CLOCK_INTERNAL	0
#define	RESP_CTRL_CLOCK_EXTERNAL	1
//RSP2
#define	CALIB_OFF		0//（默认）
#define	CALIB_ON		1		
#define	FREQ_32K		0	//（默认）
#define	FREQ_64K		1
#define	RLDREF_INT_EXTERN				0//外部馈电RLDREF
#define	RLDREF_INT_INTERNALLY		1//内部




typedef struct
{
		uint8_t Data_Rate; //ADC通道采样率  
}ADS1292_CONFIG1;
typedef struct
{
		uint8_t	Pdb_Loff_Comp; 	//导联脱落比较器是否掉电
		uint8_t	Pdb_Refbuf;			//内部参考缓冲器是否掉电
		uint8_t	Vref;				//内部参考电压设置
		uint8_t	Clk_EN;					//振荡器时钟输出设置
		uint8_t	Int_Test;				//内部测试信号使能位	
}ADS1292_CONFIG2;
typedef struct
{
		uint8_t  PD;			//通道断电？
		uint8_t	GAIN;		//设置PGA增益
		uint8_t	MUX;		//设置通道输入方式
}ADS1292_CHSET;
typedef struct
{
		uint8_t	Pdb_Rld;				//该位决定RLD缓冲电源状态
		uint8_t	Rld_Loff_Sense;	//该位使能RLD导联脱落检测功能
		uint8_t	Rld2N;					//这个位控制通道2负输入 用于右腿驱动的输出
		uint8_t	Rld2P;					//该位控制通道2正输入 用于右腿驱动的输出
		uint8_t	Rld1N;					//这个位控制通道1负输入 用于右腿驱动的输出
		uint8_t	Rld1P;					//该位控制通道1正输入 用于右腿驱动的输出
}ADS1292_RLD_SENS;
typedef struct
{
		uint8_t	Flip2;//这个位用于控制导联脱落检测通道2的电流的方向
		uint8_t	Flip1;//这个位控制用于导联脱落检测通道1的电流的方向
		uint8_t	Loff2N;//该位控制通道2负输入端的导联脱落检测
		uint8_t	Loff2P;//该位控制通道2正输入端的导联脱落检测
		uint8_t	Loff1N;//该位控制通道1负输入端的导联脱落检测
		uint8_t	Loff1P;//该位控制通道1正输入端的导联脱落检测
}ADS1292_LOFF_SENS;
typedef struct
{	
		uint8_t	RESP_DemodEN;		//通道偏移校准
		uint8_t	RESP_modEN;	//RLDREF信号源
		uint8_t	RESP_ph;	
		uint8_t	RESP_Ctrl;	
}ADS1292_RESP1;
typedef struct
{	
		uint8_t	Calib;		//通道偏移校准
		uint8_t	freq;			//频率 32k 64k
		uint8_t	Rldref_Int;	//RLDREF信号源
}ADS1292_RESP2;





void ADS1292_Init(void); //初始化ADS1292引脚
void ADS1292_PowerOnInit(void);//上电初始化
uint8_t ADS1292_SPI(uint8_t com);

void ADS1292_Send_CMD(uint8_t data);//发送命令
void ADS1292_WR_REGS(uint8_t reg,uint8_t len,uint8_t *data);//读写多个寄存器
uint8_t ADS1292_Read_Data(uint8_t *data);//读9字节数据

void ADS1292_SET_REGBUFF(void);//设置寄存器数组
uint8_t ADS1292_WRITE_REGBUFF(void);//将寄存器数组写入寄存器

uint8_t ADS1292_Noise_Test(void);
uint8_t ADS1292_Single_Test(void);//设置通道1内部1mV测试信号
uint8_t ADS1292_Single_Read(void);//设置正常信号采集模式
uint8_t Set_ADS1292_Collect(uint8_t mode);//设置数据采集方式

#endif


//手册及论坛资料：
//关于CLK您是使用内部时钟还是使用外部时钟？如果使用内部时钟，可以将其接地。如果使用外部时钟，可以接有源晶振或者MCU的时钟输出引脚给ADS1292 提供时钟。
//使用内部晶振的话，寄存器CONFIG2的bit3 如果配置为1的话，那么CLK有输出，频率即为内部时钟产生的频率，如果配置为0的话，那么CLK输出disable
//SCLK 即为SPI的频率，它的大小datasheet已经给出，当2.7 V ≤ DVDD ≤ 3.6 V，周期tSCLK（min）=50ns。 当1.7 V ≤ DVDD ≤ 2 V时，tSCLK（min）=66.6ns

//1. 在连续读模式下，不能读写寄存器。在连续读模式下，首先要发命令停止 SDATAC 。然后才能发送其他命令。
//2. 手册里面这样描述，1292 接收多字节命令时，解析一个字节需要 7.2us。因此你发送多字节命令时，两个字节之间的间隔至少要到8us
//测试流程
//	设置 CLKSEL =1 	使用内部时钟，
//	设置 PWDN/RESET = 1  等待1秒 开机复位和等待起振
//	发送SDATAC命令  设置寄存器
//	发送WREG CONFIG2 A0h	使用内部参考电压，
//	设置START = 1		激活转换
//	发送RDATAC命令	将设备恢复到RDATAC模式
//	捕获数据并检查噪音
//	捕获数据并测试信号

