/* add user code begin Header */
/**
  ******************************************************************************
  * File Name          : freertos_app.c
  * Description        : Code for freertos applications
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "freertos_app.h"
#include <mbedtls/sha256.h>
#include "version.h"
/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <string.h>
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
int SubFrameIndex = 0;
int SubFrameCount = 2;
uint8_t LastCommand=AFE_RESP_OK;
uint8_t LastResponse=AFE_RESP_OK;
static volatile JobState_t g_job_state = JOB_IDLE;
static volatile int32_t    g_job_error = 0;

static QueueHandle_t g_job_queue;

/* ===== 命令队列：spiTask 完成一帧 -> cmdProcessorTask 处理 ===== */
#define CMD_QUEUE_DEPTH  3
static QueueHandle_t g_cmd_queue;
volatile uint8_t ADC_Capture[2032*40];
volatile int ADC_Capture_Index1=0;
volatile int ADC_Capture_Index2=0;
volatile int ADC_Capture_Ready1=0;
volatile int ADC_Capture_Ready2=0;
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */
EventGroupHandle_t CommEvent = NULL;
mbedtls_sha256_context ctx;
volatile uint8_t spi_resp_buffer[2048];
/* add user code end 0 */

/* task handler */
TaskHandle_t defaultTask_handle;
TaskHandle_t cmdProcessor_handle;
TaskHandle_t spi_handle;
TaskHandle_t worker_handle;

/* Idle task control block and stack */
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t idle_task_tcb;
static StaticTask_t timer_task_tcb;

/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );
extern void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize );

/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &idle_task_tcb;
  *ppxIdleTaskStackBuffer = &idle_task_stack[0];
  *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}
/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &timer_task_tcb;
  *ppxTimerTaskStackBuffer = &timer_task_stack[0];
  *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__WEAK void configureTimerForRunTimeStats(void)
{
/* add user code begin configureTimerForRunTimeStats */

/* add user code end configureTimerForRunTimeStats */
}

__WEAK unsigned long getRunTimeCounterValue(void)
{
/* add user code begin configureTimerForRunTimeStats */

/* add user code end configureTimerForRunTimeStats */

  return 0;
}

/* add user code begin 1 */

/* add user code end 1 */

/**
  * @brief  initializes all task.
  * @param  none
  * @retval none
  */
void freertos_task_create(void)
{
	/* create defaultTask task */
	xTaskCreate(StartDefaultTask, "defaultTask", 256,
	NULL, 0, &defaultTask_handle);


}

/**
  * @brief  freertos init and begin run.
  * @param  none
  * @retval none
  */
void wk_freertos_init(void)
{
  /* add user code begin freertos_init 0 */

  /* add user code end freertos_init 0 */

  /* enter critical */
  taskENTER_CRITICAL();

  freertos_task_create();
	
  /* add user code begin freertos_init 1 */

  /* add user code end freertos_init 1 */

  /* exit critical */
  taskEXIT_CRITICAL();

  /* start scheduler */
  vTaskStartScheduler();
}

/**
 * @brief defaultTask function.
 * @param  none
 * @retval none
 */

uint8_t crc8(const uint8_t *data, int len)
{
	uint8_t crc = 0;

	for (int i = 0; i < len; i++)
	{
		crc ^= data[i];

		for (int j = 0; j < 8; j++)
		{
			if (crc & 0x80)
				crc = (crc << 1) ^ 0x07;
			else
				crc <<= 1;
		}
	}

	return crc;
}

uint32_t crc_block_calculate_bytes(uint8_t *pbuffer, uint32_t length)
{
	crc_data_reset();
	for (uint32_t i = 0; i < length; i++)
	{
		// Write each byte to the CRC_DT register (byte access)
		*((volatile uint8_t*) &CRC->dt) = pbuffer[i];
	}
	return (CRC->dt);
}

uint8_t hash96_to_8(const uint8_t data[12])
{
	uint32_t h = 0x9747b28c;

	for (int i = 0; i < 12; i++)
	{
		h ^= data[i];
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}

	// Final avalanche
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return (uint8_t) h;
}

int mbedtls_sha_test(void)
{
    unsigned char output[32];
    const char *input = "Hello, mbedTLS v3.x!";
    size_t ilen = strlen(input);

    // One-shot function: mbedtls_sha256(input, ilen, output, is224)
    int ret = mbedtls_sha256((const unsigned char *)input, ilen, output, 0); // 0 for SHA-256

    if (ret != 0) {
        printf("mbedtls_sha256 failed: -0x%04X\n", -ret);
        return ret;
    }

    printf("SHA-256 hash: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", output[i]);
    }
    printf("\n");
    return 0;
}

int fw_hash_verify(uint8_t buffer[32])
{
    int ret=0;
    unsigned char output[32]; // SHA-256 produces a 32-byte hash
    const char *input =(const char *) APP_BASE;
    AppBinHeaderType* imgheader=(AppBinHeaderType*)(APP_BASE+0x200);

    if((imgheader->magic!=0x57465645)||(imgheader->size==0)||(imgheader->size>446*1024))
	{
    	return -1;
	}
    size_t ilen = 0x210;

    // 1. Declare and initialize the context

    mbedtls_sha256_init(&ctx);

    // 2. Start the SHA-256 calculation (0 for SHA-256, 1 for SHA-224)
    if ((ret = mbedtls_sha256_starts(&ctx, 0)) != 0) {
//        printf("mbedtls_sha256_starts failed: -0x%04X\n", -ret);
        goto exit;
    }

    // 3. Feed data into the hash function (can be called multiple times)
    if ((ret = mbedtls_sha256_update(&ctx, (const unsigned char *)input, ilen)) != 0) {
//        printf("mbedtls_sha256_update failed: -0x%04X\n", -ret);
        goto exit;
    }

    ilen=imgheader->size-0x270;
    if ((ret = mbedtls_sha256_update(&ctx, (const unsigned char *)(input+0x270), ilen)) != 0) {
//            printf("mbedtls_sha256_update failed: -0x%04X\n", -ret);
            goto exit;
        }

    // 4. Finalize the calculation and get the hash
    if ((ret = mbedtls_sha256_finish(&ctx, output)) != 0) {
//        printf("mbedtls_sha256_finish failed: -0x%04X\n", -ret);
        goto exit;
    }

    memcpy(buffer,output,32);
    // 5. Print the resulting hash in hexadecimal
//    printf("SHA-256 hash: ");
    for (int i = 0; i < 32; i++) {
    	if((output[i])!=imgheader->hash[i])
		{
			ret=-6;
		}
//        printf("%02x", output[i]);
    }
//    printf("\r\n");

//    printf(" hash: ");
//	for (int i = 0; i < 32; i++) {
//		printf("%02x", imgheader->hash[i]);
//	}
//	printf("\r\n");

exit:

    // 6. Free the context
    mbedtls_sha256_free(&ctx);
    return ret;
}


void StartDefaultTask(void *pvParameters)
{
  /* add user code begin StartDefaultTask 0 */
	AFEMetaDataDef *meta = (AFEMetaDataDef*) (0x0807F800);
	AFEDataPacket *packet = (AFEDataPacket*) spi_buffer;
	uint8_t buffer[32];
	/* add user code end StartDefaultTask 0 */

  /* add user code begin StartDefaultTask 2 */
	uint32_t UID1 = *(uint32_t*) 0x1FFFF7E8;
	uint32_t UID2 = *(uint32_t*) 0x1FFFF7EC;
	uint32_t UID3 = *(uint32_t*) 0x1FFFF7F0;
	uint8_t UID[12];
	uint32_t *ptr = (uint32_t*) UID;

	ptr[0] = UID1;
	ptr[1] = UID2;
	ptr[2] = UID3;
	int index = 0;
	uint32_t t = meta->id * 5;

	vTaskDelay(t);
	wk_usart1_init();

	printf("%08X %08X %08X:%d-App\r\n", UID1, UID2, UID3, meta->id);
//	flash_erase(APP_BASE,65536);
//	for(int i=0;i<2048;i++)
//	{
//		spi_resp_buffer[i]=(i/16)%256;
//	}
//	for(int i=0;i<32;i++)
//	{
//		t=xTaskGetTickCount();
//		flash_write(APP_BASE+i*2032,spi_resp_buffer,2032);
//		t=xTaskGetTickCount()-t;
//		printf("writing %d in %d ms\r\n", i*2032,t);
//	}
//	printf("flash write done\r\n");


	wk_usart1_deinit();
//
//	vTaskDelay(200+5*meta->id);
//	wk_usart1_init();
//	printf("Debugging %d\r\n",meta->id);
//	t=xTaskGetTickCount();
//	int res=fw_hash_verify(buffer);
//	t=xTaskGetTickCount()-t;
//	printf("fw_hash_verify res=%d in %ld ms\r\n",res,t);

	CommEvent = xEventGroupCreate();
	if (CommEvent == NULL)
	{
//		printf("Fail to create CommEvent\r\n");
	}

	packet->payload.cmd = AFE_CMD_WHO_AM_I;
	packet->payload.response = 0;
	packet->payload.len = 0;
	packet->payload.offset = 0;

	for (int i = 0; i < 2048; i++)
	{
		spi_buffer[i]= i % 256;
	}

	/* ===== 创建命令队列（必须先于任务创建） ===== */
	g_cmd_queue = xQueueCreate(CMD_QUEUE_DEPTH, sizeof(AFEDataPacket));
	configASSERT(g_cmd_queue);

	/* ===== 作业队列 ===== */
	g_job_queue = xQueueCreate(1, sizeof(Job_t));
	configASSERT(g_job_queue);

	/* create cmdProcessor task */
	xTaskCreate(cmdProcessorTask, "cmdProcessor", 256, NULL, 22, &cmdProcessor_handle);

	/* create spi task */
	xTaskCreate(spiTask, "spi", 128, NULL, 24, &spi_handle);

	/* create worker task */
	xTaskCreate(workerTask, "Worker", 512, NULL, 20, NULL);

	/* add user code end StartDefaultTask 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin StartDefaultTask 1 */

		vTaskDelay(100);
		/* add user code end StartDefaultTask 1 */
  }
}


/**
 * @brief cmdProcessor function.
 * @param  none
 * @retval none
 */
/* ------------------------------------------------------------------------ */
/* Firmware page staging                                                     */
/*                                                                           */
/* flash_write() works a sector at a time: it reads the whole sector, and if  */
/* any of it is already programmed it erases and rewrites the sector.  The    */
/* packet carries at most 2032 bytes of payload, which is less than the 2048  */
/* byte sector, so writing each packet straight through means every call      */
/* straddles a sector boundary and forces an erase of a sector that was just  */
/* written - slow, and it wears the part.                                     */
/*                                                                           */
/* Buffering the payload into sector aligned, full sector writes avoids that: */
/* FW_BEGIN has already erased the image range, so every staged write lands   */
/* on blank cells and flash_write() skips its erase entirely.                 */
/* ------------------------------------------------------------------------ */

#define FW_PAGE_SIZE   2048U

static uint8_t  fw_page[FW_PAGE_SIZE];
static uint32_t fw_page_base;      /* image offset of the page being filled */
static uint32_t fw_page_fill;      /* bytes valid in fw_page                 */
static int      fw_page_active;

static void fw_page_reset(void)
{
	fw_page_active = 0;
	fw_page_fill = 0;
	fw_page_base = 0;
}

static error_status fw_page_flush(void)
{
	error_status st;

	if (!fw_page_active || (fw_page_fill == 0U))
	{
		return SUCCESS;
	}

	/* half words, as the Artery flash API counts them */
	st = flash_write(APP_BASE + fw_page_base, (uint16_t *) fw_page,
	                 (uint16_t) (fw_page_fill / 2U));

	fw_page_active = 0;
	fw_page_fill = 0;
	return st;
}

/* Stage len bytes of the image at image offset off.  Returns SUCCESS when the
 * bytes are buffered (not necessarily written yet - a partial page stays in
 * RAM until it fills or until fw_page_flush()). */
static error_status fw_page_stage(uint32_t off, const uint8_t *src, uint32_t len)
{
	while (len != 0U)
	{
		uint32_t page = off & ~(FW_PAGE_SIZE - 1U);
		uint32_t pos  = off & (FW_PAGE_SIZE - 1U);
		uint32_t space, n;

		if (!fw_page_active || (page != fw_page_base))
		{
			error_status st = fw_page_flush();
			if (st != SUCCESS)
			{
				return st;
			}
			memset(fw_page, 0xFF, sizeof(fw_page));
			fw_page_base = page;
			fw_page_fill = 0U;
			fw_page_active = 1;
		}

		space = FW_PAGE_SIZE - pos;
		n = (len < space) ? len : space;
		memcpy(fw_page + pos, src, n);
		if ((pos + n) > fw_page_fill)
		{
			fw_page_fill = pos + n;
		}

		off += n;
		src += n;
		len -= n;

		if (fw_page_fill == FW_PAGE_SIZE)
		{
			error_status st = fw_page_flush();
			if (st != SUCCESS)
			{
				return st;
			}
		}
	}
	return SUCCESS;
}

/**
  * @brief worker function.
  * @param  none
  * @retval none
  */
void workerTask(void *pvParameters)
{
  /* add user code begin workerTask 0 */
	uint8_t buffer[32];
    Job_t job;
  /* add user code end workerTask 0 */

  /* add user code begin workerTask 2 */

  /* add user code end workerTask 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin workerTask 1 */

	if (xQueueReceive(g_job_queue, &job, portMAX_DELAY) != pdPASS)
	 continue;

	 error_status ret;
	 if (job.type == JOB_ERASE) {
		 ret = flash_erase(job.addr+APP_BASE, job.size);
	 } else if (job.type == JOB_WRITE){
		 /* 走页缓冲，避免每包触发扇区擦除；单位在 flash_write 内已按 half-word 处理 */
		 ret = fw_page_stage(job.addr, (const uint8_t*) job.data, job.size);
		 //ret = flash_write(job.addr+APP_BASE, (const uint8_t*) job.data, job.size/2);

	 } else if (job.type == JOB_VERIFY){
		 /* 先 flush 未满一页的尾部，再做哈希校验 */
		 error_status st = fw_page_flush();
		 if (st != SUCCESS) {
			 ret = st;
		 } else {
			 ret = (fw_hash_verify(buffer) == 0) ? SUCCESS : ERROR;
//            	 ret = SUCCESS;
		 }

	 } else if (job.type == JOB_RESET){
		 vTaskDelay(100);
		 NVIC_SystemReset();
	 } else {
		 ret = ERROR;
	 }


	 if (job.data) {
		 vPortFree(job.data);
		 job.data = NULL;
	 }

	 g_job_error = ret;
	 g_job_state = (ret == SUCCESS) ? JOB_DONE : JOB_ERROR;

  /* add user code end workerTask 1 */
  }
}
void cmdProcessorTask(void *pvParameters)
{
  /* add user code begin cmdProcessorTask 0 */
	//AFEDataPacket *packet = (AFEDataPacket*) spi_resp_buffer;
	AFEDataPacket *packet=pvPortMalloc(sizeof(AFEDataPacket));
	AFEMetaDataDef *meta = (AFEMetaDataDef*) (0x0807F800);
	/* add user code end cmdProcessorTask 0 */

  /* add user code begin cmdProcessorTask 2 */

	/* add user code end cmdProcessorTask 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin cmdProcessorTask 1 */

		/* ===== 从命令队列阻塞取一条命令（不再用事件组，不会丢事件） ===== */
		if (xQueueReceive(g_cmd_queue, packet, portMAX_DELAY) != pdPASS)
		{
//		  printf("xQueueReceive(g_cmd_queue, packet, portMAX_DELAY) != pdPASS\r\n");
		  configASSERT(0);   /* or log "queue error" */
		}

		AFEFWParam    *fw        = (AFEFWParam*) packet->payload.data;
//		printf("id=%d,mem=%d,cmd=%d,resp=%d begin\r\n",meta->id,xPortGetFreeHeapSize(),packet->payload.cmd,packet->payload.response);
		if (packet->crc == crc_block_calculate_bytes((uint8_t*) &packet->payload,sizeof(AFEPacketPayload)))
		{
			switch (packet->payload.cmd)
			{
				case AFE_CMD_ECHO:break;
				case AFE_CMD_WHO_AM_I:
					LastCommand=AFE_CMD_WHO_AM_I;
					LastResponse=AFE_RESP_OK;
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;

				case AFE_CMD_SYNC:
					/* 先根据 job 状态更新 LastResponse，再写入 packet */
					switch(g_job_state)
					{
						case JOB_BUSY:  LastResponse=AFE_RESP_PENDING; break;
						case JOB_DONE:  LastResponse=AFE_RESP_OK;      g_job_state=JOB_IDLE; break;
						case JOB_ERROR: LastResponse=AFE_RESP_FAILED;  g_job_state=JOB_IDLE; break;
						default: break;
					}
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=AFE_CMD_SYNC;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;
				case AFE_RESET:
					LastCommand=AFE_RESET;
					if(g_job_state==JOB_IDLE)
					{

						Job_t job = {
							.type = JOB_RESET,
							.addr = 0,
							.data = NULL,
							.size = 0,
						};

						if (xQueueSend(g_job_queue, &job, 0) != pdPASS) {
							g_job_state  = JOB_IDLE;
							LastResponse = AFE_RESP_FAILED;
						} else {
							g_job_state  = JOB_BUSY;
							g_job_error  = 0;
							LastResponse = AFE_RESP_OK;
						}

					}else {
						LastResponse = AFE_RESP_BUSY;
					}
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));

					break;
				case AFE_CMD_FW_BEGIN:
					LastCommand=AFE_CMD_FW_BEGIN;
					if(g_job_state==JOB_IDLE)
					{
						if((fw->start_addr+fw->file_size)<446*1024)
						{
							Job_t job = {
								.type = JOB_ERASE,
								.addr = fw->start_addr,
								.data = NULL,
								.size = fw->file_size,
							};

							if (xQueueSend(g_job_queue, &job, 0) != pdPASS) {
								g_job_state  = JOB_IDLE;
								LastResponse = AFE_RESP_FAILED;
							} else {
								g_job_state  = JOB_BUSY;   /* 提交即置忙，消除竞态 */
								g_job_error  = 0;
								LastResponse = AFE_RESP_PENDING;
							}
						}else {
							LastResponse=AFE_RESP_FAILED;
						}
					}else {
						LastResponse = AFE_RESP_BUSY;
					}
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;

				case AFE_CMD_FW_READ:
					if(packet->payload.len<=sizeof(packet->payload.data))
					{
						flash_read(APP_BASE+packet->payload.offset,(uint16_t*)packet->payload.data,packet->payload.len);
						LastResponse=AFE_RESP_OK;
					}else
					{
						LastResponse=AFE_RESP_FAILED;
					}
					LastCommand=AFE_CMD_FW_READ;

					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;

				case AFE_CMD_FW_WRITE:
					LastCommand=AFE_CMD_FW_WRITE;
					if(g_job_state==JOB_IDLE)
					{
						if((packet->payload.offset+packet->payload.len<456704) &&
						   (packet->payload.len <= sizeof(packet->payload.data)))
						{
							uint8_t *data = pvPortMalloc(packet->payload.len);
							if (!data) {
								LastResponse=AFE_RESP_FAILED;
								g_job_state=JOB_IDLE;
		//								printf("AFE_RESP_FAILED1:%d\r\n",packet->payload.len);
								break;
							}
							memcpy(data,packet->payload.data, packet->payload.len);

							Job_t job = {
								.type = JOB_WRITE,
								.addr = packet->payload.offset,
								.data = data,
								.size = packet->payload.len,
							};

							if (xQueueSend(g_job_queue, &job, 0) != pdPASS) {
								g_job_state  = JOB_IDLE;
								vPortFree(data);
								LastResponse = AFE_RESP_FAILED;
		//								printf("AFE_RESP_FAILED2\r\n");
							} else {
								g_job_state  = JOB_BUSY;   /* 提交即置忙 */
								g_job_error  = 0;
								LastResponse = AFE_RESP_PENDING;
							}
						}else {
							LastResponse=AFE_RESP_FAILED;
		//							printf("AFE_RESP_FAILED3\r\n");
						}
					}else {
						LastResponse = AFE_RESP_BUSY;
					}
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;

				case AFE_CMD_FW_END:
					LastCommand=AFE_CMD_FW_END;
					if(g_job_state==JOB_IDLE)
					{
						if((fw->start_addr+fw->file_size)<446*1024)
						{
							Job_t job = {
								.type = JOB_VERIFY,
								.addr = 0,
								.data = NULL,
								.size = 0,
							};

							if (xQueueSend(g_job_queue, &job, 0) != pdPASS) {
								g_job_state  = JOB_IDLE;
								LastResponse = AFE_RESP_FAILED;
							} else {
								g_job_state  = JOB_BUSY;
								g_job_error  = 0;
								LastResponse = AFE_RESP_PENDING;
							}
						}else {
							LastResponse=AFE_RESP_FAILED;
						}
					}else {
						LastResponse=AFE_RESP_BUSY;
					}
					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;
				case AFE_ADC_READ:
					if(packet->payload.len<=sizeof(packet->payload.data))
					{
						if(ADC_Capture_Ready1&&ADC_Capture_Ready2)
						{
							if(packet->payload.offset+packet->payload.len<sizeof(ADC_Capture))
							{
								memcpy(packet->payload.data,&ADC_Capture[packet->payload.offset],packet->payload.len);
							}
							ADC_Capture_Ready1=0;
							ADC_Capture_Ready2=0;
							LastResponse=AFE_RESP_OK;
						}else
						{
							LastResponse=AFE_RESP_BUSY;
						}

					}else
					{
						LastResponse=AFE_RESP_FAILED;
					}
					LastCommand=AFE_ADC_READ;

					packet->sequence=packet->sequence+1;
					packet->payload.cmd=LastCommand;
					packet->payload.response=LastResponse;
					packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
					break;
				default:
					break;
			}

//			printf("AFE cmd=%d, resp=%d\r\n",packet->payload.cmd,packet->payload.response);
//			printf("id=%d,mem=%d,cmd=%d,resp=%d end\r\n",meta->id,xPortGetFreeHeapSize(),packet->payload.cmd,packet->payload.response);
			/* 把响应写回 spi_buffer，供 SPI 发送 */
			if(packet->payload.cmd!=AFE_CMD_ECHO)
			{	memcpy(spi_buffer, packet, sizeof(AFEDataPacket));

			}
		}else
		{
			packet->payload.response=AFE_RESP_FAILED;
			packet->crc=crc_block_calculate_bytes((uint8_t*) &packet->payload, sizeof(AFEPacketPayload));
			memcpy(spi_buffer, packet, sizeof(AFEDataPacket));
//			printf("AFE crc failed: crc=%d\r\n",packet->crc);
		}
		vPortFree(packet);

		/* add user code end cmdProcessorTask 1 */
  }
}


/**
  * @brief spi function.
  * @param  none
  * @retval none
  */
void spiTask(void *pvParameters)
{
  /* add user code begin spiTask 0 */
	EventBits_t uxBits;

  /* add user code end spiTask 0 */

  /* add user code begin spiTask 2 */

	SubFrameCount=2;


	/* add user code end spiTask 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin spiTask 1 */

		uxBits = xEventGroupWaitBits(CommEvent, EV_SPI_TX_DONE, pdTRUE, pdFALSE, portMAX_DELAY);

		if ((uxBits & EV_SPI_TX_DONE) != 0)
		{
			if (SubFrameIndex < SubFrameCount-1)
			{
				SubFrameIndex++;
			}
			else
			{
				SubFrameIndex = 0;

				/* ===== 一帧完整接收后把命令拷入队列；队列本身提供缓冲，不会丢事件 ===== */
				AFEDataPacket* packet=pvPortMalloc(sizeof(AFEDataPacket));
				if(packet)
				{
					memcpy((void*)packet, (void*)spi_buffer, sizeof(AFEDataPacket));
					if (xQueueSend(g_cmd_queue, packet, 0) != pdPASS)
					{
						/* 队列满：命令被丢弃 */
					}
				}
			}

		}

		/* add user code end spiTask 1 */
  }
}





/* add user code begin 2 */

/* add user code end 2 */

