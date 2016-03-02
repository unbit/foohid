#include <IOKit/IOLib.h>
#include "foohid_device.h"
#include "debug.h"

#define super IOHIDDevice
OSDefineMetaClassAndStructors(it_unbit_foohid_device, IOHIDDevice)

bool it_unbit_foohid_device::init(OSDictionary *dict) {
    LogD("Initializing a new virtual HID device.");
    
    if (!super::init(dict)) {
        return false;
    }
    
    if (isMouse) {
        setProperty("HIDDefaultBehavior", "Mouse");
    } else if (isKeyboard) {
        setProperty("HIDDefaultBehavior", "Keyboard");
    }
    
    return true;
}

bool it_unbit_foohid_device::start(IOService *provider) {
    LogD("Executing 'it_unbit_foohid_device::start()'.");
    return super::start(provider);
}

void it_unbit_foohid_device::stop(IOService *provider) {
    LogD("Executing 'it_unbit_foohid_device::stop()'.");
    
    super::stop(provider);
}

void it_unbit_foohid_device::free() {
    LogD("Executing 'it_unbit_foohid_device::free()'.");
    
    if (reportDescriptor) IOFree(reportDescriptor, reportDescriptor_len);
    if (m_name) m_name->release();
    if (m_serial_number_string) m_serial_number_string->release();
    
    super::free();
}

OSString *it_unbit_foohid_device::name() {
    return m_name;
}

void it_unbit_foohid_device::setName(OSString *name) {
    if (name) name->retain();
    m_name = name;
}

void it_unbit_foohid_device::setSerialNumberString(OSString *serialNumberString) {
    if (serialNumberString) {
        serialNumberString->retain();
    }
    
    m_serial_number_string = serialNumberString;
}

void it_unbit_foohid_device::setVendorID(uint32_t vendorID) {
    m_vendor_id = OSNumber::withNumber(vendorID, 32);
}

void it_unbit_foohid_device::setProductID(uint32_t productID) {
    m_product_id = OSNumber::withNumber(productID, 32);
}

IOReturn it_unbit_foohid_device::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    LogD("Executing 'it_unbit_foohid_device::newReportDescriptor()'.");
    IOBufferMemoryDescriptor *buffer =
        IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, reportDescriptor_len);
    
    if (!buffer) {
        LogD("Error while allocating new IOBufferMemoryDescriptor.");
        return kIOReturnNoResources;
    }
    
    buffer->writeBytes(0, reportDescriptor, reportDescriptor_len);
    *descriptor = buffer;
    
    return kIOReturnSuccess;
}

OSString *it_unbit_foohid_device::newProductString() const {
    m_name->retain();
    return m_name;
}

OSString *it_unbit_foohid_device::newSerialNumberString() const {
    m_serial_number_string->retain();
    return m_serial_number_string;
}

OSNumber *it_unbit_foohid_device::newVendorIDNumber() const {
    m_vendor_id->retain();
    return m_vendor_id;
}

OSNumber *it_unbit_foohid_device::newProductIDNumber() const {
    m_product_id->retain();
    return m_product_id;
}