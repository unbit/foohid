# foohid

OSX IOKit driver for implementing virtual HID devices from userspace.

*** The foohid driver is currently unsupported and lacks proper thread-safety (leading to security problems), please do not use it
in production unless you want to sponsor the project contacting info at unbit dot it ***

## Examples
You can check the [examples section below](#an-example-with-report-descriptor), the [examples directory](examples) or use the Python 2 wrapper (next section).

### The Python 2 wrapper

You can use the official Python 2 wrapper (available on PyPi and [here](https://github.com/unbit/foohid-py)) to start playing with virtual HID devices.
First of all, install the `foohid` Python 2 extension:

```sh
pip install foohid
```

Now clone the `foohid-py` repository:

```sh
git clone https://github.com/unbit/foohid-py
cd foohid-py
```

Three tests are available inside the repository directory:
* `test_mouse.py` will create a virtual mouse. Just run it and every second your mouse pointer will move to a random position of the screen.
* `test_joypad.py` will create a virtual joypad. Just run it and every second left and right axes will be updated.
* `test_list.py` will show the listing feature (a bunch of virtual devices will be created, then listed and destroyed).

Note: Python 3 support should be ready soon.

### A C example (with report descriptor)

Building report descriptors is a really annoying task.
To simplify testing, the following describes a generic mouse (thanks [eleccelerator](http://eleccelerator.com/tutorial-about-usb-hid-report-descriptors/)).

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

To create and move around a virtual mouse:

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
        printf("Unable to create HID device. May be fine if created previously.\n");
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
```

#### Run it

You can found the previous example [here](examples/mouse.c).
Compile it and run it with:

```bash
curl -O https://raw.githubusercontent.com/unbit/foohid/develop/examples/mouse.c
gcc mouse.c -o virtual_mouse -framework IOKit
./virtual_mouse
```

### Other examples

A few other examples may be found within the [examples directory](examples).
One that is worth noticing is the [keyboard example](examples/keyboard.c).

## The `IOUserClient` API specification

4 methods are exposed:

* `CREATE` (selector 0)
* `DESTROY` (selector 1)
* `SEND` (selector 2)
* `LIST` (selector 3)

### Create
Creates a new fake/virtual HID device with the specified `report_descriptor`.

Takes 8 input arguments:

1. name pointer.
2. name length.
3. `report_descriptor` pointer.
4. `report_descriptor` length.
5. device serial number pointer.
6. device serial number pointer length.
7. device vendor ID.
8. device product ID.

And doesn't output any argument.

### Destroy

Takes 2 input arguments:

1. name pointer.
2. name length.

And doesn't output any argument.

### Send

Generate a HID event from a previously created fake/virtual HID device.

Takes 4 input arguments:

1. name pointer.
2. name length.
3. `report_descriptor` pointer.
4. `report_descriptor` length.

And doesn't output any argument.

### List

Return (into the supplied buffer pointer) the list of available fake/virtual devices separated by `\0`. 
The items output value contains the number of returned items. 
If the supplied buffer is not big enough, the needed bytes value contains a suggestion for a second run.

Takes 2 input arguments:

1. buffer pointer.
2. buffer length.

And 2 output arguments:

1. needed bytes (suggestion for next run)
2. returned items.

## Logging

Logging is disabled by default.
You can enable it by building a DEBUG version by using XCode, or manually setting the `-DDEBUG` preprocessor flag.
