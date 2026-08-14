// SPDX-License-Identifier: GPL-2.0-or-later

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>

#include "macos.h"

struct monitor_context {
    macos_device_callback add_device;
    macos_device_callback remove_device;
};

static void get_serial_from_hid_device(IOHIDDeviceRef device, char serial_number[18])
{
    CFStringRef serial = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDSerialNumberKey));
    if (serial && CFGetTypeID(serial) == CFStringGetTypeID()) {
        char buf[64];
        if (CFStringGetCString(serial, buf, sizeof(buf), kCFStringEncodingUTF8)) {
            /* Serial may come as "aa-bb-cc-dd-ee-ff" or "aa:bb:cc:dd:ee:ff" */
            size_t len = strlen(buf);
            if (len == 17) {
                /* Replace dashes with colons if needed, uppercase */
                for (size_t i = 0; i < len; i++) {
                    if (buf[i] == '-') buf[i] = ':';
                    serial_number[i] = toupper(buf[i]);
                }
                serial_number[len] = '\0';
                return;
            }
        }
    }
    strncpy(serial_number, "00:00:00:00:00:00", 18);
}

static void iokit_device_added(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
    (void)result;
    (void)sender;

    struct monitor_context *monitor = context;
    char serial_number[] = "00:00:00:00:00:00";
    get_serial_from_hid_device(device, serial_number);
    if (monitor->add_device) {
        monitor->add_device(serial_number);
    }
}

static void iokit_device_removed(void *context, IOReturn result, void *sender, IOHIDDeviceRef device)
{
    (void)result;
    (void)sender;

    struct monitor_context *monitor = context;
    char serial_number[] = "00:00:00:00:00:00";
    get_serial_from_hid_device(device, serial_number);
    if (monitor->remove_device) {
        monitor->remove_device(serial_number);
    }
}

int macos_monitor(int vendor_id_value, int product_id_value, int edge_product_id_value,
                  macos_device_callback add_device,
                  macos_device_callback remove_device)
{
    IOHIDManagerRef manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!manager) {
        fprintf(stderr, "Failed to create IOHIDManager\n");
        return 1;
    }

    CFNumberRef vendor_id = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vendor_id_value);
    CFNumberRef product_id = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &product_id_value);
    CFNumberRef edge_product_id = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &edge_product_id_value);

    CFDictionaryRef match_ds = CFDictionaryCreate(kCFAllocatorDefault,
        (const void *[]){ CFSTR(kIOHIDVendorIDKey), CFSTR(kIOHIDProductIDKey) },
        (const void *[]){ vendor_id, product_id },
        2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryRef match_edge = CFDictionaryCreate(kCFAllocatorDefault,
        (const void *[]){ CFSTR(kIOHIDVendorIDKey), CFSTR(kIOHIDProductIDKey) },
        (const void *[]){ vendor_id, edge_product_id },
        2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryRef matches[] = { match_ds, match_edge };
    CFArrayRef match_array = CFArrayCreate(kCFAllocatorDefault, (const void **)matches, 2, &kCFTypeArrayCallBacks);

    IOHIDManagerSetDeviceMatchingMultiple(manager, match_array);

    struct monitor_context context = {
        .add_device = add_device,
        .remove_device = remove_device,
    };
    IOHIDManagerRegisterDeviceMatchingCallback(manager, iokit_device_added, &context);
    IOHIDManagerRegisterDeviceRemovalCallback(manager, iokit_device_removed, &context);

    IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    IOReturn ret = IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
    if (ret != kIOReturnSuccess) {
        fprintf(stderr, "Failed to open IOHIDManager: %#04x\n", ret);
        CFRelease(match_array);
        CFRelease(match_edge);
        CFRelease(match_ds);
        CFRelease(edge_product_id);
        CFRelease(product_id);
        CFRelease(vendor_id);
        CFRelease(manager);
        return 1;
    }

    /* Run the event loop — blocks until interrupted */
    CFRunLoopRun();

    IOHIDManagerClose(manager, kIOHIDOptionsTypeNone);
    CFRelease(match_array);
    CFRelease(match_edge);
    CFRelease(match_ds);
    CFRelease(edge_product_id);
    CFRelease(product_id);
    CFRelease(vendor_id);
    CFRelease(manager);

    return 0;
}
