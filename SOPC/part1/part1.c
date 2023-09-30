#include <stdio.h>
#include "xil_types.h"
#include "xil_assert.h"
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"


ULONG *baseaddr_p = (ULONG *)XPAR_MY_KEYPAD_0_S00_AXI_BASEADDR;


int main()
{
 int  key;
 int released;
 char key_buffer[32];
 int i,sw_check;
 XGpio led_sws;
 int key_released;


  	
   // AXI GPIO  Intialization
   
 XGpio_Initialize(&led_sws, XPAR_LED_SWS_DEVICE_ID);



  init_platform();

  xil_printf("-- Start of the Program --\r\n");
  xil_printf("-- Press Keypad to see the output on OLED --\r\n");
  xil_printf("-- Change slide switches to see corresponding output on LEDs --\r\n");


  
  i=0;
int flag;
  while (1)
    	{
			
    		key = *(baseaddr_p+0);			//Read key from KEYPAD internal register 00
			
    		
    		released = *(baseaddr_p+1);		//Read released from KEYPAD internal register 01
            
			// Add your code here. 
			// The code should set the value of the key_released variable according to your keypad implementation.
			
              if(released!=0)
			   flag=1;
    		if (released==0 && flag)
    			{
    				xil_printf("Key pressed: 0x%08x \n\r", key);  //Write to terminal windows
    				sprintf(key_buffer, "Key pressed: %X", key);  //Write formatted string to key_buffer 
    				
    			    if (i==0)
    			    	clear();								//Clear OLED display on first line
    				print_message(key_buffer,i); 				//Write a string in 'key_buffer' to line 'i' on OLED (total - 4 lines) 
     				i++;										
    				if (i==4)
    					   i=0;

    			}
                  
				  flag=0;
		   
		 sw_check = XGpio_DiscreteRead(&led_sws, 2);			//READ from SWITCHES GPIO port=2
		   
		 XGpio_DiscreteWrite(&led_sws, 1, sw_check);			//WRITE to LREDS GPIO port=1

    	}

    cleanup_platform();
    return 0;
}





