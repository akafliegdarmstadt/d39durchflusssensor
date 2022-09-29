#pragma once

#include "config.h"

#define BT_CONN_AUTH_PASSKEY 393939

int d39_bt_init();
int d39_bt_send_msg(char *buf, size_t len);
