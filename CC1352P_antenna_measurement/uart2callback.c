/*
 * This project creates a UART controlled RX/TX 2.4 GHz link, sending prop packages.
 */

/*
 *  ======== uart2callback.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* POSIX Header files */
#include <semaphore.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/rf/RF.h>

/* Driver configuration */
#include "ti_drivers_config.h"
#include "ti_radio_config.h"
#include "RFQueue.h"
/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Packet TX Configuration */
#define PAYLOAD_LENGTH      30
#ifdef POWER_MEASUREMENT
#define PACKET_INTERVAL     5  /* For power measurement set packet interval to 5s */
#else
#define PACKET_INTERVAL     500000  /* Set packet interval to 500000us or 500ms */
#endif
/* Packet RX Configuration */
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             40 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     3  /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * Max 30 payload bytes
                                   * 1 status byte (RF_cmdPropRx.rxConf.bAppendStatus = 0x1)
                                   * Also, we are including RSSI. */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN (rxDataEntryBuffer, 4);
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = 4
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)]
                                                  __attribute__((aligned(4)));
#else
#error This compiler is not supported.
#endif
/***** Prototypes *****/

/***** Variable declarations *****/
static RF_Object rfObject;
static RF_Handle rfHandle;

static uint8_t txPacket[PAYLOAD_LENGTH];
static uint16_t seqNumber;

/* UART semaphore */
static sem_t sem;
static volatile size_t numBytesRead;
/* Receive dataQueue for RF Core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;
static uint8_t rxPacket[MAX_LENGTH + NUM_APPENDED_BYTES - 1]; /* The length byte is stored in a separate variable */
static int8_t RSSI = -80;
/*
 *  ======== callbackFxn ========
 */
void callbackFxn(UART2_Handle handle, void *buffer, size_t count,
        void *userArg, int_fast16_t status)
{
    if (status != UART2_STATUS_SUCCESS) {
        /* RX error occured in UART2_read() */
        while (1);
    }

    numBytesRead = count;
    sem_post(&sem);
}

/*
 * Function to parse the input string from the UART console
 * Return true if parsing was OK, false if parsing wasn't OK
 */
bool parseInputString(char *string, uint32_t *frequency, uint8_t *tx){
    uint32_t freq = 0;
    if(string[0] == 'R' && string[1] == 'X'){
        // Set to transmit
        *tx = 0;
    }
    else if (string[0] == 'T' && string[1] == 'X'){
        // Set to receive
        *tx = 1;
    }
    else{
        // If neither, string cannot be parsed
        return false;
    }
    // String[2] should be a space, check this
    if(string[2] != ' '){
        return false;
    }
    // Check if the next four characters are numerical:
    int i = 0;
    for(i = 3; i < 7 ; i++){
        if(string[i] < '0' || string[i] > '9'){
            return false;
        }
    }
    // Finally, check that the last character is some sort of delimiter:
    if (string[7] == 0x0D || string[7] == 0x0A){
        return true;
    }
    else{
        return false;
    }

}

/* RF RX Callback */
void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (e & RF_EventRxEntryDone)
    {

        /* Get current unhandled data entry */
        currentDataEntry = RFQueue_getDataEntry();

        /* Handle the packet data, located at &currentDataEntry->data:
         * - Length is the first byte with the current configuration
         * - Data starts from the second byte */
        packetLength      = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        /* Copy the payload + the status byte to the packet variable */
        memcpy(rxPacket, packetDataPointer, (packetLength + 1));
        RSSI = packetDataPointer[packetLength];
        RFQueue_nextEntry();
    }
}
/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    static char       input[9];
    char              error[20];
    const char        echoPrompt[] = "Start Program\r\n";
    UART2_Handle      uart;
    UART2_Params      uartParams;
    int32_t           semStatus;
    uint32_t          status = UART2_STATUS_SUCCESS;
    uint8_t           tx = false; // if tx = 1, then send packet. If 0, then receive packet.
    uint32_t          freq = 0;   // Frequency to transmit/receive on.
    volatile RF_EventMask terminationReason; // To keep track of termination reasons.

    RF_Params rfParams;
    RF_Params_init(&rfParams);



    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create semaphore */
    semStatus = sem_init(&sem, 0, 0);

    if (semStatus != 0) {
        /* Error creating semaphore */
        while (1);
    }

    /* Create a UART in CALLBACK read mode */
    UART2_Params_init(&uartParams);
    uartParams.readMode = UART2_Mode_BLOCKING;
    uartParams.readCallback = callbackFxn;
    uartParams.baudRate = 115200;
    uartParams.readReturnMode = UART2_ReadReturnMode_FULL;


    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL) {
        /* UART2_open() failed */
        while (1);
    }

    rfHandle = RF_open(&rfObject, &RF_prop_custom2400_1, (RF_RadioSetup*)&RF_cmdPropRadioDivSetup_custom2400_1, &rfParams);
    /* Turn on user LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Pass NULL for bytesWritten since it's not used in this example */
    UART2_write(uart, echoPrompt, sizeof(echoPrompt), NULL);

    if( RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        /* Failed to allocate space for all data entries */
        while(1);
    }
    /* Loop forever echoing */
    while (1) {
        numBytesRead = 0;

        /* Pass NULL for bytesRead since it's not used in this example */
        //status = UART2_read(uart, &input, 1, NULL);
        status = UART2_readTimeout(uart, input, 8, &numBytesRead, 1000000);
        // UART2_STATUS_ETIMEOUT
        if(status == UART2_STATUS_ETIMEOUT){
            UART2_flushRx(uart);
            sprintf(error, "ERROR UART TIMEOUT\r\n");
            status = UART2_write(uart, &error, 20, NULL);
            if (status != UART2_STATUS_SUCCESS) {
                /* UART2_write() failed */
                while (1);
            }
            continue;
        }
        if (status != UART2_STATUS_SUCCESS) {
            /* UART2_read() failed */
            while (1);
        }


        /* Parse the input
         * The input is string based, and the input follows the following format:
         * <TX/RX> <Frequency in MHz>
         * So for example "TX 2450" will send one packet at 2450 MHz.
         * The RF RX timeout should be 2 seconds.
         * The UART RX timeout is forever. Newline is the delimiter.
         *
         * The status codes for when the code is done are in the following formats:
         * <OK> <RX/TX> <RSSI> for OK packages, where RSSI is only reported on RX.
         * <ERROR> <UART/RX/TIMEOUT/TX/FREQ> for errors, where UART is a parsing error, RX is an RF RX issue
         * e.g. wrong packet format, TIMEOUT is RF RX timeout and TX is error with TX. FREQ is wrong frequency
         *
         * */
        if (numBytesRead > 0){
            if(!parseInputString(input, &freq, &tx)){
                // Error: wasn't able to parse string
                // Flush the RX buffer
                UART2_flushRx(uart);
                //error="ERROR UART\r\n";
                sprintf(error, "ERROR UART TIMEOUT\r\n");
                status = UART2_write(uart, &error, 20, NULL);
                if (status != UART2_STATUS_SUCCESS) {
                    /* UART2_write() failed */
                    while (1);
                }
                continue;
            }
            else
            {
                freq = 1000*(input[3]-'0') + 100*(input[4]-'0') + 10*(input[5]-'0') + input[6]-'0';
                if(freq > 2480 || freq < 2400){
                    sprintf(error, "ERROR FREQ\r\n");
                    status = UART2_write(uart, &error, 12, NULL);
                    if (status != UART2_STATUS_SUCCESS) {
                        /* UART2_write() failed */
                        while (1);
                    }
                    continue;
                }

                // String parsed, set synth to correct frequency
                RF_cmdFs_custom2400_1.frequency = freq;
                // Not sure if this is needed, but set the synth in either TX or RX mode:
                RF_cmdFs_custom2400_1.synthConf.bTxMode = tx;
                RF_cmdFs_custom2400_1.startTrigger.triggerType = TRIG_NOW;
                // Run the command. This is blocking, so will not return until it's done.
                terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs_custom2400_1, RF_PriorityNormal, NULL, 0);
            }
        }

        // Either start RX or TX:
        if(tx){
            RF_cmdPropTx_custom2400_1.pktLen = PAYLOAD_LENGTH;
            RF_cmdPropTx_custom2400_1.pPkt = txPacket;
            RF_cmdPropTx_custom2400_1.startTrigger.triggerType = TRIG_NOW;
            /* Create packet with incrementing sequence number and random payload */
            txPacket[0] = (uint8_t)(seqNumber >> 8);
            txPacket[1] = (uint8_t)(seqNumber++);
            uint8_t i;
            for (i = 2; i < PAYLOAD_LENGTH; i++)
            {
                txPacket[i] = rand();
            }

            /* Send packet */
            terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx_custom2400_1,
                                                       RF_PriorityNormal, NULL, 0);

            switch(terminationReason)
            {
                case RF_EventLastCmdDone:
                    // A stand-alone radio operation command or the last radio
                    // operation command in a chain finished.
                    break;
                case RF_EventCmdCancelled:
                    // Command cancelled before it was started; it can be caused
                // by RF_cancelCmd() or RF_flushCmd().
                    break;
                case RF_EventCmdAborted:
                    // Abrupt command termination caused by RF_cancelCmd() or
                    // RF_flushCmd().
                    break;
                case RF_EventCmdStopped:
                    // Graceful command termination caused by RF_cancelCmd() or
                    // RF_flushCmd().
                    break;
                default:
                    // Uncaught error event
                    while(1);
            }

            uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropTx_custom2400_1)->status;
            switch(cmdStatus)
            {
                case PROP_DONE_OK:
                    // Packet transmitted successfully
                    break;
                case PROP_DONE_STOPPED:
                    // received CMD_STOP while transmitting packet and finished
                    // transmitting packet
                    break;
                case PROP_DONE_ABORT:
                    // Received CMD_ABORT while transmitting packet
                    break;
                case PROP_ERROR_PAR:
                    // Observed illegal parameter
                    break;
                case PROP_ERROR_NO_SETUP:
                    // Command sent without setting up the radio in a supported
                    // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
                    break;
                case PROP_ERROR_NO_FS:
                    // Command sent without the synthesizer being programmed
                    break;
                case PROP_ERROR_TXUNF:
                    // TX underflow observed during operation
                    break;
                default:
                    // Uncaught error event - these could come from the
                    // pool of states defined in rf_mailbox.h
                    while(1);
            }
            sprintf(error, "TX OK\r\n");
            status = UART2_write(uart, &error, 7, NULL);
            if (status != UART2_STATUS_SUCCESS) {
                /* UART2_write() failed */
                while (1);
            }
        }
        else{ // rx
            RF_cmdPropRx_custom2400_1.rxConf.bAppendRssi = 1;
            RF_cmdPropRx_custom2400_1.pQueue = &dataQueue;
            /* Discard ignored packets from Rx queue */
            RF_cmdPropRx_custom2400_1.rxConf.bAutoFlushIgnored = 1;
            /* Discard packets with CRC error from Rx queue */
            RF_cmdPropRx_custom2400_1.rxConf.bAutoFlushCrcErr = 1;
            /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
            RF_cmdPropRx_custom2400_1.maxPktLen = MAX_LENGTH;
            RF_cmdPropRx_custom2400_1.pktConf.bRepeatOk = 0;
            RF_cmdPropRx_custom2400_1.pktConf.bRepeatNok = 0;
            /* Timeout for RX packets */
            RF_cmdPropRx_custom2400_1.endTime = RF_RAT_TICKS_PER_US*10000000; // wait 10 seconds
            RF_cmdPropRx_custom2400_1.endTrigger.triggerType = TRIG_REL_SUBMIT;
            terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx_custom2400_1,
                                                           RF_PriorityNormal, &callback,
                                                           RF_EventRxEntryDone);
            switch(terminationReason)
            {
                case RF_EventLastCmdDone:
                    // A stand-alone radio operation command or the last radio
                    // operation command in a chain finished.
                    break;
                case RF_EventCmdCancelled:
                    // Command cancelled before it was started; it can be caused
                    // by RF_cancelCmd() or RF_flushCmd().
                    break;
                case RF_EventCmdAborted:
                    // Abrupt command termination caused by RF_cancelCmd() or
                    // RF_flushCmd().
                    break;
                case RF_EventCmdStopped:
                    // Graceful command termination caused by RF_cancelCmd() or
                    // RF_flushCmd().
                    break;
                default:
                    // Uncaught error event
                    while(1);
            }

            uint32_t cmdStatus = ((volatile RF_Op*)&RF_cmdPropRx_custom2400_1)->status;
            switch(cmdStatus)
            {
                case PROP_DONE_OK:
                    // Packet received with CRC OK
                    sprintf(error, "RX OK %d\r\n", RSSI);
                    status = UART2_write(uart, &error, 11, NULL);
                    break;
                case PROP_DONE_RXERR:
                    sprintf(error, "ERROR RX\r\n");
                    status = UART2_write(uart, &error, 10, NULL);
                    // Packet received with CRC error
                    break;
                case PROP_DONE_RXTIMEOUT:
                    sprintf(error, "ERROR RX TIMEOUT\r\n");
                    status = UART2_write(uart, &error, 18, NULL);
                    // Observed end trigger while in sync search
                    break;
                case PROP_DONE_BREAK:
                    // Observed end trigger while receiving packet when the command is
                    // configured with endType set to 1
                    break;
                case PROP_DONE_ENDED:
                    // Received packet after having observed the end trigger; if the
                    // command is configured with endType set to 0, the end trigger
                    // will not terminate an ongoing reception
                    break;
                case PROP_DONE_STOPPED:
                    // received CMD_STOP after command started and, if sync found,
                    // packet is received
                    break;
                case PROP_DONE_ABORT:
                    // Received CMD_ABORT after command started
                    break;
                case PROP_ERROR_RXBUF:
                    // No RX buffer large enough for the received data available at
                    // the start of a packet
                    break;
                case PROP_ERROR_RXFULL:
                    // Out of RX buffer space during reception in a partial read
                    break;
                case PROP_ERROR_PAR:
                    // Observed illegal parameter
                    break;
                case PROP_ERROR_NO_SETUP:
                    // Command sent without setting up the radio in a supported
                    // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
                    break;
                case PROP_ERROR_NO_FS:
                    // Command sent without the synthesizer being programmed
                    break;
                case PROP_ERROR_RXOVF:
                    // RX overflow observed during operation
                    break;
                default:
                    // Uncaught error event - these could come from the
                    // pool of states defined in rf_mailbox.h
                    while(1);
            }
            if (status != UART2_STATUS_SUCCESS) {
                /* UART2_write() failed */
                while (1);
            }
        }

    }
}
