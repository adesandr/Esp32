/********************************************************************************
 * I2C Scan
 * 
 * Used to determine the I2C devices's addresses connected on the I2C bus.
 *
 * Tested with Wemos Lolin32 board
 *
 * Alain Désandré, Paris France, V1.0 - 29/04/2018
 *
 * https://github.com/adesandr
 *******************************************************************************/

/*--- 			Includes													---*/
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"

/*--- Some defines															---*/
#define SDA_PIN 21						// I2c SDA pin on Lolin32
#define SCL_PIN 22						// I2c SCL pin on Lolin32
#define BLINK_GPIO CONFIG_BLINK_GPIO	// Builtin Led on Lolin32, pin 5

/*******************************************************************************
 * void task_i2cScanner (void *)							
 * 
 * Description : main task. Scan the i2c port
 * 
 * input(s) : none
 * 
 * output(s) : none
 * 
 * return : none
 * ****************************************************************************/
void task_i2cscanner(void *ignore) 
{
	/*-- Variable definition											  ---*/
	static char tagi[] = "i2cscanner";			// For Log
	i2c_config_t conf;							// For I2C bus
	int i;										// loop control
	esp_err_t espRc;							// catch esp errors

	ESP_LOGD(tagi, ">> i2cScanner");

	/*--- I2C bus configuration											  ---*/
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	i2c_param_config(I2C_NUM_0, &conf);

	/*--- I2c driver initialization										  ---*/
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	/*--- start main task loop, to scan periodically I2c devices  		  ---*/	
	while(1)
	{	
		printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
		printf("00:         ");
		for (i=3; i< 0x78; i++) {
			i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
			i2c_master_stop(cmd);

			espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			if (i%16 == 0) {
				printf("\n%.2x:", i);
			}
			if (espRc == 0) {
				printf(" %.2x", i);
			} else {
				printf(" --");
			}
		
			//ESP_LOGD(tagi, "i=%d, rc=%d (0x%x)", i, espRc, espRc);
		
			i2c_cmd_link_delete(cmd);
		
		} /* End i */
	
	printf("\n");

	vTaskDelay(pdMS_TO_TICKS(5000));			// print each 5s 

	} /* End while */

	vTaskDelete(NULL);							// Normally never used

} /* End task_i2cscanner() */

/*******************************************************************************
 * void blink_task (void *)							
 * 
 * Description : Blinky task, used to periodically blink the builtin led. life
 * 				 signal. Same feature xcould done also using esp_timer.
 * 
 * input(s) : none
 * 
 * output(s) : none
 * 
 * return : none
 * ****************************************************************************/
void blink_task(void *pvParameter)
{
	static char tagb[] = "blinky";

    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    ESP_LOGD(tagb, ">> blink_task\n");
    gpio_pad_select_gpio(BLINK_GPIO);
	
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    
	while(1) 
	{
        /* Blink off (output low) */
        gpio_set_level(BLINK_GPIO, 0);
		
        vTaskDelay(pdMS_TO_TICKS(1500));
		
        /* Blink on (output high) */
        gpio_set_level(BLINK_GPIO, 1);
		
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
	
	vTaskDelete(NULL);

} /*--- End blink_task() ---*/

/*******************************************************************************
 * void app_main (void *)							
 * 
 * Description : Create the two above task.
 * 
 * input(s) : none
 * 
 * output(s) : none
 * 
 * return : none
 * ****************************************************************************/
void app_main()
{

/*--- task_i2cScanner init. priority 5									   ---*/
xTaskCreate(&task_i2cscanner, "task_i2cscanner", 2048, NULL, 5, NULL);

/*--- blink_task init. priority 5										   ---*/
xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, NULL);

} /*--- End main() ---*/