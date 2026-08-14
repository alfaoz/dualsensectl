// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUALSENSECTL_MACOS_H
#define DUALSENSECTL_MACOS_H

typedef void (*macos_device_callback)(const char *serial_number);

int macos_monitor(int vendor_id, int product_id, int edge_product_id,
                  macos_device_callback add_device,
                  macos_device_callback remove_device);

#endif
