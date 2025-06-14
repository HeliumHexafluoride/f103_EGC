#ifndef __EGC_DATAPROCESS_H
#define __EGC_DATAPROCESS_H

#include"main.h"
#include "arm_math.h"

#define BLOCK_SIZE           250 /* 调用一次arm_fir_f32处理的采样点个数 */
#define PACK_NUM             2
#define PACK_NUM_HR          12
#define FIFO_SIZE (PACK_NUM * BLOCK_SIZE)//缓存区大小

// 定义环形队列容量
#define ECG_QUEUE_CAPACITY_SAVE 1000

//块长度，原始数据长度为1000
#define FIR_BLOCKSIZE 250

/*****ECG数据信息******/
typedef struct
{
    u8 right_lead_wrie_state;
    u8 left_lead_wire_state;
	u8 ecg_state;
    int respirat_impedance;//呼吸阻抗
    int ecg_data;         //心电数据
} ECG_TYPE;


typedef struct
{
    int y_min;
    int y_pose;
	int sacle;//放缩比例
    u8 send_type;
} ECG_Graph_Type;

/***导联线的状态****/
typedef enum
{
    LEAD_WIRE_OFF = 0x01,
    LEAD_WIRE_ON  = 0X02
} LEAD_STAT;


typedef enum
{
    ECG_ON  = 0x01,
    ECG_OFF = 0X02
} ECG_STAT;

typedef enum
{
    GRAPH = 0x01,
    USART  = 0X02
} SEND_TYPE;

//采用FIFO队列控制图像包的进出
typedef struct FIFO
{
	int32_t* rp; //读指针
	int32_t* wp;//写指针
	u8 state[PACK_NUM];//状态表
	u8 read_front;					 //队列的头
	u8 writer_rear;					//队列的尾
}FIFO_TypeDef;

/***FIFO的状态表的状态****/
typedef enum
{
    Empty = 0x00,
    Full  = 0X01
}FIFO_STAT;

// 2. 定义环形队列的管理结构体
typedef struct
{
    ECG_TYPE *buffer;   // 指向数据存储区的指针
    size_t head;        // 队列头部索引（读取位置）
    size_t tail;        // 队列尾部索引（写入位置）
    size_t count;       // 当前队列中的元素数量
    size_t capacity;    // 队列的总容量
} CircularQueue;


extern ECG_TYPE ecg_info;

extern int32_t AdsInBuffer[FIFO_SIZE];
extern int32_t EcgOutBuffer[FIFO_SIZE];

extern FIFO_TypeDef InFifoDev;
extern FIFO_TypeDef OutFifoDev;

extern ECG_Graph_Type ecg_graph;
void Cheack_lead_stata(u8 *p);
void Update_ECG_Data(u8 *pdata);
int S24toS32(int input);
// void EcgUsartSendInit(void);
// void EcgSendByUart(int value);
void EcgSendByUart(void);
void ECGDataFIFOInit(void);
u8 ReadAdsInBuffer(void);
u8 WriterEcgOutBuffer(void);
u8 ReadEcgOutBuffer(int32_t *p);



extern const int fir_num;
extern const float32_t fir_index[156];

extern arm_fir_instance_f32 S;
extern bool fir_cq_is_full;

// 分配存储队列数据的缓冲区
// extern ECG_TYPE ecg_data_save_buffer[ECG_QUEUE_CAPACITY_SAVE];
// 定义队列管理结构体
// extern CircularQueue ecg_save_queue;

// 分配存储队列数据的缓冲区
extern ECG_TYPE ecg_data_fir_buffer[FIR_BLOCKSIZE];
// 定义队列管理结构体
extern CircularQueue ecg_fir_queue;


/**
 * @brief 初始化环形队列
 * @param q 指向要初始化的队列管理结构体的指针
 * @param buffer 用户提供的用于存储数据的缓冲区
 * @param size 缓冲区的总容量 (可以容纳的元素个数)
 */
void cq_init(CircularQueue *q, ECG_TYPE *buffer, size_t size);

/**
 * @brief 向队列尾部添加一个单元
 * @param q 指向队列的指针
 * @param item 要添加的数据单元
 * @return 成功返回 true, 如果队列已满则返回 false
 */
bool cq_enqueue(CircularQueue *q, const ECG_TYPE *item);

/**
 * @brief 从队列头部读取并移除一个单元
 * @param q 指向队列的指针
 * @param item 指向用于存储被读取数据的变量的指针
 * @return 成功返回 true, 如果队列为空则返回 false
 */
bool cq_dequeue(CircularQueue *q, ECG_TYPE *item);

/**
 * @brief 判断队列是否已满
 * @param q 指向队列的指针
 * @return 如果已满返回 true, 否则返回 false
 */
bool cq_is_full(const CircularQueue *q);

/**
 * @brief 判断队列是否为空
 * @param q 指向队列的指针
 * @return 如果为空返回 true, 否则返回 false
 */
bool cq_is_empty(const CircularQueue *q);

/**
 * @brief 获取队列中当前存储的元素数量
 * @param q 指向队列的指针
 * @return 队列中的元素数量
 */
size_t cq_get_count(const CircularQueue *q);

/**
 * @brief 读取队列中的所有数据到另一个缓冲区（不从队列中移除）
 * @param q 指向队列的指针
 * @param output_buffer 用于存储整个队列数据的输出缓冲区
 * @param output_buffer_size 输出缓冲区的大小
 * @return 成功复制的元素数量。如果输出缓冲区太小，则不会复制任何内容并返回0。
 */
size_t cq_read_all(const CircularQueue *q, ECG_TYPE *output_buffer, size_t output_buffer_size);


// 初始化数据处理
void EGC_dataprocess_init(void);
void EcgSendByUart_SET_DATA(ECG_TYPE ecg_info_out);
void compute_integrated_signal(const float32_t* ecg_data, float32_t* integrated_out);

uint16_t hr_count(const int *h);
void EcgSendByUart_SET_Heart_Rate(uint16_t hart_rate);
#endif
