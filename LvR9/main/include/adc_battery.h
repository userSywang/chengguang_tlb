#ifndef __ADC_BATTERY_H__
#define __ADC_BATTERY_H__

/**
 * @brief    get battery charge status.
 * @return 
 *     - 0 : no charge 
 *     - 1 : charge 
 */
int get_battery_charge_status(void );

/**
 * @brief    get battery voltage and charge status.
 * The true sampling value of the battery ADC is cycled 14 times, the minimum 2 and the maximum 2 are removed, and the ADC voltage fluctuation is calculated by optimizing the average value
 * @param	
 		- voltage : battery voltage
 		- charge_status : battery charge status .
 * @return
 *     - ESP_OK :on success
 *     - ESP_FAIL : fail 
 */
int get_battery_voltage_charge( int* voltage , int* charge_status );


/**
 * @brief    get battery real voltage and charge status.
			 The real sampling value of the battery ADC is sampled 14 times, the minimum 2 and the maximum 2 are removed, and the average value is taken
 *
 * @param	
 		- voltage : battery voltage
 		- charge_status : battery charge status .
 * @return
 *     - ESP_OK :on success
 *     - ESP_FAIL : fail 
 */
int get_battery_real_voltage_charge( int* voltage , int* charge_status );


#endif  // __ADC_BATTERY_H__
