#ifndef _GPIO_H_
#define _GPIO_H_
#include <stdint.h>
#include <stdbool.h>

typedef enum gpio_mode {
  GPIO_MODE_DISABLED,                                 /** input disabled */
  GPIO_MODE_DISABLED_INTERNAL_PULL_UP,                /** input disabled with pull-up */
  GPIO_MODE_INPUT,                                    /** input enabled */
  GPIO_MODE_INPUT_EXTERNAL_PULL_UP,                   /** input enabled with external pull-up */
  GPIO_MODE_INPUT_EXTERNAL_PULL_DOWN,                 /** input enabled with external pull-down */
  GPIO_MODE_INPUT_INTERNAL_PULL_UP,                   /** input enabled with pull-up */
  GPIO_MODE_INPUT_INTERNAL_PULL_DOWN,                 /** input enabled with pull-down */
  GPIO_MODE_OUTPUT_PUSH_PULL_SET,                     /** output push-pull, output set */
  GPIO_MODE_OUTPUT_PUSH_PULL_CLEAR,                   /** output push-pull, output clear */
  GPIO_MODE_OUTPUT_OPEN_SOURCE,                       /** output open-source AKA wiredOR*/
  GPIO_MODE_OUTPUT_OPEN_SOURCE_INTERNAL_PULL_DOWN,    /** output open-source with pull-down AKA WiredORPull-Down */
  GPIO_MODE_OUTPUT_OPEN_DRAIN,                        /** output open-drain aka WiredAND */
  GPIO_MODE_OUTPUT_OPEN_DRAIN_INTERNAL_PULL_UP,       /** output open-drain with pull-up aka WiredANDPull-Up */
} gpio_mode_t;

/**
 * @brief Add gpio port name number as per need.
 *        Mapping of GPIO port name/number should be made in the 
 *        architecture specific file.
 * 
 */
typedef enum gpio_port {
  GPIO_PORT_A = 0,
  GPIO_PORT_B,
  GPIO_PORT_C,
  GPIO_PORT_D,
  GPIO_PORT_E,
  GPIO_PORT_F,
} gpio_port_t;

/**
 * @brief Add gpio interrupt mode as per need.
 * 
 */
typedef enum gpio_interrupt_mode {
  GPIO_INTERRUPT_MODE_RISING_EDGE,         /** rising edge interrupt */
  GPIO_INTERRUPT_MODE_FALLING_EDGE,        /** falling edge interrupt */
  GPIO_INTERRUPT_MODE_BOTH_EDGES,          /** both edges interrupt */
  GPIO_INTERRUPT_MODE_LEVEL_HIGH,          /** level high interrupt */
  GPIO_INTERRUPT_MODE_LEVEL_LOW,           /** level low interrupt */
  GPIO_INTERRUPT_MODE_BOTH_LEVELS,         /** both level interrupt */
  GPIO_INTERRUPT_MODE_NO_INTERRUPT         /** no interrupt */
} gpio_interrupt_mode_t;

/**
 * @brief structure to hold GPIO interrupt configuration
 * 
 */
typedef struct gpio_interrupt {
  const uint8_t pin;                      /** pin number */
  const uint8_t port;                     /** port number */
  const gpio_mode_t gpio_mode;                 /** gpio pin mode i.e. pulled up or low */
  const uint32_t debouncing_time_ms;      /** debouncing time in ms */
  const gpio_interrupt_mode_t int_mode;   /** interrupt mode, falling edge, rising edge etc. */
  const uint8_t int_no;                   /** interrupt number, usually the pin number for efr32 series */
  const bool low_power_interrupt;         /** if true, configure this as a low power interrupt */
  uint32_t pulse_start_ts_ms;             /** pulse start time stamp in ms */
  uint32_t pulse_time_ms;                 /** pulse stop time stamp in ms */
  void (*callback)(struct gpio_interrupt *ptr);                 /** callback function pointer */
} gpio_interrupt_t;

typedef enum gpio_pin_logic {
  GPIO_PIN_LOGIC_LOW = 0,                 /** pin logic low */
  GPIO_PIN_LOGIC_HIGH                     /** pin logic high */
} gpio_pin_logic_t;

/**
 * @brief function should initializes GPIO peripherals.
 *        This could include enabling the clock for GPIO peripherals,
 *        GPIO strength, GPIO mode, etc.
 */
void gpio_init(void);

/**
 * @brief function should set the GPIO mode and set the out reg value.
 * 
 * @param port  GPIO port number
 * @param pin   GPIO pin number
 * @param mode  GPIO mode to be set
 * @param out   GPIO output value to be set
 */
void gpio_set_mode(gpio_port_t port, uint8_t pin, gpio_mode_t mode, uint8_t out);

/**
 * @brief function should get GPIO pin value.
 *        This should be actual pin value.
 * 
 * @param port  GPIO port number
 * @param pin   GPIO pin number
 * @return GPIO pin value
 */
uint8_t gpio_get_pin_logic(gpio_port_t port, uint8_t pin);

/**
 * @brief function should set the GPIO pin value.
 * 
 * @param port  GPIO port number
 * @param pin   GPIO pin number
 * @param logic  GPIO pin logic value to be set
 */
void gpio_set_pin_logic(gpio_port_t port, uint8_t pin, gpio_pin_logic_t logic);

/**
 * @brief function should toggle the GPIO pin value.
 * 
 * @param port  GPIO port number
 * @param pin   GPIO pin number
 */
void gpio_toggle_pin_logic(gpio_port_t port, uint8_t pin);

/**
 * @brief function should enable or disable the GPIO interrupt.
 * 
 * @param gpio_interrupt  pointer to the gpio interrupt structure
 * @param enable          true to enable the interrupt, false to disable it
 */
void gpio_interrupt(gpio_interrupt_t *gpio_interrupt, bool enable);

/**
 * @brief function should set the GPIO interrupt callback function
 *        into the given gpio interrupt structure and register the
 *        interrupt device to the global interrupt device register 
 *        array.
 * 
 * @param gpio_interrupt  pointer to the gpio interrupt structure
 * @param callback        pointer to the callback function
 */
void gpio_set_interrupt_callback(gpio_interrupt_t *gpio_interrupt, void (*callback)(gpio_interrupt_t *ptr));

/**
 * @brief function should clear the GPIO interrupt.
 * 
 * @param gpio_interrupt  pointer to the gpio interrupt structure
 */
void gpio_clear_interrupt(gpio_interrupt_t *gpio_interrupt);
#endif /* _GPIO_H_ */
