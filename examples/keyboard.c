/**
 * Create a virtual keyboard.
 * Compile me with: gcc keyboard.c -o virtual_keyboard -framework IOKit
 */

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x01,                    //   INPUT (Cnst,Ary,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x01,                    //   OUTPUT (Cnst,Ary,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0,                          // END_COLLECTION

    // CONSUMER CONTROL
    //
    0x05,0x0C,                          // USAGE_PAGE (Consumer Devices)
    0x09,0x01,                          // USAGE (Consumer Control)
    0xA1,0x01,                          // COLLECTION (Application)
    0x85,0x02,                          //   REPORT_ID (2)
    0x05,0x0C,                          //   USAGE_PAGE (Consumer Devices)
    0x19,0x00,                          //   USAGE_MINIMUM (Unassigned)
    0x2A,0x08,0x01,                     //   USAGE_MAXIMUM (Police Alarm)
    0x15,0x00,                          //   LOGICAL_MINIMUM (0)
    0x26,0x08,0x01,                     //   LOGICAL_MAXIMUM (264)
    0x75,0x10,                          //   REPORT_SIZE (16)
    0x95,0x02,                          //   REPORT_COUNT (2)
    0x81,0x00,                          //   INPUT (Data,Ary,Abs)
    0xC0                                // END_COLLECTION
};

typedef struct keyboard_report_t {
    uint8_t report_id;
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t key_codes[6];
} keyboard_report_t;

typedef struct consumer_report_t {
    uint8_t report_id;
    uint8_t key_codes[4];
} consumer_report_t;


typedef union composite_report_t {
    keyboard_report_t keyboard;
    consumer_report_t consumer;
} composite_report_t;

#define SERVICE_NAME "it_unbit_foohid"

#define FOOHID_CREATE 0  // create selector
#define FOOHID_SEND 2  // send selector

#define DEVICE_NAME "Foohid Virtual KB"
#define DEVICE_SN "SN 123456"

int main() {
    io_iterator_t iterator;
    io_service_t service;
    io_connect_t connect;

    // Get a reference to the IOService
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(SERVICE_NAME), &iterator);

    if (ret != KERN_SUCCESS) {
        printf("Unable to access IOService.\n");
        exit(1);
    }

    // Iterate till success
    int found = 0;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, &connect);

        if (ret == KERN_SUCCESS) {
            found = 1;
            break;
        }

        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);

    if (!found) {
        printf("Unable to open IOService.\n");
        exit(1);
    }

    // Fill up the input arguments.
    uint32_t input_count = 8;
    uint64_t input[input_count];
    input[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    input[1] = strlen((char *)input[0]);  // name length
    input[2] = (uint64_t) report_descriptor;  // report descriptor
    input[3] = sizeof(report_descriptor);  // report descriptor len
    input[4] = (uint64_t) strdup(DEVICE_SN);  // serial number
    input[5] = strlen((char *)input[4]);  // serial number len
    input[6] = (uint64_t) 2;  // vendor ID
    input[7] = (uint64_t) 3;  // device ID

    ret = IOConnectCallScalarMethod(connect, FOOHID_CREATE, input, input_count, NULL, 0);
    if (ret != KERN_SUCCESS) {
        printf("Unable to create HID device. May be fine if created previously.\n");
    }

    // Arguments to be passed through the HID message.
    composite_report_t report;

    uint32_t send_count = 4;
    uint64_t send[send_count];
    send[0] = (uint64_t)input[0];  // device name
    send[1] = strlen((char *)input[0]);  // name length
    send[2] = (uint64_t) &report;  // keyboard struct
    send[3] = sizeof(composite_report_t);  // keyboard struct len

    int i = 0;
    for(i = 0; i < 5 ; i++) {
        memset(&report, 0, sizeof(report));
        report.keyboard.report_id = 0x01;
        report.keyboard.key_codes[0] = kHIDUsage_KeyboardA;

        ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
        if (ret != KERN_SUCCESS) {
            printf("Unable to send message to HID device.\n");
        }

        report.keyboard.key_codes[0] = 0x00;
        ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
        if (ret != KERN_SUCCESS) {
            printf("Unable to send message to HID device.\n");
        }

        memset(&report, 0, sizeof(report));
        report.consumer.report_id = 0x02;
        report.consumer.key_codes[0] = kHIDUsage_Csmr_Mute;

        ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
        if (ret != KERN_SUCCESS) {
            printf("Unable to send message to HID device.\n");
        }

        report.consumer.key_codes[0] = 0x00;
        ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
        if (ret != KERN_SUCCESS) {
            printf("Unable to send message to HID device.\n");
        }

        sleep(1);  // sleep for a second
    }
}

