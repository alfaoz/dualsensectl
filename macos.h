// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUALSENSECTL_MACOS_H
#define DUALSENSECTL_MACOS_H

#include <time.h>

/* Portable thrd_sleep replacement using nanosleep on macOS */
#define thrd_sleep(ts, rem) nanosleep((ts), (rem))

typedef void (*macos_device_callback)(const char *serial_number);

int macos_monitor(int vendor_id, int product_id, int edge_product_id,
                  macos_device_callback add_device,
                  macos_device_callback remove_device);

#endif
