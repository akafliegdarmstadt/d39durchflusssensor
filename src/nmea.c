#include "nmea.h"
#include "string.h"
#include "stdio.h"
#include <sys/cbprintf.h>

/* build nmea message for fuel flow sensor */
void nmeaflow_buildmsg(char *buf, double val) {
    
    // create string from value
    char val_str[8] = "";
    snprintfcb(val_str, 8, "%#05.1f", val);
    
    // calculate checksum
    uint8_t cs = nmeaflow_checksum(val_str, 0, strlen(val_str), PFLO_CS_SEED);

    // build complete msg
    snprintfcb(buf, 15, "$PFLO,%s*%2X\n", val_str, cs);
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