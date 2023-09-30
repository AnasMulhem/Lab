
#include <stdio.h>
#include <string.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "platform.h"
#include "platform_config.h"
#include "xgpio.h"
#include "xgpiops.h"
#ifdef __arm__
#include "xil_printf.h"
#endif
#include "lwip/err.h"
#include "lwip/tcp.h"
struct tcp_pcb* active_client_pcb = NULL;

/***************DEFINE******************/
//Define IP packet Message ID:

#define SWITCH_A 0
#define	WR_OLED 2
#define	WR_LED 3
#define WR_7SEG 4
#define RD_FILE 5
#define WR_TIMESTAMP 7
#define SENT_TO_HOST 8
#define HEATER 9  //to control the heater remotly

//Read file:
#define FILE_LINE_NUM 16          //determine the number of line to be read from the recieved file
#define LINE_CHAR_NUM 15          //determine the number of chars at each file line
#define CHAR_NUM_COL1 6           //determine the number of char at first line word
#define NUM_OF_SPACE 1            //determine the number of space between the line words
#define END_LINE_CHARS 2          //determine the number of end line chars, \n\r
#define DATA_OFFSET 5             //determine the offset of the data from the packet head
#define ASCII_BIGLETTER_VAL 64    //Big letters offset in ascii table
#define ZERO_ASCII_VAL 48         //
#define A_ASCII_TO_10 55          //The "distance" between A to 10 in ascii table
/************END-DEFINE*****************/

/* GPIO Globals */
/*Global variables declaration*/
int heater = 0; //heater is on from home
int sender_heater = 0; //heater is on remotly
int time_opened = 0;   // in which time the door opened
int print_once=0; //to print the close door once

XGpio led_sws_gpio_inst;
XGpioPs psGpioInstancePtr;

XGpioPs_Config* GpioConfigPtr;
int old_state_of_switch = 0; // old state switch
int door_state_flag = 1; // it mean that switch 0 changed
int xStatus;
int iPinNumberEMIO = 54;
u32 uPinDirectionEMIO = 0x0;
u32 uPinDirection = 0x1;
unsigned int code_entered;

/*Variables declarations for time counters and file reading*/
long long unsigned int time_value, time_sec, time_sec2;
u32 time_1;
u32 time_2;
int reservation[16];
int assign_code[16];
char key_buffer[64];


/* Add Your Global variables */



/*Transfare_data- this function allocate the data structure
and initiate the data transfer to the Zedbourd */
int transfer_data(void* data, unsigned int len) {
	err_t err;
	struct pbuf* transfer_pbuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	pbuf_take(transfer_pbuf, data, len);
	err = tcp_write(active_client_pcb, transfer_pbuf->payload, transfer_pbuf->len, 1);
	tcp_output(active_client_pcb);
	pbuf_free(transfer_pbuf);
	return err;
}


///function that reurn the current time in secend
/// /*****************************************************************************************
int current_time()
{
	u32 time_1;
	u32 time_2;
	long long unsigned int time_value;
	long long unsigned int time_sec;
	ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
	time_2 = *(time_count_baseaddr + 0);
	time_1 = *(time_count_baseaddr + 1);
	time_value = time_2;
	time_value <<= 32;
	time_value = time_value + time_1;
	time_value = time_value / 1000;
	time_value = time_value + 7200;

	return time_value;
}

//****************************************************************************************************
void print_app_header()
{
	xil_printf("\n\r\n\r-----lwIP TCP echo server ------\n\r");
	xil_printf("TCP packets sent to port 6001 will be echoed back\n\r");
}

/*recv_callback- handle the receiving packets, getting the pcb structure
that contain the ID of the event (the type of the data), packet length and data
Predefined ID numbers (data[4]):
0.	Initiate read from switches A
1.	Initiate read from switches B
2.	Write to a line on OLED
3.	Write to the 8 LEDs
4.	Write 2 digits to a seven segment display
5.	Write the content of a file to the board
6.	When receiving a message, display key-pad pressed and single digit hex value.
7.	Write the timestamp as 64 bit value representing the number of milliseconds past since 1/1/1970.
8.	When receiving a message, display the bytes as asci text in the message field.
 */
err_t recv_callback(void* arg, struct tcp_pcb* tpcb,
	struct pbuf* p, err_t err)
{
	unsigned char* received_data;
	u16_t received_data_len;

	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);

	/* Allocate buffer for received data and copy it. */
	received_data = (unsigned char*)malloc(p->len);
	memcpy(received_data, p->payload, p->len);
	received_data_len = p->len;

	/* ****************************************************************************************************************************************************************
	 * User Logic start
	 * Handle received_data here.
	 *
	 * Example logic: echo server - send all received packets back:
	 * transfer_data(received_data, received_data_len);
	 * ************************************************************
	 */

	ULONG message_len = *((ULONG*)received_data); // take recived data, 6


	if ((message_len == 1) && received_data[4] == SWITCH_A) {
		// Switches A read request.
		// Read switches.
		unsigned char switches_a = (unsigned char)XGpio_DiscreteRead(&led_sws_gpio_inst, 2);
		// Build response message and send it.
		u16_t response_message_len = 4 + 1 + 1;
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 2;
		response_message[4] = 0;
		response_message[5] = switches_a;
		transfer_data(response_message, response_message_len);
		free(response_message);
	}

	else if (received_data[4] == WR_OLED) {
		// OLED write request.
		// Build the buffer to write.
		unsigned int line_number = received_data[5];

		u16_t string_len = message_len - 2; // Subtracting size of messageID and line number.
		if (string_len <= 16) {
			unsigned char* string_buffer = (unsigned char*)malloc(string_len + 1); // With a place for '\0'.
			memcpy(string_buffer, received_data + 6, string_len); // 6-> 4 byte + type + line
			string_buffer[string_len] = '\0';

			// Write to OLED.
			clear();
			print_message(string_buffer, 1);

			free(string_buffer);
		}
	}
	else if ((message_len == 2) && received_data[4] == WR_LED) {
		// LEDs write request.
		// Write to LEDs.
		XGpio_DiscreteWrite(&led_sws_gpio_inst, 1, received_data[5]);
	}
	else if (received_data[4] == RD_FILE) {
		//Save file to memory array.
		int temp, i, k;
		unsigned int data = 0;
		// receive file with reservation data.
		// Build the buffer to write.
		unsigned char* my_data = (unsigned char*)malloc(LINE_CHAR_NUM);
		// 2* 6 char + 1 space + carrige-return + line-feed = 15 byte per record
		for (k = 1; k < FILE_LINE_NUM; k++)
		{
			memcpy(my_data, received_data + DATA_OFFSET + ((k - 1) * LINE_CHAR_NUM), LINE_CHAR_NUM);
			for (i = 0; i < CHAR_NUM_COL1; i++)
			{
				data <<= 4;
				if (my_data[i] < ASCII_BIGLETTER_VAL)
					// ascii numbers are between 48 and 57 decimal value
					temp = my_data[i] - ZERO_ASCII_VAL;

				else
					// ascii big letters are above 64 decimal value
					temp = my_data[i] - A_ASCII_TO_10;

				data = data + temp;
			}
			reservation[k] = data;
			data = 0;
			for (i = CHAR_NUM_COL1 + NUM_OF_SPACE; i < LINE_CHAR_NUM - END_LINE_CHARS; i++)
			{
				data <<= 4;
				if (my_data[i] < ASCII_BIGLETTER_VAL)
					temp = my_data[i] - ZERO_ASCII_VAL;

				else
					temp = my_data[i] - A_ASCII_TO_10;

				data = data + temp;
			}
			assign_code[k] = data;
			data = 0;

		}

		free(my_data);

	}


	else if ((received_data[4] == WR_TIMESTAMP))
	{
		// Receive Timestamp and write it to the counters
		unsigned long long  int timestamp = 0;
		unsigned long int timeh = 0;
		unsigned long int timel = 0;
		long  unsigned int time_value;
		int i, temp;
		// receive 8 byte time of day value.
		unsigned char* my_data = (unsigned char*)malloc(8);
		memcpy(my_data, received_data + 5, 8);

		for (i = 7; i >= 4; i--)
		{
			timeh <<= 8;
			temp = my_data[i];
			timeh = timeh + temp;
		}
		for (i = 3; i >= 0; i--)
		{
			timel <<= 8;
			temp = my_data[i];
			timel = timel + temp;
		}
		ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;

		*(time_count_baseaddr + 0) = timeh;
		*(time_count_baseaddr + 1) = timel;

		free(my_data);
	}
	else if (received_data[4] == WR_7SEG) {
		ULONG* seven_baseaddr = (ULONG*)XPAR_SEVEN_SEGMENT_0_S00_AXI_BASEADDR;
		int num = received_data[5];
		*(seven_baseaddr + 0) = 0x0200 + received_data[5]; //0x200 to appear in 7 segments


	}






	//***************************************************************************************************
	else if (received_data[4] == HEATER)   //if recevied id = 9 turn of the heater if the code is correct
	{
		int current_t=current_time();
		unsigned int temp = received_data[6];
		int need_to_wait= current_t + (temp>>4)*10 + (temp & 15 ) ;  //to calclate until which time should the boiler be ON
		if(received_data[5]==23){ //17 in hexa is 23 in decimal
		sender_heater = 1; //heater work remotly
		ULONG* seven_baseaddr = (ULONG*)XPAR_SEVEN_SEGMENT_0_S00_AXI_BASEADDR;
		*(seven_baseaddr + 0) = 0x0200 + 0x88; //0x200 to appear in 7 segments
		//to print the time
		u32 time_1;
		u32 time_2;
		long long unsigned int time_value;
		long long unsigned int time_sec;
		ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
		time_2 = *(time_count_baseaddr + 0);
		time_1 = *(time_count_baseaddr + 1);
		time_value = time_2;
		time_value <<= 32;
		time_value = time_value + time_1;
		time_value = time_value / 1000;
		time_value = time_value + 7200; // for Israel time GMT+2 ;
		xil_printf(" %s  Heater is on \n\r", ctime(&time_value));
		sprintf(key_buffer, "%s, Heater is on \n", ctime(&time_value));
		u16_t response_message_len = 4 + 1 + 64; // to build the message
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);

		//wait untill the requested time TT
         while(current_t <= need_to_wait )
         {
        	 current_t=current_time();
         }

 		*(seven_baseaddr + 0) = 0x0400; //0x400 to  7 segments is off
		  sender_heater = 0; // heater is off remotly


		}
	}
	//*************************************************************************************************************************************************
	

	 /* free the received pbuf */
	pbuf_free(p);

	/* Free the allocated buffer. */
	free(received_data);

	return ERR_OK;
}

// accept_callback- received new pcd struct and bind it to the recv_callback
err_t accept_callback(void* arg, struct tcp_pcb* newpcb, err_t err)
{
	static int connection = 1;

	/* set the receive callback for this connection */
	tcp_recv(newpcb, recv_callback);

	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)connection);

	/* increment for subsequent accepted connections */
	connection++;

	/* Save the new client pcb. */
	active_client_pcb = newpcb;

	return ERR_OK;
}

/*start_application-  */
int start_application()
{
	struct tcp_pcb* pcb;
	err_t err;
	unsigned port = 54321;

	/* create new TCP PCB structure */
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP echo server started @ port %d\n\r", port);

	return 0;
}

/* missing declaration in lwIP */
void lwip_init();

static struct netif server_netif;
struct netif* echo_netif;

void print_ip(char* msg, ip_addr_t* ip)
{
	print(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
		ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t* ip, ip_addr_t* mask, ip_addr_t* gw)
{

	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

#ifdef __arm__
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
int ProgramSi5324(void);
int ProgramSfpPhy(void);
#endif
#endif


#define KEYPAD_ALL_RELEASED 0x00000000
ULONG released_state = KEYPAD_ALL_RELEASED;
ULONG released_state_prev = KEYPAD_ALL_RELEASED;
ULONG *sound_baseaddr_so = (ULONG *)XPAR_SOUND1_0_S00_AXI_BASEADDR;

void handle_events() {

	ULONG* btns_baseaddr_p = (ULONG*)XPAR_BTNS_0_S00_AXI_BASEADDR;
	int btn = *(btns_baseaddr_p + 0);


	if ((btn > 0))
	{
		switch (btn)
		{
		case 1:
			xil_printf("%s, Left Button was pressed \n", ctime(&time_value));
			sprintf(key_buffer, "%s, Left Button was pressed \n", ctime(&time_value));
			break;
		case 2:
			xil_printf("%s, Right Button was pressed \n", ctime(&time_value));
			sprintf(key_buffer, "%s, Right Button was pressed \n", ctime(&time_value));

			break;
		case 4:
			xil_printf("%s, Upper Button was pressed \n", ctime(&time_value));
			sprintf(key_buffer, "%s, Upper Button was pressed \n", ctime(&time_value));

			break;
		case 8:
			xil_printf("%s,  Lower was pressed \n", ctime(&time_value));
			sprintf(key_buffer, "%s,  Lower Button was pressed \n", ctime(&time_value));

			break;
		case 16:
			xil_printf("%s, Center Button was pressed \n", ctime(&time_value));
			sprintf(key_buffer, "%s, Center Button was pressed \n", ctime(&time_value));

			break;
		}

		//time display

		u32 time_1;
		u32 time_2;
		long long unsigned int time_value;
		long long unsigned int time_sec;
		ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
		time_2 = *(time_count_baseaddr + 0);
		time_1 = *(time_count_baseaddr + 1);
		time_value = time_2;
		time_value <<= 32;
		time_value = time_value + time_1;
		time_value = time_value / 1000;
		time_value = time_value + 7200;

		//prepare the messeage
		u16_t response_message_len = 4 + 1 + 64;
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);
	}

	//****************************************************************************************************************************
	//do the heater from home irrilavt to remotly
	if (btn == 2 && !sender_heater)
	{
		heater = !heater; // when right button pressed need to change the heater is state
		if (heater ) // if we need to turn the heater on
		{

			ULONG* seven_baseaddr = (ULONG*)XPAR_SEVEN_SEGMENT_0_S00_AXI_BASEADDR;
			*(seven_baseaddr + 0) = 0x0200 + 0x88; //0x200 to appear in 7 segments
			u32 time_1;
			u32 time_2;
			long long unsigned int time_value;
			long long unsigned int time_sec;
			ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
			time_2 = *(time_count_baseaddr + 0);
			time_1 = *(time_count_baseaddr + 1);
			time_value = time_2;
			time_value <<= 32;
			time_value = time_value + time_1;
			time_value = time_value / 1000;
			time_value = time_value + 7200; // for Israel time GMT+2 ;
			xil_printf(" %s  Heater is on \n\r", ctime(&time_value));
			sprintf(key_buffer, "%s, Heater is on \n", ctime(&time_value));
			u16_t response_message_len = 4 + 1 + 64; // to build the message
			unsigned char* response_message = (unsigned char*)malloc(response_message_len);
			*((ULONG*)response_message) = 65;
			response_message[4] = SENT_TO_HOST;
			memcpy(response_message + 5, key_buffer, 64);
			transfer_data(response_message, response_message_len);
			free(response_message);

		}
		else if (heater == 0) //turn the heater off
		{
			ULONG* seven_baseaddr = (ULONG*)XPAR_SEVEN_SEGMENT_0_S00_AXI_BASEADDR;
			*(seven_baseaddr + 0) = 0x0400; //0x400 to 7 segments off
			u32 time_1;
			u32 time_2;
			long long unsigned int time_value;
			long long unsigned int time_sec;
			ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
			time_2 = *(time_count_baseaddr + 0);
			time_1 = *(time_count_baseaddr + 1);
			time_value = time_2;
			time_value <<= 32;
			time_value = time_value + time_1;
			time_value = time_value / 1000;
			time_value = time_value + 7200; // for Israel time GMT+2 ;
			xil_printf(" %s  Heater is OFF \n\r", ctime(&time_value));
			sprintf(key_buffer, "%s, Heater is OFF \n", ctime(&time_value));
			u16_t response_message_len = 4 + 1 + 64; // to build the message
			unsigned char* response_message = (unsigned char*)malloc(response_message_len);
			*((ULONG*)response_message) = 65;
			response_message[4] = SENT_TO_HOST;
			memcpy(response_message + 5, key_buffer, 64);
			transfer_data(response_message, response_message_len);
			free(response_message);
		}
	}


	int new_switch = old_state_of_switch;
	old_state_of_switch = (unsigned char)XGpio_DiscreteRead(&led_sws_gpio_inst, 2);
	 //once the switch state change
	if (new_switch != old_state_of_switch) {
		// Switches A read request.
		// Read switches.
		unsigned char switches_a = (unsigned char)XGpio_DiscreteRead(&led_sws_gpio_inst, 2);
		u32 time_1;
		u32 time_2;
		long long unsigned int time_value;
		long long unsigned int time_sec;
		ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
		time_2 = *(time_count_baseaddr + 0);
		time_1 = *(time_count_baseaddr + 1);
		time_value = time_2;
		time_value <<= 32;
		time_value = time_value + time_1;
		time_value = time_value / 1000;
		time_value = time_value + 7200; // for Israel time GMT+2 ;
		xil_printf(" %s  Key pressed: 0x%01x \n\r", ctime(&time_value), switches_a);
		sprintf(key_buffer, "%s, Switches changed to : 0x%x \n", ctime(&time_value), switches_a);
		u16_t response_message_len = 4 + 1 + 64; // to build the message
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);

	}


  //door state changes (only first switch)
	if ((old_state_of_switch & 0x1) != (new_switch & 0x1)) {
		door_state_flag = 1;
	}
	//if the door opened print and update which time opend
	unsigned char switches_a = (unsigned char)XGpio_DiscreteRead(&led_sws_gpio_inst, 2);
	if (((switches_a & 0x1) == 1) && door_state_flag)
	{
		char string_buffer[10] = "Door open";
		string_buffer[9]=0;
		// Write to OLED.
		clear();
		print_message(string_buffer, 0);
		u32 time_1;
				u32 time_2;
				long long unsigned int time_value;
				long long unsigned int time_sec;
				ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
				time_2 = *(time_count_baseaddr + 0);
				time_1 = *(time_count_baseaddr + 1);
				time_value = time_2;
				time_value <<= 32;
				time_value = time_value + time_1;
				time_value = time_value / 1000;
				time_value = time_value + 7200; // for Israel time GMT+2 ;
		time_opened = time_value;
		xil_printf(" %s  Door open \n\r", ctime(&time_value));
		sprintf(key_buffer, "%s,  Door  open  \n", ctime(&time_value));
		u16_t response_message_len = 4 + 1 + 64; // to build the message
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);
		door_state_flag = 0;
	}
	u32 current_t = current_time(); //current time
	//if it open and more than 5s open the sound
	if (((current_t - time_opened) >= 5) && ((switches_a & 0x1) == 1) )
	{
		int s;
		s = 0x9;
		*(sound_baseaddr_so + 0) = s;

        //print the alaram once
		if(print_once==1) {
		u32 time_1_alarms;
						u32 time_2_alarms;
						long long unsigned int time_value_alarms;
						long long unsigned int time_sec_alarms;
						ULONG* time_count_baseaddr_alarms = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
						time_2_alarms = *(time_count_baseaddr_alarms + 0);
						time_1_alarms = *(time_count_baseaddr_alarms + 1);
						time_value_alarms = time_2_alarms;
						time_value_alarms <<= 32;
						time_value_alarms = time_value_alarms + time_1_alarms;
						time_value_alarms = time_value_alarms / 1000;
						time_value_alarms = time_value_alarms + 7200; // for Israel time GMT+2 ;
						xil_printf(" %s  Door open more than 5s \n\r", ctime(&time_value_alarms));
						sprintf(key_buffer, "%s, Door open more than 5s \n", ctime(&time_value_alarms) );
						u16_t response_message_len = 4 + 1 + 64; // to build the message
						unsigned char* response_message = (unsigned char*)malloc(response_message_len);
						*((ULONG*)response_message) = 65;
						response_message[4] = SENT_TO_HOST;
						memcpy(response_message + 5, key_buffer, 64);
						transfer_data(response_message, response_message_len);
						free(response_message);
						print_once=0;
		}
	}


//door closed
	if (((switches_a & 0x1) == 0) && door_state_flag)
	{
		time_opened=current_t;
		char string_buffer[11] = "Door close";
		string_buffer[10]=0;
		// Write to OLED.
		clear();
		print_message(string_buffer, 0);
		int s;
		s = 0x0;
		*(sound_baseaddr_so + 0) = s;
		door_state_flag = 0;
		u32 time_1_closed;
		u32 time_2_closed;
		long long unsigned int time_value_closed;
		long long unsigned int time_sec_closed;
		ULONG* time_count_baseaddr_closed = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
		time_2_closed = *(time_count_baseaddr_closed + 0);
		time_1_closed = *(time_count_baseaddr_closed + 1);
		time_value_closed = time_2_closed;
		time_value_closed <<= 32;
		time_value_closed = time_value_closed + time_1_closed;
		time_value_closed = time_value_closed / 1000;
		time_value_closed = time_value_closed + 7200; // for Israel time GMT+2 ;
		xil_printf(" %s  Door close  \n\r", ctime(&time_value_closed));
		sprintf(key_buffer, "%s, Door close  \n", ctime(&time_value_closed));
		u16_t response_message_len = 4 + 1 + 64; // to build the message
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);
		door_state_flag = 0;
        print_once=1;
	}

//*************************************************************************************************************************************

	// KEYPAD

	ULONG* keypad_baseaddr_p = (ULONG*)XPAR_MY_KEYBOARD_0_S00_AXI_BASEADDR;
	// Update keys and released_states.
	released_state_prev = released_state;
	released_state = *(keypad_baseaddr_p + 1);
	int key = *(keypad_baseaddr_p + 0);


	// Check if keypad has just been released.
	if ((released_state == KEYPAD_ALL_RELEASED) && (released_state_prev != KEYPAD_ALL_RELEASED))
	{

		int s, i, k;
		s = 0x9;
		*(sound_baseaddr_so + 0) = s;
		for (i = 0; i < 30000; i++); // delay loop
		s = 0x0;
		*(sound_baseaddr_so + 0) = s;



		u32 time_1;
		u32 time_2;
		long long unsigned int time_value;
		long long unsigned int time_sec, new_time;
		ULONG* time_count_baseaddr = (ULONG*)XPAR_TIME_COUNT_0_S00_AXI_BASEADDR;
		time_2 = *(time_count_baseaddr + 0);
		time_1 = *(time_count_baseaddr + 1);
		time_value = time_2;
		time_value <<= 32;
		time_value = time_value + time_1;
		time_value = time_value / 1000;
		time_value = time_value + 7200; //  for summertime 10800;

		xil_printf(" %s  Key pressed: 0x%01x \n\r", ctime(&time_value), key);

		sprintf(key_buffer, "%s, Key pressed : 0x%01x \n", ctime(&time_value), key);
		u16_t response_message_len = 4 + 1 + 64;
		unsigned char* response_message = (unsigned char*)malloc(response_message_len);
		*((ULONG*)response_message) = 65;
		response_message[4] = SENT_TO_HOST;
		memcpy(response_message + 5, key_buffer, 64);
		transfer_data(response_message, response_message_len);
		free(response_message);
		if (key == 0x1)
		{
			for (k = 1; k < 16; k++)
			{
				xil_printf("Reservation number: %x  Room number: %x  Room Code: %x \n\r", reservation[k], assign_code[k] >> 16, assign_code[k] - ((assign_code[k] >> 16) << 16));
			}

		}
	}


}


/* **************
 * User Logic end
 * **************
 */


int main()
{
	ip_addr_t  ipaddr, netmask, gw;

	// AXI GPIO leds & switches  Intialization
	XGpio_Initialize(&led_sws_gpio_inst, XPAR_LED_SWS_DEVICE_ID);

	// PS GPIO Initialization
	GpioConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	if (GpioConfigPtr == NULL) {
		return XST_FAILURE;
	}
	xStatus = XGpioPs_CfgInitialize(&psGpioInstancePtr, GpioConfigPtr, GpioConfigPtr->BaseAddr);
	if (xStatus != XST_SUCCESS) {
		print("PS GPIO INIT FAILED\n\r");
	}


	// EMIO PIN Setting to Input port
	XGpioPs_SetDirectionPin(&psGpioInstancePtr, iPinNumberEMIO, uPinDirectionEMIO);
	XGpioPs_SetOutputEnablePin(&psGpioInstancePtr, iPinNumberEMIO, 0);


	/* the mac address of the board. this should be unique per board */
	unsigned char mac_ethernet_address[] =
	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };

	echo_netif = &server_netif;
#ifdef __arm__
#if XPAR_GIGE_PCS_PMA_SGMII_CORE_PRESENT == 1 || XPAR_GIGE_PCS_PMA_1000BASEX_CORE_PRESENT == 1
	ProgramSi5324();
	ProgramSfpPhy();
#endif
#endif

	init_platform();

	/* initialize IP addresses to be used */
	IP4_ADDR(&ipaddr, 192, 168, 0, 1);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 192, 168, 0, 1);

	print_app_header();

	lwip_init();

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(echo_netif, &ipaddr, &netmask,
		&gw, mac_ethernet_address,
		PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n\r");
		return -1;
	}
	netif_set_default(echo_netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	/* specify that the network if is up */
	netif_set_up(echo_netif);

	print_ip_settings(&ipaddr, &netmask, &gw);

	/* start the application (web server, rxtest, txtest, etc..) */
	start_application();

	while (1) {
		/* receive and process packets-
		The receive interrupt handlers move the packet data from the MAC and store them in a queue.
		The xemacif_input function takes those received packets from the queue, and passes them to lwIP
		NOTICE- The program is notified of the received data through callbacks.*/
		xemacif_input(echo_netif);

		/* Handle other events. */
		handle_events();
	}

	/* never reached */
	cleanup_platform();

	return 0;

}
/* **************
 * User Logic end
 * **************
 */
