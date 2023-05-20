#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "tm4c123gh6pm.h"
#include "GPIO.h"
#include "types.h"
#include "queue.h"
#include "semphr.h"
#include "Jam&Lock.h"
#define mainSW_INTURRUPT_PortF ((IRQn_Type)30)

#define GPIO_PORTF_CLK_EN  0x20
#define GPIO_PORTF_PIN1_EN 0x02
#define GPIO_PORTF_PIN2_EN 0x04
#define GPIO_PORTF_PIN3_EN 0x08
#define RED                0x02
#define BLUE               0x04
#define GREEN              0x08

#define DELAY_VALUE        5000000  

int queueV = 1;



/*TickType_t time_count_start;
TickType_t time_count_end;*/


xSemaphoreHandle ctrlSemaphore; //Semaphore for passenger doown auto
xSemaphoreHandle dumSemaphore; //Semaphore for driver up manual
xSemaphoreHandle ddmSemaphore; //Semaphore for driver down manual
xSemaphoreHandle duaSemaphore; //Semaphore for driver up auto
xSemaphoreHandle ddaSemaphore; //Semaphore for driver down auto
xSemaphoreHandle pumSemaphore; //Semaphore for passenger up manual
xSemaphoreHandle pdmSemaphore; //Semaphore for passenger down manual
xSemaphoreHandle puaSemaphore; //Semaphore for passenger up auto
xSemaphoreHandle pdaSemaphore; //Semaphore for passenger doown auto

xQueueHandle xQueueLock;
xQueueHandle xQueueJam;


xSemaphoreHandle xMutex;


/*xSemaphoreHandle jamSemaphore;
xSemaphoreHandle lockSemaphore;  

void PortF_ISR(void *pvParameters);
void PortF_ISR_Handler(void);*/

void jamWindow(void *pvParameters);
void lockWindow(void *pvParameters); 

// Prototype for each task
void driver_up_manual(void *pvParameters); 
void driver_down_manual(void *pvParameters);
void driver_up_auto(void *pvParameters);
void driver_down_auto(void *pvParameters);
void psng_up_manual(void *pvParameters);
void psng_down_manual(void *pvParameters);
void psng_up_auto(void *pvParameters);
void psng_down_auto(void *pvParameters); 

void check_pushed_btn(void *pvParameters);
TaskHandle_t check_btn_handle; // check_Pushed_btn handler

int lockChk =0;

void Delay(void);

int main(void)
 {
	 
	DIO_Init();
	GPIO_PORTB_DATA_R = 0x00; 
	 
	//Semaphores declared as binary semaphores
	ctrlSemaphore = xSemaphoreCreateBinary();
	dumSemaphore = xSemaphoreCreateBinary();
	ddmSemaphore = xSemaphoreCreateBinary();
	duaSemaphore = xSemaphoreCreateBinary();
	ddaSemaphore = xSemaphoreCreateBinary();
	pumSemaphore = xSemaphoreCreateBinary();
	pdmSemaphore = xSemaphoreCreateBinary();
	puaSemaphore = xSemaphoreCreateBinary();
	pdaSemaphore = xSemaphoreCreateBinary();
	
	/*jamSemaphore = xSemaphoreCreateBinary();
	lockSemaphore = xSemaphoreCreateBinary(); */

	xMutex = xSemaphoreCreateMutex();	 
	 
	xQueueJam   = xQueueCreate(1, sizeof(int));
	xQueueLock 	= xQueueCreate(1, sizeof(int));
	

	
	
	// using xTaskCreate to create the tasks for the scheduler 
	xTaskCreate(lockWindow,"Lock window",90,NULL,4,NULL);
	xTaskCreate(jamWindow,"Jam window",90,NULL,4,NULL);
	xTaskCreate(check_pushed_btn,"Check push Pressed",90,NULL,1,&	check_btn_handle);
	xTaskCreate(driver_up_manual,"up driver manual Pressed",90,NULL,2,NULL);
	xTaskCreate(driver_down_manual,"down driver manual Pressed",90,NULL,2,NULL);
	xTaskCreate(driver_up_auto,"up driver auto Pressed",90,NULL,2,NULL);
	xTaskCreate(driver_down_auto,"down driver auto Pressed",90,NULL,2,NULL);
	xTaskCreate(psng_up_manual,"up passenger manual Pressed",90,NULL,2,NULL);
	xTaskCreate(psng_down_manual,"down passenger manual Pressed",90,NULL,2,NULL);
	xTaskCreate(psng_up_auto,"up passeenger auto Pressed",90,NULL,2,NULL);
	xTaskCreate(psng_down_auto,"down passenger auto Pressed",90,NULL,2,NULL); 
	
	
	GPIO_PORTF_DATA_R = 0x00;   // Clear the data register of PORTF
	GPIO_PORTB_DATA_R = 0x00;   // Clear the data register of PORTB

	// Run the scheduler in FreeRTOS
	vTaskStartScheduler();
	
	// This loop should never be reached
	for(;;){}
		
}

// Delay with the delay value defined earlier
void Delay(void)
{
	volatile unsigned long i;
	for(i=0;i<DELAY_VALUE;i++);
}

// Function used to check for which button is pressed
void check_pushed_btn(void *pvParameters){
	for(;;){
		
		if((GPIOB->DATA & (1<<3))== 0) {  // Button connected to PB3 pressed 
			xSemaphoreGive(ctrlSemaphore);
			xSemaphoreGive(dumSemaphore);
		}
		if((GPIOB->DATA & (1<<2))== 0) { // Button connected to PB2 pressed
			xSemaphoreGive(ctrlSemaphore);
			xSemaphoreGive(ddmSemaphore);
			
		}
		if((GPIOA->DATA & (1<<6))== 0) { // Button connected to PA6pressed
			xSemaphoreGive(duaSemaphore);
			
		}
		if((GPIOA->DATA & (1<<5))== 0) { // Button connected to PA5 pressed
			xSemaphoreGive(ddaSemaphore);
			
		}
		if((GPIOA->DATA & (1<<2))== 0) { // Button connected to PA2 pressed
			xSemaphoreGive(pumSemaphore);
		}
		if((GPIOA->DATA & (1<<3))== 0) { // Button connected to PA3 pressed
			xSemaphoreGive(pdmSemaphore);
		}
		if((GPIOB->DATA & (1<<5))== 0) { // Button connected to PB5 pressed
			xSemaphoreGive(puaSemaphore);
		}
		if((GPIOB->DATA & (1<<4))== 0) { // Button connected to PB4 pressed
			xSemaphoreGive(pdaSemaphore);
		}
		else{
			// Motor is connected to PB0 and PB1, we clear these two pins
			clear_bit(GPIO_PORTB_DATA_R, 0);
			clear_bit(GPIO_PORTB_DATA_R, 1);
		}
		taskYIELD();
	}
}

// Manual window up task for driver
void driver_up_manual(void *pvParameters){
	for(;;)
	{
		// Takes semaphore
		xSemaphoreTake(dumSemaphore, 0);
		xSemaphoreTake(dumSemaphore, portMAX_DELAY);
		xSemaphoreTake(ctrlSemaphore, portMAX_DELAY);
		if((GPIO_PORTB_DATA_R &(1<<3)) ==0) // Keeps checking if the button is pressed as this is manual
		{
			GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins			
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|GREEN; // Make green LED on
			set_bit(GPIO_PORTB_DATA_R, 1); // PB1 is set by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		}
		GPIO_PORTF_DATA_R &= ~0x0E;	
		xSemaphoreGive(dumSemaphore); // Give semaphore
		xSemaphoreTake(dumSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		xSemaphoreGive(ctrlSemaphore); // Give semaphore
		xSemaphoreTake(ctrlSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Manual window down task for driver
void driver_down_manual(void *pvParameters){
	for(;;)
	{
		
		// Takes semaphore
		xSemaphoreTake(ddmSemaphore, 0);
		xSemaphoreTake(ddmSemaphore, portMAX_DELAY);
		xSemaphoreTake(ctrlSemaphore, portMAX_DELAY);
		if((GPIOB->DATA & (1<<2))== 0) // Keeps checking if the button is pressed
		{
			GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins			
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|GREEN; // Make green LED on
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|BLUE;	 // Make blue LED on
			
			set_bit(GPIO_PORTB_DATA_R, 0); // Set PB0 by 1
			clear_bit(GPIO_PORTB_DATA_R, 1); // Clear PB1
		}
		GPIO_PORTF_DATA_R &= ~0x0E;	
		xSemaphoreGive(ddmSemaphore); // Give semaphore
		xSemaphoreTake(ddmSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore 
		xSemaphoreGive(ctrlSemaphore); // Give semaphore
		xSemaphoreTake(ctrlSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Auto window up task for driver
void driver_up_auto(void *pvParameters){
	for(;;)
	{

		xSemaphoreTake(duaSemaphore, portMAX_DELAY);
		
		
		GPIO_PORTF_DATA_R &= ~0x0E;				
		GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x04; // Make blue LED on
		set_bit(GPIO_PORTB_DATA_R, 1); // PB1 is set by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		for (int i = 0; i<DELAY_VALUE; i++) // For loop with a value polling for, makes the auto rotate for certain time then stops
		{
			if((GPIOF -> DATA & (1<<0)) ==0)
			{
				
				xQueueSendToBack(xQueueLock,&queueV,portMAX_DELAY);
			}
			if((GPIOF -> DATA & (1<<4)) ==0)
			{
				
				xQueueSendToBack(xQueueJam,&queueV,portMAX_DELAY);
			}
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x04; // Make blue LED on
			set_bit(GPIO_PORTB_DATA_R, 1); // PB1 is set by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		
			}
		GPIO_PORTF_DATA_R =0x00;
		
		// Give semaphoe
		xSemaphoreGive(duaSemaphore);
		xSemaphoreTake(duaSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Auto window down task for driver
void driver_down_auto(void *pvParameters){
for(;;)
	{

		xSemaphoreTake(ddaSemaphore, portMAX_DELAY);
		
		
		GPIO_PORTF_DATA_R &= ~0x0E;				
		GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x02; // Make blue LED on
		set_bit(GPIO_PORTB_DATA_R, 1); // PB1 is set by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		for (int i = 0; i<DELAY_VALUE; i++) // For loop with a value polling for, makes the auto rotate for certain time then stops
		{
			if((GPIOF -> DATA & (1<<0)) ==0)
			{
				
				xQueueSendToBack(xQueueLock,&queueV,portMAX_DELAY);
			}
			if((GPIOF -> DATA & (1<<4)) ==0)
			{
				
				xQueueSendToBack(xQueueJam,&queueV,portMAX_DELAY);
			}
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x02; // Make blue LED on
			set_bit(GPIO_PORTB_DATA_R, 1); // PB1 is set by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		
			}
		GPIO_PORTF_DATA_R =0x00;
		
		// Give semaphoe
		xSemaphoreGive(ddaSemaphore);
		xSemaphoreTake(ddaSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Manual window up task for passenger
void psng_up_manual(void *pvParameters){
	for(;;)
	{
		
		// Take semaphore
		xSemaphoreTake(pumSemaphore, 0);
		xSemaphoreTake(pumSemaphore, portMAX_DELAY);
		if((GPIOA->DATA & (1<<2))== 0) // Keeps checking if the button is pressed as this is manual
		{
			GPIO_PORTF_DATA_R &= ~0x0E;	// Clear LEDs pins	
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|BLUE; // Make blue LED on
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|RED;	// Make red LED on		
			set_bit(GPIO_PORTB_DATA_R, 1); // Set PB1 by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		}
		GPIO_PORTF_DATA_R &= ~0x0E;	// Clear LEDs pins
		
		// Give Semaphore
		xSemaphoreGive(pumSemaphore);
		xSemaphoreTake(pumSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Manual window down task for passenger
void psng_down_manual(void *pvParameters){
	for(;;)
	{
		
		// Take semaphore
		xSemaphoreTake(pdmSemaphore, 0);
		xSemaphoreTake(pdmSemaphore, portMAX_DELAY);
		if((GPIOA->DATA & (1<<3))== 0) // Keeps checking if the button is pressed as this is manual
		{
			GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins	
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|GREEN; // Make green LED on
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|BLUE;	 // Make blue LED on
			set_bit(GPIO_PORTB_DATA_R, 0); // Set PB0 by 1
			clear_bit(GPIO_PORTB_DATA_R, 1); // Clear PB1
		}
		GPIO_PORTF_DATA_R &= ~0x0E;	
		
		// Give semaphore
		xSemaphoreGive(pdmSemaphore);
		xSemaphoreTake(pdmSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Auto window up task for passenger
void psng_up_auto(void *pvParameters){
	for(;;)
	{
		
		// Take semaphore
		xSemaphoreTake(puaSemaphore, portMAX_DELAY);
    
		GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins	
		GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|RED; // Make red LED on
		set_bit(GPIO_PORTB_DATA_R, 1); // set PB1 by 1
		clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		for (int i = 0; i<DELAY_VALUE; i++) // For loop with a value polling for, makes the auto rotate for certain time then stops
		{
			if((GPIOF -> DATA & (1<<0)) ==0)
			{
				
				xQueueSendToBack(xQueueLock,&queueV,portMAX_DELAY);
			}
			if((GPIOF -> DATA & (1<<4)) ==0)
			{
				
				xQueueSendToBack(xQueueJam,&queueV,portMAX_DELAY);
			}
			GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins	
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|RED; // Make red LED on
			set_bit(GPIO_PORTB_DATA_R, 1); // set PB1 by 1
			clear_bit(GPIO_PORTB_DATA_R, 0); // Clear PB0
		}
		GPIO_PORTF_DATA_R =0x00;
		
		// Give Semaphore
		xSemaphoreGive(puaSemaphore);
		xSemaphoreTake(puaSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}

// Auto window down task for passenger
void psng_down_auto(void *pvParameters){
	for(;;)
	{
		
		// Give Semaphore
		xSemaphoreTake(pdaSemaphore, portMAX_DELAY);
    
		GPIO_PORTF_DATA_R &= ~0x0E;	// Clear the LEDs pins 			
		GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x02; // Make red LED on
		GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x04; // Make blue LED on
		set_bit(GPIO_PORTB_DATA_R, 0); // Set PB0 by 1
		clear_bit(GPIO_PORTB_DATA_R, 1); // Clear PB1
		for (int i = 0; i<DELAY_VALUE; i++); // For loop with a value polling for, makes the auto rotate for certain time then stops
		{
			if((GPIOF -> DATA & (1<<0)) ==0)
			{
				
				xQueueSendToBack(xQueueLock,&queueV,portMAX_DELAY);
			}
			if((GPIOF -> DATA & (1<<4)) ==0)
			{
				
				xQueueSendToBack(xQueueJam,&queueV,portMAX_DELAY);
			}
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x02; // Make red LED on
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x04; // Make blue LED on
			set_bit(GPIO_PORTB_DATA_R, 0); // Set PB0 by 1
			clear_bit(GPIO_PORTB_DATA_R, 1); // Clear PB1
		}
		GPIO_PORTF_DATA_R =0x00;
		xSemaphoreGive(pdaSemaphore);
		xSemaphoreTake(pdaSemaphore, 0); // Take semaphore again so that when the we loop again it becomes blocked waiting for semaphore
		taskYIELD();
	}
}



void jamWindow(void *pvParameters) // Jams the motor until stop pressing on button
{
	xQueueReceive(xQueueJam,&queueV,portMAX_DELAY); // Recieve from Queue 
	for (;;)
	{
		while (((GPIOF -> DATA & (1<<4))) == 0) 
		{
		GPIO_PORTF_DATA_R &= ~0x0E;
		// Clear both PB0 and PB1 for motor
		clear_bit(GPIO_PORTB_DATA_R, 1);
		clear_bit(GPIO_PORTB_DATA_R, 0);
		}
			xQueueReceive(xQueueJam,&queueV,portMAX_DELAY);
			taskYIELD();
	}
}


void lockWindow(void *pvParameters)
{
	
	xQueueReceive(xQueueLock,&queueV,portMAX_DELAY);
	for (;;)
	{
		
		GPIO_PORTF_DATA_R &= ~0x0E;
		// Clear both PB0 and PB1 for motor
		clear_bit(GPIO_PORTB_DATA_R, 1);
		clear_bit(GPIO_PORTB_DATA_R, 0);
		if (((GPIOF -> DATA & (1<<0))) == 0)
		{
			xQueueReceive(xQueueLock,&queueV,portMAX_DELAY);
			taskYIELD();
		}
	}
}


/*void PortF_ISR(void *pvParameters){
	xSemaphoreGive(xMutex);
	PortF_ISR_Handler();
}

void PortF_ISR_Handler(void){
	for( ; ;)
	{
		//uint32_t pin_statusPF0 = get_bit(GPIO_PORTF_DATA_R,0);
		//uint32_t pin_statusPF4 = get_bit(GPIO_PORTF_DATA_R,4);
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if((GPIO_PORTF_DATA_R & (1<<0)) == 0) // Switch 1 is pressed
    {
        // Switch 1 was pressed (jamming Detected)
				//pass semaphore to appropriate task to unblock it
				xSemaphoreGiveFromISR(jamSemaphore, &xHigherPriorityTaskWoken);
		}
		if((GPIO_PORTF_DATA_R & (1<<4)) == 0) // Switch 2 is pressed
    {
        // Switch 2 was pressed (window Lock Engaged)
				//pass semaphore to appropriate task to unblock it
				xSemaphoreGiveFromISR(lockSemaphore, &xHigherPriorityTaskWoken);
    }
	}
}

void jamWindow(void){
	for(;;){
		xSemaphoreTake(jamSemaphore, portMAX_DELAY);
		if((GPIO_PORTF_DATA_R & (1<<0)) == 0)
		{
			GPIO_PORTF_DATA_R &= ~0x0E;				
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x08; 
			clear_bit(GPIO_PORTB_DATA_R, 0);
			clear_bit(GPIO_PORTB_DATA_R, 1);
		}
		xSemaphoreGive(jamSemaphore);
		xSemaphoreTake(jamSemaphore,0);
	}
}

void lockWindow(void){
	for(;;){
		xSemaphoreTake(lockSemaphore, portMAX_DELAY);
		for(int i=0;i<50;i++);
		while((GPIO_PORTF_DATA_R & (1<<4)) == 1)
		{
			GPIO_PORTF_DATA_R &= ~0x0E;				
			GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R|0x04; 
			clear_bit(GPIO_PORTB_DATA_R, 0);
			clear_bit(GPIO_PORTB_DATA_R, 1);
		}
		xSemaphoreGive(lockSemaphore);
		xSemaphoreTake(lockSemaphore, 0);
	}
} */

