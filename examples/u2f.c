/**
 * Create a virtual U2F device.
 * Compile me with: gcc u2f.c -o virtual_u2f -framework IOKit
 */

#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CFRunLoop.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "foohid_types.h"

unsigned char report_descriptor[] = {
    0x06, 0xD0, 0xF1,  // Usage Page (Reserved 0xF1D0)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x20,        //   Usage (0x20)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x09, 0x21,        //   Usage (0x21)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x40,        //   Report Count (64)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

#define SERVICE_NAME "it_unbit_foohid"

#define FOOHID_CREATE  0 // create selector
#define FOOHID_DESTROY 1 // destroy selector
#define FOOHID_SEND    2 // send selector
#define FOOHID_LIST    3 // list selector
#define FOOHID_NOTIFY  4 // notify selector

#define DEVICE_NAME "Foohid Virtual U2F Device"
#define DEVICE_SN "SN 123456"

io_connect_t foohid_connect;
CFRunLoopRef run_loop = NULL;

void cleanup(int sig) {
    kern_return_t ret;
    
    printf("Cleaning up...\n");
    
    if (run_loop)
      CFRunLoopStop(run_loop);

    // Destroy the device we created.
    uint32_t destroy_count = 2;
    uint64_t destroy[destroy_count];
    destroy[0] = (uint64_t) strdup(DEVICE_NAME); // device name
    destroy[1] = strlen((char *)destroy[0]);     // name length

    ret = IOConnectCallScalarMethod(foohid_connect, FOOHID_DESTROY, destroy, destroy_count, NULL, NULL);
    if (ret != KERN_SUCCESS) {
        printf("Unable to destroy HID device. May be fine if it wasn't created.\n");
    }

    // Close our connection to the user client.
    IOServiceClose(foohid_connect);

    exit(0);
}

void callback(void *refcon, IOReturn result, io_user_reference_t* args, uint32_t numArgs) {
  foohid_report *report;

  printf("callback called.\n");

  if (sizeof(io_user_reference_t) * numArgs != sizeof(foohid_report)) {
      printf("unexpected number of arguments.\n");
      return;
  }

  report = (foohid_report *)args;
  printf("received report (%llu bytes).\n", report->size);
}

void run() {
  IONotificationPortRef notification_port;
  mach_port_t mnotification_port;
  CFRunLoopSourceRef run_loop_source;
  io_async_ref64_t async_ref;
  kern_return_t ret;

  // Create port to listen for kernel notifications on.
  notification_port = IONotificationPortCreate(kIOMasterPortDefault);
  if (!notification_port) {
    printf("Error getting notification port.\n");
    return;
  }

  // Get lower level mach port from notification port.
  mnotification_port = IONotificationPortGetMachPort(notification_port);
  if (!mnotification_port) {
    printf("Error getting mach notification port.\n");
    return;
  }

  // Create a run loop source from our notification port so we can add the port to our run loop.
  run_loop_source = IONotificationPortGetRunLoopSource(notification_port);
  if (run_loop_source == NULL) {
    printf("Error getting run loop source.\n");
    return;
  }

  // Add the notification port and timer to the run loop.
  CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop_source, kCFRunLoopDefaultMode);

  // Params to pass to the kernel.
  async_ref[kIOAsyncCalloutFuncIndex] = (uint64_t)callback;
  async_ref[kIOAsyncCalloutRefconIndex] = 0;
  
  uint32_t sub_count = 2;
  uint64_t sub[sub_count];
  sub[0] = (uint64_t) strdup(DEVICE_NAME); // device name
  sub[1] = strlen((char *)sub[0]);     // name length

  // Tell the kernel how to notify us.
  ret = IOConnectCallAsyncScalarMethod(foohid_connect, FOOHID_NOTIFY, mnotification_port, async_ref, kIOAsyncCalloutCount, sub, sub_count, NULL, 0);
  if (ret != kIOReturnSuccess) {
    printf("Error registering for setFrame notifications.\n");
    return;
  }

  // Blocks until the run loop is stopped in our callback.
  printf("Starting async run loop.\n");
  run_loop = CFRunLoopGetCurrent();
  CFRunLoopRun();
  run_loop = NULL;

  // Clean up.
  IONotificationPortDestroy(notification_port);
}

int main() {
    io_iterator_t iterator;
    io_service_t service;

    // Get a reference to the IOService
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(SERVICE_NAME), &iterator);

    if (ret != KERN_SUCCESS) {
        printf("Unable to access IOService.\n");
        exit(1);
    }

    // Iterate till success
    int found = 0;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, &foohid_connect);

        if (ret == KERN_SUCCESS) {
            found = 1;
            break;
        }

        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);

    if (!found) {
        printf("Unable to open IOService.\n");
        cleanup(0);
        exit(1);
    }

    // Register handler for interrupts.
    signal(SIGHUP, cleanup);
    signal(SIGINT, cleanup);
    signal(SIGQUIT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGKILL, cleanup);

    // Fill up the input arguments for device creation.
    uint32_t create_count = 8;
    uint64_t create[create_count];
    create[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    create[1] = strlen((char *)create[0]);  // name length
    create[2] = (uint64_t) report_descriptor;  // report descriptor
    create[3] = sizeof(report_descriptor);  // report descriptor len
    create[4] = (uint64_t) strdup(DEVICE_SN);  // serial number
    create[5] = strlen((char *)create[4]);  // serial number len
    create[6] = (uint64_t) 2;  // vendor ID
    create[7] = (uint64_t) 3;  // device ID

    // Create HID device.
    ret = IOConnectCallScalarMethod(foohid_connect, FOOHID_CREATE, create, create_count, NULL, NULL);
    if (ret != KERN_SUCCESS) {
        printf("Unable to create HID device. May be fine if created previously.\n");
    }
    
    printf("Starting run loop.\n");
    run();
    printf("Run loop stopped.\n");
}