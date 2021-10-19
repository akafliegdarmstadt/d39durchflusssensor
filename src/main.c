
#include <stdint.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <logging/log.h>

#include "d39_gpio/d39_gpio.h"
#include "d39_bt/d39_bt.h"
#include "nmea.h"
#include "config.h"


LOG_MODULE_REGISTER(main, 3);

/* DECLARATIONS */
void handle_timer(struct k_timer *dummy);

K_TIMER_DEFINE(update_timer, handle_timer, NULL);

/*
 * Entry Point
 */
void main(void)
{
	int ret = d39_gpio_init();
	if (ret != 0) {
		printk("Failed to initialize GPIO, error %d", ret);
		return;
	}

	printk("Hello World! %s\n", CONFIG_BOARD);

	int err = d39_bt_enable();

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	k_timer_start(&update_timer, K_SECONDS(1), K_SECONDS(1));
}

/*
 * Reset Tick counter and send update.
 */
void handle_timer(struct k_timer *dummy)
{
	uint32_t tiks = d39_gpio_get_and_reset_ticks();
	double flow_rate =  tiks / TIKSCONF;
	char buf[NMEA_LENGTH] = "";

	nmeaflow_buildmsg(buf, flow_rate);

	printk("Update - %d, %s\n", tiks, buf);

	d39_bt_send_msg( &buf, 15);
}
