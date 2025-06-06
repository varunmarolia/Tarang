/**
 * @file vayu.c
 * @author Varun Marolia
 * @brief This is the main app file for this project. The file must implement
 *        the app_init and app_poll functions.
 * 
 * @copyright Copyright (c) 2025 Varun Marolia
 *   MIT License
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:**
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 * 
 */

#include "board.h"
#include "vayu.h"
#include <stdio.h>
#include "sht4x.h"
#include "guart.h"
#include "ntc.h"
#include "fan-blower.h"
#include "board-common.h"
#include "clock.h"

#define FAN_OUTLET_RPM 3500
#define FAN_INLET_RPM 3500
#define TEMPERATURE_HYSTERESIS 1000       /* 1 degree Celsius hysteresis for temperature */
#define TEMPERATURE_DEFROSTING 5000       /* 5 degree Celsius for defrosting */
#define TEMPERATURE_HRV_MODE_MIN 8000     /* 8 degree Celsius minimum temperature for HRV mode */
#define TEMPERATURE_HRV_MODE_MAX 16000    /* 16 degree Celsius maximum temperature for HRV mode */
#define TEMPERATURE_INLET_MODE_MIN 17000  /* 18 degree Celsius minimum temperature for Inlet mode */
#define TEMPERATURE_INLET_MODE_MAX 22000  /* 22 degree Celsius maximum temperature for Inlet mode */
/*---------------------------------------------------------------------------*/
sht4x_t sht4x_sensor = {
  .last_rh_ppm = 0,
  .last_temp_mk = 0,
  .serial_number = 0,
  .sht4x_dev = &SHT4X_DEV
};
/*---------------------------------------------------------------------------*/
ntc_thermistor_t ntc_dev[NTC_TOTAL] = {
  { /* board NTC details */
    .adc_dev = &BOARD_NTC_ADC_DEV,
    .beta_value_25 = 4250,
    .known_resistance_ohm = 200000,
    .max_negative_temp_C = -20,
    .max_positive_temp_C = 125,
    .ntc_config = NTC_PULLED_DOWN_CONFIG,
    .R0_ohm = 100000,
    .RT_chart = NULL,
    .supply_mV = 3000,
    .T0_C = 25,
    .temp_step = 0
  },
  { /* HRV NTC details */
    .adc_dev = &HA_NTC_ADC_DEV,
    .beta_value_25 = 3950,
    .known_resistance_ohm = 200000,
    .max_negative_temp_C = -18,
    .max_positive_temp_C = 125,
    .ntc_config = NTC_PULLED_DOWN_CONFIG,
    .R0_ohm = 100000,
    .RT_chart = NULL,
    .supply_mV = 3300,
    .T0_C = 25,
    .temp_step = 0
  }
};
/*---------------------------------------------------------------------------*/
fan_blower_t fan = {
  .pulses_per_rev = 2,
  .bidir = FAN_BLOWER_DIR_BIDIRECTIONAL_OVER_PWM,
  .max_rpm = 5500,
  .min_rpm = 1000,
  .min_pwm = 10,
  .fan_blower_dir_handler = NULL,
  .current_rpm = 0,
  .current_dir = 0,
  .pwm_dev = &FAN_PWM_DEV
};
/*---------------------------------------------------------------------------*/
guart_t uart_debug = {
  .rx_buff_head = 0,
  .rx_buff_tail = 0,
  .rx_buff = {0},
  .new_line_buff_index = GUART_RX_BUFFER_SIZE,
  .guart_dev = &UART_GENERIC_DEV
};
/*---------------------------------------------------------------------------*/
static void
read_sht4x(void)
{
  int16_t temperature_mC;
  uint16_t rh_percentage_100x;
  uint8_t bus_status;
  bus_status = shrt4x_get_temp_humidity(&sht4x_sensor, &temperature_mC, &rh_percentage_100x);
  if(bus_status == BUS_OK) {
    printf("App_poll: sht4x temperature:%03d.%02u 'C humidity:%02u.%02u %%RH\n", 
           (int16_t)(temperature_mC / 1000), 
           (int16_t)(temperature_mC % 1000) / 10, 
           rh_percentage_100x / 100, 
           rh_percentage_100x % 100);
  } else {
    printf("App_poll: Failed to read measurement!!!\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
read_ntc(uint8_t ntc_type) 
{
  int32_t temp_mC = 0;
  if(ntc_type >= NTC_TOTAL) { 
    return;
  }
  temp_mC = ntc_read_temp_mc_using_beta(&ntc_dev[ntc_type]);
  if(temp_mC == NTC_ERROR) {
    printf("App_poll: NTC error\n");
  } else {
    switch(ntc_type) {
      case NTC_HRV:
        printf("App_poll: NTC_HRV ");
      break;
      case NTC_BOARD:
        printf("App_poll: NTC_BOARD ");
      break;
      default:
      break;
    }
    printf("temperature:%03d.%02u 'C\n", (int16_t)(temp_mC / 1000), (int16_t)(temp_mC % 1000) / 10);
  }
}
/*---------------------------------------------------------------------------*/
volatile hrv_mode_t mode_hrv = HRV_MODE_OFF; /* default mode is OFF */
static void 
mode_button_handler(gpio_interrupt_t *button) {
  /* This is the mode button handler. The function will be called when the mode button is pressed */
  (void)button;  /* avoid unused parameter warning */
  mode_hrv++;
  if(mode_hrv > HRV_MODE_AUTO) {
    mode_hrv = HRV_MODE_OFF;
  }
}
/*---------------------------------------------------------------------------*/
static void
led_error_blink(void)
{
  gpio_set_pin_logic(LED_MODE_GREEN_PORT, LED_MODE_GREEN_PIN, GPIO_PIN_LOGIC_LOW); /* turn on the mode LED */
  gpio_set_pin_logic(LED_MODE_YELLOW_PORT, LED_MODE_YELLOW_PIN, GPIO_PIN_LOGIC_LOW); /* turn on the mode LED */
  clock_wait_ms(500); /* wait for 1 second */
  gpio_set_pin_logic(LED_MODE_GREEN_PORT, LED_MODE_GREEN_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off the mode LED */
  gpio_set_pin_logic(LED_MODE_YELLOW_PORT, LED_MODE_YELLOW_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off the mode LED */
  clock_wait_ms(500); /* wait for 1 second */
}
/*---------------------------------------------------------------------------*/
ttimer_t poll_timer;
uint8_t 
app_init(void) {
  guart_init(&uart_debug);              /* Initialize generic UART */
  guart_set_debug_stdo(&uart_debug);    /* Set the debug UART */
  sht4x_init(&sht4x_sensor);            /* Initialize the SHT4X sensor */
  fan_blower_init(&fan);                /* This will enable the FAN, initialize the PWM 
                                          arch and set 50% duty cycle to keep the FAN Off */
  pwm_dev_init(&HA_HEATER_DEV);         /* Initialize the heater pwm. keep the duty cycle 0% i.e. OFF */
  MODE_BUTTON.callback = mode_button_handler;  /* Set callback function for mode button */
  gpio_interrupt(&MODE_BUTTON, true);  /* Enable GPIO interrupt for mode button */
  timer_set(&poll_timer,  0);
  mode_hrv = HRV_MODE_OFF;              /* default mode is OFF */
  return 0;
}
/*---------------------------------------------------------------------------*/
void
app_poll(void) {
static hrv_mode_t mode_hrv_previous = HRV_MODE_OFF;
static uint8_t system_err_flag = 0;
static ttimer_t fan_dir_toggle_timer = {.start_time = 0, 
                                        .interval = 0
                                        };
const uint32_t fan_cycle_time_ms = 70 * 1000; /* 1 minute 10 second */
static ttimer_t HA_heater_setting_changed_timer = {.start_time = 0, 
                                        .interval = 0
                                        };
const uint32_t HA_heater_setting_changed_time_ms = 5 * 1000;
int32_t HA_temp_mC = 0;     /* Heat Accumulator temperature in milli Celsius */
serial_bus_status_t bus_status = BUS_OK;

  if(!system_err_flag) {
    if(mode_hrv != mode_hrv_previous) {
      switch(mode_hrv) {
        case HRV_MODE_OFF:
          fan_blower_set_rpm(&fan, 0, 0); /* turn off the fan */
          pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 0);  /* turn off heater */
          gpio_set_pin_logic(LED_MODE_GREEN_PORT, LED_MODE_GREEN_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off the mode LED */
          gpio_set_pin_logic(LED_MODE_YELLOW_PORT, LED_MODE_YELLOW_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off the mode LED */
          system_err_flag = 0; /* reset the error flag */
          printf("App_poll: HRV mode OFF\n");
          break;
        case HRV_MODE_INLET:
          fan_blower_set_rpm(&fan, FAN_OUTLET_RPM, FAN_DIR_REVERSE); /* set the fan to FAN_OUTLET_RPM RPM in reverse/inlet direction */
          pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 0);  /* turn off heater */
          gpio_set_pin_logic(LED_MODE_GREEN_PORT, LED_MODE_GREEN_PIN, GPIO_PIN_LOGIC_LOW); /* turn ON GREEN LED */
          gpio_set_pin_logic(LED_MODE_YELLOW_PORT, LED_MODE_YELLOW_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off YELLOW LED */
          printf("App_poll: HRV mode INLET\n");
          break;
        case HRV_MODE_AUTO:
          fan_blower_set_rpm(&fan, FAN_INLET_RPM, FAN_DIR_FORWARD); /* set the fan to FAN_INLET_RPM RPM in forward/exhaust direction */
          timer_set(&fan_dir_toggle_timer, fan_cycle_time_ms); /* set the timer for 1 minute 10 seconds */
          gpio_set_pin_logic(LED_MODE_GREEN_PORT, LED_MODE_GREEN_PIN, GPIO_PIN_LOGIC_HIGH); /* turn off GREEN LED */
          gpio_set_pin_logic(LED_MODE_YELLOW_PORT, LED_MODE_YELLOW_PIN, GPIO_PIN_LOGIC_LOW); /* turn on YELLOW LED */
          printf("App_poll: HRV mode AUTO\n");
          break;
        default:
          break;
      }
      mode_hrv_previous = mode_hrv; /* update the previous mode */
    }
    if(mode_hrv == HRV_MODE_AUTO) {
      /* HRV algorithm */
      if(timer_timedout(&HA_heater_setting_changed_timer)) {
        HA_temp_mC = ntc_read_temp_mc_using_beta(&ntc_dev[NTC_HRV]);
        if(HA_temp_mC != NTC_ERROR && bus_status == BUS_OK) {
          
          /* Defrosting or very low temperature */
          if(HA_temp_mC < TEMPERATURE_DEFROSTING) {
            /* This could mean frosting so we need to defrost by using heater */
            if(HA_HEATER_DEV.duty_cycle_100x != 10000) {
              pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 10000);
              timer_set(&HA_heater_setting_changed_timer, HA_heater_setting_changed_time_ms);
              printf("App_poll: Defrosting. Heater ON @ 100%% HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            }
            if(HA_temp_mC > 1000 && (fan.current_rpm != 1500 || fan.current_dir != FAN_DIR_FORWARD)) {
              /*  Start heating the HA (heat accumulator) with warm room air at slow speed. 
              *  The fan speed should be low as the HA could be blocked with ice.
              */
              if(timer_timedout(&fan_dir_toggle_timer)) {
                fan_blower_set_rpm(&fan, 1500, FAN_DIR_FORWARD); /* set the fan to 1500 RPM in forward/exhaust direction */
                timer_set(&fan_dir_toggle_timer, fan_cycle_time_ms); /* set the timer for 1 minute 10 seconds */
              }
              printf("App_poll: Fan ON exhaust mode 1500 RPM HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            } else if((HA_temp_mC <= 0000 && fan.current_rpm != 0)
                      && timer_timedout(&fan_dir_toggle_timer)) {
              fan_blower_set_rpm(&fan, 0, 0); /* turn off the fan */
              timer_set(&fan_dir_toggle_timer, fan_cycle_time_ms);
              printf("App_poll: Defrosting. Fan OFF HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            }
          }

          /* HRV mode range */ 
          if(HA_temp_mC >= TEMPERATURE_HRV_MODE_MIN && HA_temp_mC <= TEMPERATURE_HRV_MODE_MAX) {
            /* Keep heater ON ! Experiment shows heat recovery does not seem to be very effective at this stage and requires extra heating */
            if(HA_HEATER_DEV.duty_cycle_100x != 10000) {
              pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 10000); /* 100% duty cycle */
              timer_set(&HA_heater_setting_changed_timer, HA_heater_setting_changed_time_ms);
              printf("App_poll: HA heating. Heater ON @ 100%% HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            }
            if(timer_timedout(&fan_dir_toggle_timer)) {
              if(fan.current_dir == FAN_DIR_FORWARD) {
                fan_blower_set_rpm(&fan, FAN_INLET_RPM, FAN_DIR_REVERSE); /* set the fan to 4500 RPM in reverse/inlet direction */
                printf("App_poll: timer timeout, Fan ON inlet mode %u RPM HA temp:%03d.%02u 'C\n", FAN_INLET_RPM, (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
              } else {
                fan_blower_set_rpm(&fan, FAN_OUTLET_RPM, FAN_DIR_FORWARD); /* set the fan to 4500 RPM in forward/exhaust direction */
                printf("App_poll: timer timeout, Fan ON exhaust mode %u RPM HA temp:%03d.%02u 'C\n", FAN_OUTLET_RPM, (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
              }
              timer_set(&fan_dir_toggle_timer, fan_cycle_time_ms); /* set the timer for 1 minute 10 seconds */
            }
          }

          /* Inlet only mode */ 
          if(HA_temp_mC > TEMPERATURE_INLET_MODE_MIN) {
            if(HA_temp_mC < TEMPERATURE_INLET_MODE_MAX && HA_HEATER_DEV.duty_cycle_100x != 5000) {
              pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 5000);   /* 50% duty cycle */
              timer_set(&HA_heater_setting_changed_timer, HA_heater_setting_changed_time_ms);
              printf("App_poll: HA heating. Heater @ 50%% HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            } else if(HA_temp_mC > (TEMPERATURE_INLET_MODE_MAX + TEMPERATURE_HYSTERESIS) 
              && HA_HEATER_DEV.duty_cycle_100x != 0) {
              pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 0);      /* turn off heater */
              timer_set(&HA_heater_setting_changed_timer, HA_heater_setting_changed_time_ms);
              printf("App_poll: HA heating. Heater OFF HA temp:%03d.%02u 'C\n", (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            }
            if((fan.current_rpm != FAN_INLET_RPM || fan.current_dir != FAN_DIR_REVERSE) 
                && timer_timedout(&fan_dir_toggle_timer)) {
              /* set the fan to FAN_OUTLET_RPM RPM in reverse/inlet direction */
              fan_blower_set_rpm(&fan, FAN_INLET_RPM, FAN_DIR_REVERSE);
              timer_set(&fan_dir_toggle_timer, fan_cycle_time_ms); /* set the timer for 1 minute 10 seconds */
              printf("App_poll: Fan ON inlet mode %u RPM HA temp:%03d.%02u 'C\n", FAN_OUTLET_RPM, (int16_t)(HA_temp_mC / 1000), (int16_t)(HA_temp_mC % 1000) / 10);
            }
          }
          
        } else {
          system_err_flag = 1;
          pwm_dev_set_duty_cycle(&HA_HEATER_DEV, 0);  /* turn off heater */
          timer_set(&HA_heater_setting_changed_timer, HA_heater_setting_changed_time_ms);
          fan_blower_set_rpm(&fan, 0, 0); /* fan OFF */
          printf("App_poll: Error in reading NTC or SHT4X\n");
        }
      } /* if(timer_timedout(&HA_heater_setting_changed_time_ms)) */
    } /* if(mode_hrv == HRV_MODE_AUTO) */
  } else {
    led_error_blink(); /* blink the error LED */
  }
  if(timer_timedout(&poll_timer)) {
    printf("\n");
    read_sht4x();
    read_ntc(NTC_HRV);
    read_ntc(NTC_BOARD);
    led_sys_blink(LED_SYS_YELLOW_PORT, LED_SYS_YELLOW_PIN, 1, 100);
    timer_set(&poll_timer, 10000); /* set the timer for 10 seconds */
  }
}
/*---------------------------------------------------------------------------*/
