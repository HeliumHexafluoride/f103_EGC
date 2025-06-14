#include "EGC_dataprocess.h"
#include "ADS1292R.h"

const int fir_num = 156;
const float32_t fir_index[156] = {
    1.619828981e-05, -1.039873223e-05, -0.000210547063, -0.000817734166, -0.002075471682,
    -0.004063331056, -0.006509099621, -0.008731112815, -0.009831191972, -0.009125375189,
    -0.006614431739, -0.003186754184, -0.0003287061118, 0.0006053747493, -0.0008108157199,
    -0.003724452807, -0.006409872789, -0.007279003039, -0.005897671916, -0.00331709953,
    -0.001421164023, -0.001654193737, -0.003978955559, -0.006834441796, -0.008203573525,
    -0.007088393439, -0.004321446177, -0.002048129449, -0.002168453764, -0.004850672558,
    -0.008272318169, -0.009893217124, -0.008395514451, -0.004824161995, -0.001957619563,
    -0.002248840407, -0.00584224984, -0.01025249809, -0.01211506501, -0.00980245322,
    -0.004891912453, -0.001198086771, -0.001914391294, -0.00697969133, -0.01281237882,
    -0.01485840417, -0.01117634587, -0.004269252997, 0.0004926596885, -0.001069789287,
    -0.008432084695, -0.01629677415, -0.01838555746, -0.01243472286, -0.002479870338,
    0.003717676038, 0.0005617462448, -0.01056268718, -0.02157283574, -0.02344852872,
    -0.01347735245, 0.001688381191, 0.01017459854, 0.003846205538, -0.01446899492,
    -0.03155166656, -0.03286547959, -0.01421622373, 0.01283952408, 0.02716168761,
    0.01302709803, -0.02589009888, -0.06415703148, -0.06754042208, -0.0146015184,
    0.0865836218, 0.1982622147, 0.2711901665, 0.2711901665, 0.1982622147,
    0.0865836218, -0.0146015184, -0.06754042208, -0.06415703148, -0.02589009888,
    0.01302709803, 0.02716168761, 0.01283952408, -0.01421622373, -0.03286547959,
    -0.03155166656, -0.01446899492, 0.003846205538, 0.01017459854, 0.001688381191,
    -0.01347735245, -0.02344852872, -0.02157283574, -0.01056268718, 0.0005617462448,
    0.003717676038, -0.002479870338, -0.01243472286, -0.01838555746, -0.01629677415,
    -0.008432084695, -0.001069789287, 0.0004926596885, -0.004269252997, -0.01117634587,
    -0.01485840417, -0.01281237882, -0.00697969133, -0.001914391294, -0.001198086771,
    -0.004891912453, -0.00980245322, -0.01211506501, -0.01025249809, -0.00584224984,
    -0.002248840407, -0.001957619563, -0.004824161995, -0.008395514451, -0.009893217124,
    -0.008272318169, -0.004850672558, -0.002168453764, -0.002048129449, -0.004321446177,
    -0.007088393439, -0.008203573525, -0.006834441796, -0.003978955559, -0.001654193737,
    -0.001421164023, -0.00331709953, -0.005897671916, -0.007279003039, -0.006409872789,
    -0.003724452807, -0.0008108157199, 0.0006053747493, -0.0003287061118, -0.003186754184,
    -0.006614431739, -0.009125375189, -0.009831191972, -0.008731112815, -0.006509099621,
    -0.004063331056, -0.002075471682, -0.000817734166, -0.000210547063, -1.039873223e-05,
    1.619828981e-05};

// 分配存储队列数据的缓冲区
// ECG_TYPE ecg_data_save_buffer[ECG_QUEUE_CAPACITY_SAVE];
// 定义队列管理结构体
// CircularQueue ecg_save_queue;

// 分配存储队列数据的缓冲区
ECG_TYPE ecg_data_fir_buffer[FIR_BLOCKSIZE];
// 定义队列管理结构体
CircularQueue ecg_fir_queue;

ECG_TYPE ecg_info;

ECG_Graph_Type ecg_graph = {0, 0, 200, GRAPH};

uint8_t SampleStartFlag = 0;

uint32_t blockSize = FIR_BLOCKSIZE;
arm_fir_instance_f32 S;
float32_t pState[512] = {0};

// uint8_t ads1292_Cache[9];    //数据缓冲区
// 读取72位的数据1100+LOFF_STAT[4:0]+GPIO[1:0]+13个0+2CHx24位，共9字节
//  1100    LOFF_STAT[4         3           2           1           0   ]   //导联脱落相关的信息在LOFF_STAT寄存器里
//                    RLD     1N2N         1N2P         1N1N    1N1P
//  例  C0 00 00 FF E1 1A FF E1 52

int32_t AdsInBuffer[FIFO_SIZE];
int32_t EcgOutBuffer[FIFO_SIZE];

FIFO_TypeDef InFifoDev;
FIFO_TypeDef OutFifoDev;

bool fir_cq_is_full = 0;

void EGC_dataprocess_init(void)
{
    // cq_init(&ecg_save_queue, ecg_data_save_buffer, ECG_QUEUE_CAPACITY_SAVE);
    cq_init(&ecg_fir_queue, ecg_data_fir_buffer, FIR_BLOCKSIZE);
    arm_fir_init_f32(&S, fir_num, (float32_t *)fir_index, pState, blockSize);
}

void ECGDataFIFOInit(void)
{
    u8 i;
    InFifoDev.read_front = 0;
    InFifoDev.writer_rear = 0;
    InFifoDev.rp = &AdsInBuffer[0];
    InFifoDev.wp = &AdsInBuffer[0];

    OutFifoDev.read_front = 0;
    OutFifoDev.writer_rear = 0;
    OutFifoDev.rp = &EcgOutBuffer[0];
    OutFifoDev.wp = &EcgOutBuffer[0];

    for (i = 0; i < PACK_NUM; i++)
    {
        InFifoDev.state[i] = Empty;
        OutFifoDev.state[i] = Empty;
    }
}

/**
 * @Brief 检测导联线是否脱落
 * @Call   Cheack_lead_stata
 * @Param
 * @Retval None
 */
void Cheack_lead_stata(u8 *p)
{
    u8 data = p[0];
    u8 i = 0;
    if (data & 0x04)
    {
        ecg_info.left_lead_wire_state = LEAD_WIRE_OFF;
    }
    else
    {
        ecg_info.left_lead_wire_state = LEAD_WIRE_ON;
        i++;
    }

    if (data & 0x02)
    {
        ecg_info.right_lead_wrie_state = LEAD_WIRE_OFF;
    }
    else
    {
        ecg_info.right_lead_wrie_state = LEAD_WIRE_ON;
        i++;
    }
    if (i == 2)
        ecg_info.ecg_state = ECG_ON;
    else
        ecg_info.ecg_state = ECG_OFF;
}

/**
 * @Brief 将有符号的24位数转换成32位有符号数据，并更新ECG数据
 * @Call   Update_ECG_Data
 * @Param
 * @Retval None
 */
void Update_ECG_Data(u8 *pdata)
{
    uint16_t respirat = 0;
    uint16_t ecgdata = 0;
    respirat = pdata[3] << 16 | pdata[4] << 8 | pdata[5];
    ecgdata = pdata[6] << 16 | pdata[7] << 8 | pdata[8];
    ecg_info.respirat_impedance = S24toS32(respirat);
    ecg_info.ecg_data = S24toS32(ecgdata);
}

/**
 * @Brief 24位数转32位
 * @Call   Update_ECG_Data
 * @Param
 * @Retval None
 */
int S24toS32(int input)
{
    if ((input & 0x800000) == 0x800000) // 如果最高位为1，则是负数
    {
        input |= 0xff000000; // 高位补1
    }
    return input;
}

u8 data_to_send[13]; // 串口发送缓存
u8 past_value = 0;
// void EcgUsartSendInit(void)
// {
//     MYDMA_Config(DMA2_Stream7,DMA_Channel_4,(u32)&USART1->DR,(u32)data_to_send,13);//串口1DMA设置
// }

void EcgSendByUart(void)
{
    u8 i = 0, sum = 0;
    data_to_send[0] = 0xAA; // 初始化串口初值
    data_to_send[1] = 0xAA;
    data_to_send[2] = 0xF1;
    data_to_send[3] = 8;
    data_to_send[4] = ecg_info.respirat_impedance >> 24; // 25-32位
    data_to_send[5] = ecg_info.respirat_impedance >> 16; // 17-24
    data_to_send[6] = ecg_info.respirat_impedance >> 8;  // 9-16
    data_to_send[7] = ecg_info.respirat_impedance;       // 1-8
    data_to_send[8] = ecg_info.ecg_data >> 24;           // 25-32位
    data_to_send[9] = ecg_info.ecg_data >> 16;           // 17-24
    data_to_send[10] = ecg_info.ecg_data >> 8;           // 9-16
    data_to_send[11] = ecg_info.ecg_data;                // 1-8

    for (i = 0; i < 12; i++)
        sum += data_to_send[i];
    data_to_send[12] = sum; // 校验和
    HAL_UART_Transmit(&huart1, data_to_send, sizeof(data_to_send), 100);
    // if(data_to_send[12]!=past_value){

    //     past_value=data_to_send[12];
    // }

    // USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口 1 的 DMA 发送
    // MYDMA_Enable(DMA2_Stream7,13); //开始一次 DMA 传输！
}

void EcgSendByUart_SET_DATA(ECG_TYPE ecg_info_out)
{
    u8 i = 0, sum = 0;
    data_to_send[0] = 0xAA; // 初始化串口初值
    data_to_send[1] = 0xAA;
    data_to_send[2] = 0xF1;
    data_to_send[3] = 8;
    data_to_send[4] = ecg_info_out.respirat_impedance >> 24; // 25-32位
    data_to_send[5] = ecg_info_out.respirat_impedance >> 16; // 17-24
    data_to_send[6] = ecg_info_out.respirat_impedance >> 8;  // 9-16
    data_to_send[7] = ecg_info_out.respirat_impedance;       // 1-8
    data_to_send[8] = ecg_info_out.ecg_data >> 24;           // 25-32位
    data_to_send[9] = ecg_info_out.ecg_data >> 16;           // 17-24
    data_to_send[10] = ecg_info_out.ecg_data >> 8;           // 9-16
    data_to_send[11] = ecg_info_out.ecg_data;                // 1-8

    for (i = 0; i < 12; i++)
        sum += data_to_send[i];
    data_to_send[12] = sum; // 校验和
    HAL_UART_Transmit(&huart1, data_to_send, sizeof(data_to_send), 100);
    // if(data_to_send[12]!=past_value){

    //     past_value=data_to_send[12];
    // }

    // USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口 1 的 DMA 发送
    // MYDMA_Enable(DMA2_Stream7,13); //开始一次 DMA 传输！
}

void EcgSendByUart_SET_Heart_Rate(uint16_t hart_rate)
{
    u8 i = 0, sum = 0;
    data_to_send[0] = 0xAA; // 初始化串口初值
    data_to_send[1] = 0xFF;
    data_to_send[2] = 0xF1;
    data_to_send[3] = 8;
    data_to_send[4] = hart_rate >> 24; // 25-32位
    data_to_send[5] = hart_rate >> 16; // 17-24
    data_to_send[6] = hart_rate >> 8;  // 9-16
    data_to_send[7] = hart_rate;       // 1-8

    for (i = 0; i < 8; i++)
        sum += data_to_send[i];
    data_to_send[8] = sum; // 校验和
    HAL_UART_Transmit(&huart1, data_to_send, sizeof(data_to_send), 100);
    // if(data_to_send[12]!=past_value){

    //     past_value=data_to_send[12];
    // }

    // USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口 1 的 DMA 发送
    // MYDMA_Enable(DMA2_Stream7,13); //开始一次 DMA 传输！
}


static void WriteAdsInBuffer(int date)
{
    static u8 cnt = 0;
    if (InFifoDev.state[InFifoDev.writer_rear] == Empty)
    {                                                                      // 缓存块可写
        InFifoDev.wp = &AdsInBuffer[InFifoDev.writer_rear * (BLOCK_SIZE)]; // 将写指针定位写缓存块
        InFifoDev.wp[cnt++] = date;
        if (cnt == BLOCK_SIZE)
        {
            cnt = 0;
            InFifoDev.state[InFifoDev.writer_rear] = Full;
            InFifoDev.writer_rear = (InFifoDev.writer_rear + 1) % PACK_NUM; // 切换写缓存块
        }
    }
}

// 定位读指针
// 成功则返回1，不成功则返回0
u8 ReadAdsInBuffer(void)
{
    if (InFifoDev.state[InFifoDev.read_front] == Full)
    {                                                                     // 缓存块可读
        InFifoDev.rp = &AdsInBuffer[InFifoDev.read_front * (BLOCK_SIZE)]; // 将读指针定位读缓存块
        return 1;
    }
    return 0;
}

// 定位读指针
u8 WriterEcgOutBuffer(void)
{
    if (OutFifoDev.state[OutFifoDev.writer_rear] == Empty)
    {                                                                         // 缓存块可写
        OutFifoDev.wp = &EcgOutBuffer[OutFifoDev.writer_rear * (BLOCK_SIZE)]; // 将读指针定位读缓存块
        return 1;
    }
    return 0;
}

// 成功则返回1，不成功则返回0
u8 ReadEcgOutBuffer(int32_t *p)
{
    static u8 cnt = 0;
    if (OutFifoDev.state[OutFifoDev.read_front] == Full)
    {                                                                        // 缓存块可读
        OutFifoDev.rp = &EcgOutBuffer[OutFifoDev.read_front * (BLOCK_SIZE)]; // 将写指针定位读缓存块
        *p = OutFifoDev.rp[cnt++];
        if (cnt == BLOCK_SIZE)
        {
            cnt = 0;
            OutFifoDev.state[OutFifoDev.read_front] = Empty;
            OutFifoDev.read_front = (OutFifoDev.read_front + 1) % PACK_NUM; // 切换写读缓存块
        }
        return 1;
    }
    return 0;
}

void ADS1292_DRDY_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(ADS1292_DRDY_Pin) != RESET && ADS_DRDY_STATE == 0) // 数据接收中断
    {
        __HAL_GPIO_EXTI_CLEAR_IT(ADS1292_DRDY_Pin);
        ADS1292_Read_Data((uint8_t *)ads1292_Cache); // 数据存到9字节缓冲区
        ads1292_recive_flag = 1;
        Update_ECG_Data(ads1292_Cache);
        Cheack_lead_stata(ads1292_Cache);
        // if(SampleStartFlag)
        // WriteAdsInBuffer(ecg_info.ecg_data);//数据写入缓存区
        cq_enqueue(&ecg_fir_queue, &ecg_info);
        if (cq_is_full(&ecg_fir_queue))
            fir_cq_is_full = 1;

        // EcgSendByUart();
    }
}

// 初始化队列
void cq_init(CircularQueue *q, ECG_TYPE *buffer, size_t size)
{
    q->buffer = buffer;
    q->capacity = size;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

// 判断队列是否已满
bool cq_is_full(const CircularQueue *q)
{
    return q->count == q->capacity;
}

// 判断队列是否为空
bool cq_is_empty(const CircularQueue *q)
{
    return q->count == 0;
}

// 获取当前元素数量
size_t cq_get_count(const CircularQueue *q)
{
    return q->count;
}

// 入队操作（添加单元）
bool cq_enqueue(CircularQueue *q, const ECG_TYPE *item)
{
    // 如果队列已满，则无法添加
    if (cq_is_full(q))
    {
        return false;
    }

    // 将数据复制到队尾
    q->buffer[q->tail] = *item;

    // 移动队尾指针，如果到达末尾则回到开头
    q->tail = (q->tail + 1) % q->capacity;

    // 元素数量加一
    q->count++;

    return true;
}

// 出队操作（读取并移除最老的单元）
bool cq_dequeue(CircularQueue *q, ECG_TYPE *item)
{
    // 如果队列为空，则无法读取
    if (cq_is_empty(q))
    {
        return false;
    }

    // 从队头复制数据
    *item = q->buffer[q->head];

    // 移动队头指针，如果到达末尾则回到开头
    q->head = (q->head + 1) % q->capacity;

    // 元素数量减一
    q->count--;

    return true;
}

// 读取整个队列（不修改原始队列）
size_t cq_read_all(const CircularQueue *q, ECG_TYPE *output_buffer, size_t output_buffer_size)
{
    // 如果输出缓冲区不够大，无法完成操作
    if (output_buffer_size < q->count)
    {
        return 0; // 返回0表示没有复制任何元素
    }

    if (cq_is_empty(q))
    {
        return 0; // 队列为空，直接返回
    }

    size_t current_head = q->head;
    for (size_t i = 0; i < q->count; i++)
    {
        // 从队头开始，依次复制所有元素到输出缓冲区
        output_buffer[i] = q->buffer[current_head];
        current_head = (current_head + 1) % q->capacity;
    }

    // 返回成功复制的元素数量
    return q->count;
}

void compute_integrated_signal(const float32_t *ecg_data, float32_t *integrated_out)
{
    // 微分滤波器参数
    const float32_t der_coeffs[5] = {-1.0f, -2.0f, 0.0f, 2.0f, 1.0f};
    float32_t der_buf[5] = {0};
    int der_idx = 0;

    // 移动平均参数
    const int window_size = 38; // 150ms窗口@250Hz
    float32_t ma_buf[38] = {0};
    int ma_idx = 0;
    float32_t ma_sum = 0;

    // 临时工作数组
    float32_t derivative[BLOCK_SIZE];
    float32_t squared[BLOCK_SIZE];

    // 1. 微分处理
    for (int i = 2; i < BLOCK_SIZE - 2; i++)
    {
        der_buf[der_idx] = ecg_data[i];
        der_idx = (der_idx + 1) % 5;
        derivative[i] = 0;
        for (int k = 0; k < 5; k++)
        {
            derivative[i] += der_coeffs[k] * der_buf[(der_idx + k) % 5];
        }
    }

    // 2. 平方运算
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        squared[i] = derivative[i] * derivative[i];
    }

    // 3. 移动平均积分
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        ma_sum -= ma_buf[ma_idx];
        ma_sum += squared[i];
        ma_buf[ma_idx] = squared[i];
        ma_idx = (ma_idx + 1) % window_size;

        if (i >= window_size / 2)
        {
            integrated_out[i - window_size / 2] = ma_sum / window_size;
        }
    }
}

uint16_t thresh = 3;
uint16_t thresh_mid=75;  // bpm=200

uint16_t hr_count(const int *h)
{
    bool is_up = true;
    bool first = true;    
    int id_up = 0, id_down = 0;
    int count = 0;
    // float32_t avg = 0;

    uint16_t mid_avg = 0;
    uint16_t mid1 = 0;
    uint16_t mid2 = 0;
    for (int i ; i < BLOCK_SIZE * PACK_NUM_HR - 2; i++)
    {
        if ( (h[i] < h[i + 1]) && (!is_up) && (h[i+1] > thresh))
        {
            id_up = i;
            is_up = true;
        }
        if ( (h[i] > h[i + 1]) && (is_up) && (h[i+1] < thresh))
        {
            id_down = i;
            is_up = false;
            if (first)
            {
                mid1 = id_up + (id_down - id_up) / 2;
                first = false;
            }
            else if((id_up + (id_down - id_up) / 2-mid1)>thresh_mid){
                mid2 = id_up + (id_down - id_up) / 2;
                mid_avg +=mid2-mid1;
                mid1 = mid2;
                count++;                
            }

        }
    }
    if(count==0){
    return 0;
    }
    else
    mid_avg = mid_avg*60/(count * 250);
    mid_avg*=1.6;
    return mid_avg;
}

