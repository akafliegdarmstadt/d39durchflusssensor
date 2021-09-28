
#include <stdint.h>
#include <zephyr.h>
#include <device.h>
#include <sys/printk.h>
#include <logging/log.h>

#include "d39_nmea/nmea.h"
#include "d39_gpio/d39_gpio.h"
#include "d39_bt/d39_bt.h"

LOG_MODULE_REGISTER(main, 3);

/* CONFIGURATION */
#include "config.h"

/* DECLARATIONS */
void handle_timer(struct k_timer *dummy);

void bt_ready(int err);
static void connected(struct bt_conn *connected, uint8_t err);
static void disconnected(struct bt_conn *disconn, uint8_t reason);
static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

/* GLOBALS */

K_TIMER_DEFINE(update_timer, handle_timer, NULL);

/*
 * Entry Point
 */
void main(void)
{
	char *err = NULL;

	d39_init_gpio(&err);
	if (err) {
		printk("ERROR: %s", err);
		return;
	}

	printk("Hello World! %s\n", CONFIG_BOARD);

	int _err = bt_enable(NULL);

	if (_err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	} else {
		printk("Bluetooth initialized");
	}

	bt_ready(_err);
	bt_conn_cb_register(&conn_callbacks);

	k_timer_start(&update_timer, K_SECONDS(1), K_SECONDS(1));
}

/*
 * Reset Tick counter and send update.
 */
void handle_timer(struct k_timer *dummy)
{
	char *err = NULL;

	// getticks can't fail so no need to check
	const uint32_t ticks = d39_gpio_getticks(&err);
	const double flow_rate = ticks / TIKSCONF;

	char buf[NMEA_LENGTH] = "";
	nmeaflow_buildmsg(buf, flow_rate);

	printk("Update - ints: %d, %s\n", ticks, buf);

	d39_bt_send_msg(buf, 15, &err);
	if (err) {
		printk("ERROR: %s", err);
	}
}