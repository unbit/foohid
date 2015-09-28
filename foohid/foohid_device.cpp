#include <IOKit/IOLib.h>
#include "foohid_device.h"

#define super IOHIDDevice
OSDefineMetaClassAndStructors(it_unbit_foohid_device, IOHIDDevice)

bool it_unbit_foohid_device::init(OSDictionary* dict) {
    IOLog("initializing a new virtual hid device\n");
    
    if (!super::init(dict)) return false;
    
    if (isMouse)
        setProperty("HIDDefaultBehavior", OSString::withCString("Mouse"));
    
    return true;
}

bool it_unbit_foohid_device::start(IOService *provider) {
    IOLog("starting hid device\n");
    return super::start(provider);
}

void it_unbit_foohid_device::stop(IOService *provider) {
    IOLog("stopping hid device\n");
    super::stop(provider);
}

void it_unbit_foohid_device::free() {
    IOLog("free hid device\n");
    if (reportDescriptor) {
        IOFree(reportDescriptor, reportDescriptor_len);
    }
    if (name) {
        name->release();
    }
    super::free();
}

IOReturn it_unbit_foohid_device::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOLog("it_unbit_foohid_device::newReportDescriptor()\n");
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, reportDescriptor_len);
    if (buffer == NULL) {
        IOLog("OOOOPS");
        return kIOReturnNoResources;
    }
    
    buffer->writeBytes(0, reportDescriptor, reportDescriptor_len);
    *descriptor = buffer;
    
    IOLog("all fine\n");
    
    return kIOReturnSuccess;
}

OSString* it_unbit_foohid_device::newProductString() const {
    return OSString::withString(name);
}