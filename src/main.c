
#include <zephyr.h>
#include <sys/printk.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/addr.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "nmea.h"

#define SENSOR_THREAD_STACK_SIZE 500
#define SENSOR_THREAD_PRIORITY 5

void timer_handler(struct k_timer *dummy);


K_TIMER_DEFINE(update_timer, timer_handler, NULL);

void main(void)
{

	printk("Hello World! %s\n %d\n", CONFIG_BOARD, 1);

	k_timer_start(&update_timer, K_SECONDS(1), K_SECONDS(1));
}

void timer_handler(struct k_timer *dummy) {
	printk("Hallololo\n");
	printk("Hallololo\n");
}