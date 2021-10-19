#include <zephyr.h>

#include "config.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

static void connected_cb(struct bt_conn *connected, uint8_t err);
static void disconnected_cb(struct bt_conn *connected, uint8_t reason);
static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN)
};

/// The bluetooth connection object
struct bt_conn *conn;

/// Should we be sending push updates?
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
	.connected = connected_cb,
	.disconnected = disconnected_cb,
};

/**
 * Initializes the bluetooth driver and registers the callbacks.
 * @param err Gets set to an error message if an error occurrs.
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

int d39_bt_enable() {
	return bt_enable(bt_ready);
}

void d39_bt_send_msg(char *buf, size_t len) {
    if (!notify_enable) {
        return;
    }

    int ret = bt_gatt_notify(NULL, &d39fuelsensor_svc.attrs[1], &buf, len);
    if (ret < 0) {
        return ret;
    }
}

static void connected_cb(struct bt_conn *connected, uint8_t err)
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

static void disconnected_cb(struct bt_conn *disconn, uint8_t reason)
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
