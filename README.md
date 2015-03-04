# foohid
OSX IOKit driver for implementing virtual HID devices from userspace

Quick start
===========

Install https://github.com/unbit/foohid/releases/download/0.1/foohid.pkg and reboot your system.

The kext will expose a IOUserClient allowing you to create and manage virtual HID devices.

Use the official python2 wrapper (available at https://github.com/unbit/foohid-py) to start playing with virtual hid devices:

```sh
pip install foohid
```

(will install the foohid extension).

Now clone the foohid-py repository

```sh
git clone https://github.com/unbit/foohid-py
```

where 2 tests are available:

test_mouse.py will create a virtual mouse. Just run it, and every second your mouse pointer will move to a random position

test_list.py will show the listing feature (a bunch of virtual devices will be created, listed and destroyed)

Note: python3 support should be ready soon


The IOUserClient api
====================

4 Methods are exposed:

* create (selector 0)
* destroy (selector 1)
* send (selector 2)
* list (selector 3)

create
------

takes 4 input arguments (name pointer, name length, reportdescriptor pointer, reportdescriptor length) and 1 output argument (64bit integer set to 0 on success).

It creates a new fake/virtual HID device with the specified reportdescriptor

destroy
-------

takes 2 input arguments (name pointer, name length) and 1 output argument (64bit integer set to 0 on success).

It cancels a previously created fake/virtual HID device

send
----

takes 4 input arguments (name pointer, name length, msg pointer, msg length) and 1 output argument (64bit integer set to 0 on success).

It generates a HID event from a previously created fake/virtual HID device

list
----

takes 2 input arguments (buffer pointer, buffer length) and 2 output arguments (needed bytes and returned items)

It returns (into the supplied buffer pointer) the list of available fake/virtual devices separated by \0. The items output value contains the number of returned items. If the supplied buffer is not big enough, the needed bytes value contains a suggestion for a second run

An example report descriptor
============================

Building report descriptors is a really annoying task.

To simplify testing, the following one is a generic mouse (thanks http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/)

```c
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
}
```

that maps to this c structure

```c
struct mouse_report_t {
    uint8_t buttons;
    int8_t x;
    int8_t y;
}
```

So to create and move a virtual mouse:

```c

#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOOHID_CREATE 0

int main() {

    io_iterator_t   iterator;
    io_service_t    service;
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("it_unbit_foohid"), &iterator);
    
    if (ret != KERN_SUCCESS) {
        printf("unable to access IOService\n");
        exit(1);
    }
    
    io_connect_t connect;
    
    int found = 0;
    
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
            ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
            if (ret = KERN_SUCCESS) {
                found = 1;
                break;
            }
    }
    IOObjectRelease(iterator);
    
    if (!found) {
        printf("unable to open IOService\n");
        exit(1);
    }

    uint32_t output_count = 1;
    uint64_t output = 0;
    
    uint64_t input[4];
    input[0] = (uint64_t) strdup("Virtual GamePad FooBar");
    input[1] = strlen( (char *)input[0]);
    input[2] = (uint64_t) report_descriptor;
    input[3] = sizeof(report_descriptor);

    ret = IOConnectCallScalarMethod(connect, FOOHID_CREATE, input, 4, &output, &output_count);
    if (ret != KERN_SUCCESS) {
        printf("unable to create HID device\n");
        exit(1);
    }

    struct mouse_report_t mouse;
    input[2] = (uint64_t) &mouse;
    input[3] = sizeof(struct mouse_report_t);
    
    for(;;) {
        mouse.buttons = 0;
        mouse.x = rand();
        mouse.y = rand();
        
        // ignore return value, just for testing
        IOConnectCallScalarMethod(connect, 2, input, 4, &output, &output_count);
    }
}
```
