
#include <stdint.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <logging/log.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "d39_gpio/d39_gpio.h"
#include "nmea.h"
#include "config.h"


LOG_MODULE_REGISTER(main, 3);

// bluetooth
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define ADV_LEN 12

/* DECLARATIONS */
void handle_timer(struct k_timer *dummy);

void bt_ready(int err);
static void connected(struct bt_conn *connected, uint8_t err);
static void disconnected(struct bt_conn *disconn, uint8_t reason);
static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);


K_TIMER_DEFINE(update_timer, handle_timer, NULL);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

/* Bluetoot service */

struct bt_conn *conn;
uint32_t current_count = 0;

/* Notification state */
volatile bool notify_enable;

static struct bt_uuid_128 service_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x7067452c, 0x0513, 0x41a0, 0xa0bd, 0xb8582a217bb0)
	);

static struct bt_uuid_128 characteristic_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x0000FFE1, 0x0000, 0x1000, 0x8000, 0x00805F9B34FB)
	);

BT_GATT_SERVICE_DEFINE(d39fuelsensor_svc,
		       BT_GATT_PRIMARY_SERVICE(&service_uuid),
		       BT_GATT_CHARACTERISTIC(&characteristic_uuid.uuid,
					      BT_GATT_CHRC_NOTIFY,
					      BT_GATT_PERM_READ, NULL, NULL, NULL),
		       BT_GATT_CCC(ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
		       );

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

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

	int err = bt_enable(NULL);

	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_ready(err);

	bt_conn_cb_register(&conn_callbacks);

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

	if (notify_enable) {
		bt_gatt_notify(NULL, &d39fuelsensor_svc.attrs[1], &buf, 15);
	}
}

/*
 * initialize bluetooth services
 */
void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Connection Callbacks registered\n");
}

static void connected(struct bt_conn *connected, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)", err);
	} else {
		printk("Connected");
		if (!conn) {
			conn = bt_conn_ref(connected);
		}
	}
}

static void disconnected(struct bt_conn *disconn, uint8_t reason)
{
	if (conn) {
		bt_conn_unref(conn);
		conn = NULL;
	}

	printk("Disconnected (reason %u)", reason);
}

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);
	notify_enable = (value == BT_GATT_CCC_NOTIFY);
}
