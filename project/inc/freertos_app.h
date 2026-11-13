/* add user code begin Header */
/**
  ******************************************************************************
  * File Name          : freertos_app.h
  * Description        : Code for freertos applications
  */
/* add user code end Header */
  
#ifndef FREERTOS_APP_H
#define FREERTOS_APP_H

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"
#include "wk_system.h"

/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */
#include "flash.h"
/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
/* add user code begin exported types */

typedef struct
{
	uint8_t hash[32];
	int id;
	uint8_t reserved[2012];
} AFEMetaDataDef;

typedef struct
{
    uint8_t cmd;
    uint8_t response;
    uint16_t len;

    union {
        uint32_t offset;
        uint32_t index;
    };

    uint8_t data[2032];
} AFEPacketPayload;

typedef struct
{
	uint32_t start_addr;
	uint32_t file_size;
}AFEFWParam;

typedef struct
{
    uint32_t sequence;
    uint32_t crc;
    AFEPacketPayload payload;
} AFEDataPacket;


_Static_assert(sizeof(AFEPacketPayload) == 2040, "Payload size error");
_Static_assert(offsetof(AFEDataPacket, payload) == 8, "Payload offset error");
_Static_assert(sizeof(AFEDataPacket) == 2048, "Packet size error");


typedef enum{
	AFE_CMD_ECHO=0,
	AFE_CMD_WHO_AM_I,
	AFE_CMD_SYNC,
	AFE_RESET,
	AFE_RUN,
	AFE_CMD_FW_BEGIN,
	AFE_CMD_FW_READ,
	AFE_CMD_FW_WRITE,
	AFE_CMD_FW_END,
}AFECmd;

typedef enum{
	AFE_RESP_OK=0,
	AFE_RESP_FAILED,
	AFE_RESP_PENDING,
	AFE_RESP_CRC_FAILED,
}AFEResponse;


typedef enum {
    JOB_IDLE = 0,
    JOB_BUSY,
    JOB_DONE,
    JOB_ERROR
} JobState_t;

typedef enum {
    JOB_ERASE,
    JOB_WRITE,
	JOB_VERIFY
} JobType_t;

typedef struct {
    JobType_t type;
    uint32_t  addr;
    uint8_t  *data;
    uint32_t  size;
} Job_t;



/* add user code end exported types */

/* exported constants --------------------------------------------------------*/
/* add user code begin exported constants */

/* add user code end exported constants */

/* exported macro ------------------------------------------------------------*/
/* add user code begin exported macro */
#define EV_SPI_RX_DONE 		(1<<0)
#define EV_SPI_TX_DONE 		(1<<1)
#define EV_CMD_READY 		(1<<2)
#define EV_ADC1_DATA_RDY 	(1<<3)
/* add user code end exported macro */

/* task handler */
extern TaskHandle_t defaultTask_handle;
extern TaskHandle_t cmdProcessor_handle;
extern TaskHandle_t spi_handle;
extern TaskHandle_t worker_handle;
/* declaration for task function */
void StartDefaultTask(void *pvParameters);
void cmdProcessorTask(void *pvParameters);
void spiTask(void *pvParameters);
void workerTask(void *pvParameters);

/* add user code begin 0 */
extern int SubFrameIndex;
extern EventGroupHandle_t CommEvent;
extern volatile uint8_t spi_buffer[2048];
extern volatile uint16_t adc1_buffer[512];
/* add user code end 0 */

void freertos_task_create(void);
void wk_freertos_init(void);

/* add user code begin 1 */

/* add user code end 1 */

#endif /* FREERTOS_APP_H */
