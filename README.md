# foohid
OSX IOKit driver for implementing virtual HID devices from userspace.

## Quick start (for OSX Yosemite)

Install `https://github.com/unbit/foohid/releases/download/0.1/foohid.pkg` and reboot your system.
The kext will expose a `IOUserClient` allowing you to create and manage virtual HID devices (see the [examples section below](#an-example-with-report-descriptor)).

You can use the official Python 2 wrapper (also available at https://github.com/unbit/foohid-py) to start playing with virtual hid devices.
First of all, install the foohid Python 2 extension:

```sh
pip install foohid
```

Now clone the foohid-py repository:

```sh
git clone https://github.com/unbit/foohid-py
cd foohid-py
```

Three tests are available inside the repository directory:
* `test_mouse.py` will create a virtual mouse. Just run it and every second your mouse pointer will move to a random position of the screen.
* `test_joypad.py` will create a virtual joypad. Just run it and every second lext and right axis will be updated.
* `test_list.py` will show the listing feature (a bunch of virtual devices will be created, then listed and destroyed).

Note: Python 3 support should be ready soon.

## The `IOUserClient` API

4 methods are exposed:

* `create` (selector 0)
* `destroy` (selector 1)
* `send` (selector 2)
* `list` (selector 3)

### create
Creates a new fake/virtual HID device with the specified report_descriptor.

Takes 7 input arguments:

1. name pointer.
2. name length.
3. report_descriptor pointer.
4. report_descriptor length.
5. device serial number.
6. device vendor ID.
7. device product ID.

And 1 output argument:

1. 64 bit integer set to 0 on success.

### destroy

Takes 2 input arguments:

1. name pointer.
2. name length.

And 1 output argument:

1. 64 bit integer set to 0 on success.

### send

Generate a HID event from a previously created fake/virtual HID device.

Takes 4 input arguments:

1. name pointer.
2. name length.
3. report_descriptor pointer.
4. report_descriptor length.

And 1 output argument:

1. 64 bit integer set to 0 on success.

### list

Return (into the supplied buffer pointer) the list of available fake/virtual devices separated by `\0`. 
The items output value contains the number of returned items. 
If the supplied buffer is not big enough, the needed bytes value contains a suggestion for a second run.

Takes 2 input arguments:

1. buffer pointer.
2. buffer length.

And 2 output arguments:

1. needed bytes
2. returned items.


### An example (with report descriptor)

Building report descriptors is a really annoying task.
To simplify testing, the following one is a generic mouse (thanks http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/).

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

It maps to the following structure:

```c
struct mouse_report_t {
    uint8_t buttons;
    int8_t x;
    int8_t y;
}
```

To create and move a virtual mouse:

```c
#include <IOKit/IOKitLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char report_descriptor[] = {
    // ...
};

struct mouse_report_t {
    // ...
};

#define FOOHID_CREATE 0  // create selector
#define FOOHID_SEND 2  // send selector
#define DEVICE_NAME "Foohid Virtual Mouse"

int main() {

    io_iterator_t   iterator;
    io_service_t    service;

    // get a reference to the IOService
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("it_unbit_foohid"), &iterator);

    if (ret != KERN_SUCCESS) {
        printf("unable to access IOService\n");
        exit(1);
    }

    io_connect_t connect;

    // Iterate till success
    int found = 0;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
        if (ret == KERN_SUCCESS) {
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

    // fill input args
    uint32_t input_count = 7;
    uint64_t input[input_count];
    input[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    input[1] = strlen((char *)input[0]);  // name length
    input[2] = (uint64_t) report_descriptor;  // report descriptor
    input[3] = sizeof(report_descriptor);  // report descriptor len
    input[4] = (uint64_t) 1;  // serial number
    input[5] = (uint64_t) 2;  // vendor ID
    input[6] = (uint64_t) 3;  // device ID

    ret = IOConnectCallScalarMethod(connect, FOOHID_CREATE, input, input_count, &output, &output_count);
    if (ret != KERN_SUCCESS) {
        printf("unable to create HID device\n");
        exit(1);
    }

    // args to pass hid message
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

        // ignore return value, just for testing
        IOConnectCallScalarMethod(connect, FOOHID_SEND, input, 4, &output, &output_count);

        sleep(1);  // sleep for a second
    }
}
```

#### Run it
You can found the previous example [here](examples/mouse.c).
Compile it and run it with:

```bash
curl -O https://raw.githubusercontent.com/unbit/foohid/develop/examples/mouse.c
gcc mouse.c -o virtual_mouse -framework IOKit
./virtual_mouse
```

## Logging

Logging is disabled by default.
You can enable it by building a DEBUG version by using XCode, or manually setting the `-DDEBUG` preprocessor flag.
