/**
 * Create a virtual mouse.
 * Compile me with: gcc mouse.c -o virtual_mouse -framework IOKit
 */

#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

struct mouse_report_t {
    uint8_t buttons;
    int8_t x;
    int8_t y;
};

#define SERVICE_NAME "it_unbit_foohid"

#define FOOHID_CREATE 0  // create selector
#define FOOHID_SEND 2  // send selector

#define DEVICE_NAME "Foohid Virtual Mouse"
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
        printf("Unable to create HID device.\n");
        exit(1);
    }

    // Arguments to be passed through the HID message.
    struct mouse_report_t mouse;
    uint32_t send_count = 4;
    uint64_t send[send_count];
    send[0] = (uint64_t)input[0];  // device name
    send[1] = strlen((char *)input[0]);  // name length
    send[2] = (uint64_t) &mouse;  // mouse struct
    send[3] = sizeof(struct mouse_report_t);  // mouse struct len

    for(;;) {
        mouse.buttons = 0;
        mouse.x = rand();
        mouse.y = rand();

        ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
        if (ret != KERN_SUCCESS) {
            printf("Unable to send message to HID device.\n");
        }

        sleep(1);  // sleep for a second
    }
}
