#include <zephyr.h>
#include <drivers/gpio.h>

#include "config.h"
#include "d39_gpio.h"


static const struct device *gpio0;
static struct gpio_callback tick_cb_data;

atomic_t interrupt_count = ATOMIC_INIT(0);

void handle_tick(const struct device *dev, struct gpio_callback *cb, uint32_t pint) {
    atomic_inc(&interrupt_count);
}

/**
 * Sets up the gpio pin and adds the corresponding callback.
 * @param err Gets set to an error message in case of an error.
 */
int d39_gpio_init() {
    gpio0 = device_get_binding(DT_LABEL(DT_NODELABEL(gpio0)));

    if (gpio0 == NULL) {
        return -ENXIO;
    }

    int ret = gpio_pin_configure(gpio0, SENS_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret != 0) {
        return ret;
    }

    ret = gpio_pin_interrupt_configure(gpio0, SENS_PIN, GPIO_INT_TRIG_LOW);
    if (ret != 0) {
        return ret;
    }

    // These do not return an error code
    gpio_init_callback(&tick_cb_data, handle_tick, BIT(SENS_PIN));
    gpio_add_callback(gpio0, &tick_cb_data);

    return 0;
}

/**
 * Gets the interrupt count and sets it to zero.
 * @param err Gets set to a error message in case of an error.
 */
uint32_t d39_gpio_get_and_reset_ticks() {
    // atomic_set returns the previous value of the atomic
    return atomic_set(&interrupt_count, (atomic_val_t)0);
}
