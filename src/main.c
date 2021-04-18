
#include <zephyr.h>
#include <sys/printk.h>
#include "stdint.h"
#include "string.h"
#include "stdio.h"

/* checksum for "PFLO," string part */
#define PFLO_CS_SEED 57

void nmeaflow_buildmsg(char *buf, double val);
uint8_t nmeaflow_checksum(const char *line, int start_id, int end_id, uint8_t cs_seed);

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
}

/* build nmea message for fuel flow sensor */
void nmeaflow_buildmsg(char *buf, double val) {
    
    // create string from value
    char val_str[7];
    sprintf(val_str, "%#05.1f", val);
    
    // calculate checksum
    uint8_t cs = nmeaflow_checksum(buf, 0, strlen(val_str), PFLO_CS_SEED);

    // build complete msg
    sprintf(buf, "$PFLO,%s*%2X\n", val_str, cs);
}

/* Calculate Checksum based on sequential XOR operations */
uint8_t nmeaflow_checksum(const char *line, int start_id, int end_id, uint8_t cs_seed) {
    uint8_t cs = cs_seed;

    // XOR each character with checksum
    for (int i=start_id; i<end_id; i++) {
        cs ^= line[i];
    }

    return cs;
}