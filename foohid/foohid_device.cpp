#include <IOKit/IOLib.h>
#include "foohid_device.h"
#include "debug.h"

#define super IOHIDDevice
OSDefineMetaClassAndStructors(it_unbit_foohid_device, IOHIDDevice)

bool it_unbit_foohid_device::init(
                                  OSDictionary* dict,
                                  uint32_t serialNumber,
                                  uint32_t vendorID,
                                  uint32_t productID
                                  ) {
    LogD("initializing a new virtual hid device\n");
    
    m_SerialNumber = serialNumber;
    m_VendorID = vendorID;
    m_ProductID = productID;
    
    if (!super::init(dict)) return false;
    
    if (isMouse)
        setProperty("HIDDefaultBehavior", OSString::withCString("Mouse"));
    
    return true;
}

bool it_unbit_foohid_device::init(OSDictionary* dict) {
    return it_unbit_foohid_device::init(dict, 0, 0);
}

bool it_unbit_foohid_device::start(IOService *provider) {
    LogD("starting hid device\n");
    return super::start(provider);
}

void it_unbit_foohid_device::stop(IOService *provider) {
    LogD("stopping hid device\n");
    super::stop(provider);
}

void it_unbit_foohid_device::free() {
    LogD("free hid device\n");
    if (reportDescriptor) {
        IOFree(reportDescriptor, reportDescriptor_len);
    }
    if (name) {
        name->release();
    }
    super::free();
}

IOReturn it_unbit_foohid_device::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    LogD("it_unbit_foohid_device::newReportDescriptor()\n");
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, reportDescriptor_len);
    if (buffer == NULL) {
        LogD("OOOOPS");
        return kIOReturnNoResources;
    }
    
    buffer->writeBytes(0, reportDescriptor, reportDescriptor_len);
    *descriptor = buffer;
    
    LogD("all fine\n");
    
    return kIOReturnSuccess;
}

OSString* it_unbit_foohid_device::newProductString() const {
    return OSString::withString(name);
}

OSNumber* it_unbit_foohid_device::newSerialNumber() const {
    return OSNumber::withNumber(m_SerialNumber, 32);
}

OSNumber *it_unbit_foohid_device::newVendorIDNumber() const {
    return OSNumber::withNumber(m_VendorID, 32);
}

OSNumber *it_unbit_foohid_device::newProductIDNumber() const {
    return OSNumber::withNumber(m_ProductID, 32);
}