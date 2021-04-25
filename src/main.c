
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

extern void my_entry_point(void *, void *, void *);

K_THREAD_STACK_DEFINE(sensor_stack_area, SENSOR_THREAD_STACK_SIZE);
struct k_thread sensor_thread_data;

void main(void)
{
	k_tid_t sensor_thread_id = k_thread_create(&sensor_thread_data, sensor_stack_area,
										   K_THREAD_STACK_SIZEOF(sensor_stack_area),
										   my_entry_point,
										   NULL, NULL, NULL,
										   SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
	int a = (int) sensor_thread_id;
	printk("Hello World! %s\n %d", CONFIG_BOARD, a);
}
