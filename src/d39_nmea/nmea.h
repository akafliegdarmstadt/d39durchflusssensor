#include "stdint.h"

/* checksum for "PFLO," string part */
#define PFLO_CS_SEED 57
#define NMEA_LENGTH 16

void nmeaflow_buildmsg(char *buf, double val);
uint8_t nmeaflow_checksum(const char *line, int start_id, int end_id, uint8_t cs_seed);
