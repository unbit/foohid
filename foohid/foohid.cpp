#include <IOKit/IOLib.h>
#include <libkern/OSMalloc.h>

#include "foohid.h"
#include "foohid_device.h"
#include "debug.h"

#define super IOService
OSDefineMetaClassAndStructors(it_unbit_foohid, IOService);

bool it_unbit_foohid::start(IOService *provider) {
    LogD("Executing 'it_unbit_foohid::start()'.");
    
    bool ret = super::start(provider);
    if (ret) {
        LogD("Calling 'it_unbit_foohid:registerService()'.");
        registerService();
    }
    
    return ret;
}

void it_unbit_foohid::stop(IOService *provider) {
    LogD("Executing 'it_unbit_foohid:stop()'.");
    
    // Terminate and release every managed HID device.
    OSCollectionIterator *iter = OSCollectionIterator::withCollection(m_hid_devices);
    if (iter) {
        const OSString *key = nullptr;
        while ((key = (OSString *)iter->getNextObject())) {
            it_unbit_foohid_device *device = (it_unbit_foohid_device *)m_hid_devices->getObject(key);
            
            if (device) {
                LogD("Terminating device '%s'.", device->name()->getCStringNoCopy());
                device->terminate();
                device->release();
            }
        }
        
        iter->release();
    }
    
    super::stop(provider);
}

bool it_unbit_foohid::init(OSDictionary *dictionary) {
    LogD("Executing 'it_unbit_foohid:init()'.");
    
    m_hid_devices = OSDictionary::withCapacity(1);
    if (!m_hid_devices) {
        LogD("Unable to inizialize HID devices dictionary.");
        return false;
    }
    
    return super::init(dictionary);
}

void it_unbit_foohid::free() {
    LogD("Executing 'it_unbit_foohid:free()'.");
    
    // Clear the HID devices dictionary.
    if (m_hid_devices) {
        m_hid_devices->release();
    }
    
    super::free();
}

bool it_unbit_foohid::methodCreate(char *name, UInt8 name_len,
                                   unsigned char *report_descriptor,
                                   UInt16 report_descriptor_len,
                                   char *serial_number, UInt16 serial_number_len,
                                   UInt32 vendor_id, UInt32 product_id) {
    it_unbit_foohid_device *device = nullptr;
    
    OSString *key = nullptr;
    OSString *serial_number_s = nullptr;
    
    if (report_descriptor_len == 0 || name_len == 0) return false;
    
    {
        char *c_name = (char *)IOMalloc(name_len + 1);
        if (!c_name) return false;
        memcpy(c_name, name, name_len);
        c_name[name_len] = 0;
        key = OSString::withCString(c_name);
        IOFree(c_name, name_len + 1);
    }
    if (!key) return false;
    
    // Has the device already been created?
    device = (it_unbit_foohid_device *)m_hid_devices->getObject(key);
    if (device) return false;
    
    device = OSTypeAlloc(it_unbit_foohid_device);
    if (!device) goto fail;
    
    if (report_descriptor_len >= 4) {
        if (report_descriptor[0] == 0x05 && report_descriptor[1] == 0x01 &&
            report_descriptor[2] == 0x09 && report_descriptor[3] == 0x02) {
            device->isMouse = true;
        } else if (report_descriptor[0] == 0x05 && report_descriptor[1] == 0x01 &&
                   report_descriptor[2] == 0x09 && report_descriptor[3] == 0x06) {
            device->isKeyboard = true;
        }
    }
    
    {
        char *c_serial_number = (char *)IOMalloc(serial_number_len + 1);
        if (!c_serial_number) return false;
        memcpy(c_serial_number, serial_number, serial_number_len);
        c_serial_number[serial_number_len] = 0;
        serial_number_s = OSString::withCString(c_serial_number);
        IOFree(c_serial_number, serial_number_len + 1);
    }
    if (!serial_number_s) goto fail;
    
    device->setSerialNumberString(serial_number_s);
    device->setVendorID(vendor_id);
    device->setProductID(product_id);
    
    LogD("Attempting to init a new virtual device with name: '%s'; "
         "serial number ('%s'); vendor ID (%d); product ID (%d).",
         key->getCStringNoCopy(), serial_number_s->getCStringNoCopy(),
         vendor_id, product_id);
    
    if (!device->init(nullptr)) {
        goto fail;
    }
    
    device->reportDescriptor = (unsigned char *)IOMalloc(report_descriptor_len);
    if (!device->reportDescriptor) {
        goto fail;
    }
    memcpy(device->reportDescriptor, report_descriptor, report_descriptor_len);
    device->reportDescriptor_len = report_descriptor_len;
    
    device->setName(key);
    if (!m_hid_devices->setObject(key, device)) {
        goto fail;
    }
    
    device->attach(this);
    device->start(this);
    
    key->release();
    serial_number_s->release();
    
    return true;
    
fail:
    key->release();
    if (serial_number_s) serial_number_s->release();
    if (device) device->release();
    
    return false;
}

bool it_unbit_foohid::methodDestroy(char *name, UInt8 name_len) {
    OSString *key = nullptr;
    it_unbit_foohid_device *device = nullptr;
    
    if (name_len == 0) return false;
    
    {
        char *cname = (char *)IOMalloc(name_len + 1);
        if (!cname) return false;
        memcpy(cname, name, name_len);
        cname[name_len] = 0;
        key = OSString::withCString(cname);
        IOFree(cname, name_len + 1);
    }
    if (!key) goto end;
    
    device = (it_unbit_foohid_device *)m_hid_devices->getObject(key);
    if (!device) goto end;
    
    device->terminate();
    m_hid_devices->removeObject(key);
    key->release();
    device->release();
    
    return true;
    
end:
    if (key) key->release();
    return false;
}

bool it_unbit_foohid::methodSend(char *name, UInt8 name_len,
                                 unsigned char *report_descriptor,
                                 UInt16 report_descriptor_len) {
    it_unbit_foohid_device *device = nullptr;
    OSString *key = nullptr;
    IOMemoryDescriptor *report = nullptr;
    bool ret = false;
    
    if (name_len == 0) return false;
    
    {
        char *cname = (char *)IOMalloc(name_len + 1);
        if (!cname) return false;
        memcpy(cname, name, name_len);
        cname[name_len] = 0;
        key = OSString::withCString(cname);
        IOFree(cname, name_len + 1);
    }
    if (!key) goto end;
    
    device = (it_unbit_foohid_device *)m_hid_devices->getObject(key);
    if (!device) goto end;
    
    report = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0,
                                                         report_descriptor_len);
    if (!report) goto end;
    
    LogD("Handling report of size: %d.", (int)report->getLength());
    report->writeBytes(0, report_descriptor, report_descriptor_len);
    
    if (device->handleReport(report, kIOHIDReportTypeInput) == kIOReturnSuccess) {
        LogD("Report correctly sent to device.");
        ret = true;
    } else {
        LogD("Error while sending report to device.");
    }
    
    report->release();
    key->release();
    
    return ret;
    
end:
    if (key) key->release();
    if (report) report->release();
    return false;
}


bool it_unbit_foohid::methodList(char *buf, UInt16 buf_len,
                                 UInt16 *needed, UInt16 *items) {
    if (buf_len == 0) return false;
    
    LogD("Building HID virtual devices list.");
    
    UInt16 current_len = 0;
    *needed = 0;
    *items = 0;
    
    // Iterate through managed HID devices.
    OSCollectionIterator *iter = OSCollectionIterator::withCollection(m_hid_devices);
    if (iter) {
        const OSString *key = nullptr;
        while ((key = (OSString *)iter->getNextObject())) {
            UInt8 key_len = key->getLength();
            
            if (key_len + 1 + current_len > buf_len) {
                *needed = buf_len + key_len + 1;
                break;
            }
            
            memcpy(buf + current_len, key->getCStringNoCopy(), key_len);
            buf[current_len + key_len] = 0;
            current_len += key_len + 1;
            (*items)++;
        }
        
        iter->release();
    }
    
    if (*needed != 0) return false;
    return true;
}

bool it_unbit_foohid::methodSubscribe(char *name, UInt8 name_len, IOService *userClient) {
    OSString *key = nullptr;
    it_unbit_foohid_device *device = nullptr;

    if (name_len == 0) return false;

    {
        char *cname = (char *)IOMalloc(name_len + 1);
        if (!cname) return false;
        memcpy(cname, name, name_len);
        cname[name_len] = 0;
        key = OSString::withCString(cname);
        IOFree(cname, name_len + 1);
    }
    if (!key) goto end;

    device = (it_unbit_foohid_device *)m_hid_devices->getObject(key);
    if (!device) goto end;

    device->subscribe(userClient);

    key->release();

    return true;

end:
    if (key) key->release();
    return false;
}
