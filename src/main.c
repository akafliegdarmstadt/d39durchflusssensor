
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "nmea.h"

/* CONFIGURATION */
#define SENSOR_THREAD_STACK_SIZE 500
#define SENSOR_THREAD_PRIORITY 5

#define SENS_PIN 4

/* DECLARATIONS */
const struct device *initialize_gpio();

void handle_tick(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void handle_timer(struct k_timer *dummy);

/* GLOBALS */
static struct gpio_callback tick_cb_data;
const struct device *gpio0;

// use atomic variable to circumvent semaphore/mutex and so on
atomic_t interrupt_count;

K_TIMER_DEFINE(update_timer, handle_timer, NULL);

/*
 * Entry Point 
 */
void main(void)
{
	if(initialize_gpio()==NULL) {
		printk("Failed to initialize GPIO");
		return;
	}

	printk("Hello World! %s\n", CONFIG_BOARD);

	k_timer_start(&update_timer, K_SECONDS(1), K_SECONDS(1));
}

/* 
 * Reset Tick counter and send update. 
 */
void handle_timer(struct k_timer *dummy) {
	int pin_in_val = gpio_pin_get_raw(gpio0, SENS_PIN);
	printk("Update - %d (%d)\n", pin_in_val, atomic_get(&interrupt_count));
	atomic_set(&interrupt_count, (atomic_val_t)0);
}

/*
 * setup gpio0 SENS_PIN
 */
const struct device *initialize_gpio() {
	
	int ret;

	gpio0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));
	if (gpio0==NULL) {
		printk("Could not get %s device\n", DT_LABEL(DT_NODELABEL(gpio0)));
		return NULL;
	}

	ret = gpio_pin_configure(gpio0, SENS_PIN, GPIO_INPUT | GPIO_PULL_DOWN | GPIO_INT_DEBOUNCE);
	if(ret) {
		printk("Failed to configure GPIO input (%d).", ret);
		return NULL;
	}

	if(gpio_pin_interrupt_configure(gpio0, SENS_PIN, GPIO_INT_TRIG_HIGH)) {
		printk("Failed to configure GPIO interrupt.");
		return NULL;
	}

	gpio_init_callback(&tick_cb_data, handle_tick, BIT(SENS_PIN));
	gpio_add_callback(gpio0, &tick_cb_data);

	return gpio0;
}

/*
 * increase counter on each interrupt for SENS_PIN
 */
void handle_tick(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	atomic_inc(&interrupt_count);
}
