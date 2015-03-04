


#include <IOKit/IOLib.h>
#include <libkern/OSMalloc.h>
#include "foohid.h"
#include "foohid_device.h";

#define super IOService

OSDefineMetaClassAndStructors(it_unbit_foohid, IOService);

bool it_unbit_foohid::start(IOService *provider) {
    IOLog("foohid::start\n");
    bool ret = super::start(provider);
    if (ret) {
        IOLog("foohid::registerServicet\n");
        registerService();
        
    }
    return ret;
}

void it_unbit_foohid::stop(IOService *provider) {
    IOLog("foohid::stop\n");
    // here we need to terminate() and release() every
    // managed hid device
    OSCollectionIterator *iter = OSCollectionIterator::withCollection(hid_devices);
    if (iter != NULL) {
        const OSString *key = NULL;
        while((key = (OSString *)iter->getNextObject()) != NULL) {
            it_unbit_foohid_device *device = (it_unbit_foohid_device *) hid_devices->getObject(key);
            if (device) {
                IOLog("terminate device %p\n", device);
                device->terminate();
                device->release();
            }
        }
        iter->release();
    }
    super::stop(provider);
}

bool it_unbit_foohid::init(OSDictionary* dictionary) {
    IOLog("foohid::init\n");
    hid_devices = OSDictionary::withCapacity(1);
    if (!hid_devices) {
        IOLog("foohid::unable to inizialize hid dictionary\n");
        return false;
    }
    return super::init(dictionary);
}

void it_unbit_foohid::free() {
    IOLog("foohid::free\n");
    
    // clear the devices dictionary
    if (hid_devices) {
        hid_devices->release();
    }
    super::free();
}

bool it_unbit_foohid::methodDestroy(char *name, UInt8 name_len) {
    OSString *key = NULL;
    it_unbit_foohid_device *device = NULL;
    
    if (name_len == 0) return false;

    char *cname = (char *) IOMalloc(name_len+1);
    if (!cname) return false;
    memcpy(cname, name, name_len);
    cname[name_len] = 0;
    
    key = OSString::withCString(cname);
    if (!key) goto end;
    
    device = (it_unbit_foohid_device *) hid_devices->getObject(key);
    if (!device) goto end;
    
    device->terminate();
    hid_devices->removeObject(key);
    key->release();
    device->release();
    
    IOFree(cname, name_len+1);
    return true;
    
end:
    IOFree(cname, name_len+1);
    if (key) key->release();
    return false;
}

bool it_unbit_foohid::methodSend(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len) {
    OSString *key = NULL;
    it_unbit_foohid_device *device = NULL;
    IOMemoryDescriptor *report = NULL;
    
    if (name_len == 0) return false;
    
    char *cname = (char *) IOMalloc(name_len+1);
    if (!cname) return false;
    memcpy(cname, name, name_len);
    cname[name_len] = 0;
    
    key = OSString::withCString(cname);
    if (!key) goto end;
    
    device = (it_unbit_foohid_device *) hid_devices->getObject(key);
    if (!device) goto end;
    
    report = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, report_descriptor_len);
    if (!report) goto end;
    
    report->writeBytes(0, report_descriptor, report_descriptor_len);
    
    IOLog("size of report = %d %d %d %d\n", (int) report->getLength(), report_descriptor[0], report_descriptor[1], report_descriptor[2]);
    
    if (device->handleReport(report, kIOHIDReportTypeInput) == kIOReturnSuccess) {
        IOLog("report sent\n");
    }
    
    report->release();
    key->release();
    
    IOFree(cname, name_len+1);
    return true;
    
end:
    IOFree(cname, name_len+1);
    if (key) key->release();
    if (report) report->release();
    return false;
}

bool it_unbit_foohid::methodCreate(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len) {
    
    OSString *key = NULL;
    it_unbit_foohid_device *device = NULL;
    
    if (report_descriptor_len == 0 || name_len == 0) return false;
   
    char *cname = (char *) IOMalloc(name_len+1);
    if (!cname) return false;
    memcpy(cname, name, name_len);
    cname[name_len] = 0;
    
    key = OSString::withCString(cname);
    if (!key) goto end;
    
    // is the device already created ?
    device = (it_unbit_foohid_device *) hid_devices->getObject(OSString::withCString(cname));
    
    if (device != NULL) goto end;
    
    device = OSTypeAlloc(it_unbit_foohid_device);
    if (!device) goto end;
    
    device->isMouse = false;
    
    if (report_descriptor_len >= 4) {
        if (report_descriptor[0] == 0x05 &&
            report_descriptor[1] == 0x01 &&
            report_descriptor[2] == 0x09 &&
            report_descriptor[3] == 0x02) {
            device->isMouse = true;
        }
        
    }
    
    if (!device->init(NULL)) {
        device->release();
        goto end;
    }
    
    device->reportDescriptor = (unsigned char *)IOMalloc(report_descriptor_len);
    if (!device->reportDescriptor) {
        device->release();
        goto end;
    }
    memcpy(device->reportDescriptor, report_descriptor, report_descriptor_len);
    device->reportDescriptor_len = report_descriptor_len;
    
    device->name = OSString::withCString(cname);
    if (!device->name) {
        device->release();
        goto end;
    }
    

    if (!hid_devices->setObject(key, device)) {
        device->release();
        goto end;
    }
    
    IOLog("foohid:: attempt to create a new virtual device: %s\n", cname);

    IOFree(cname, name_len+1);
    key->release();
    
    
    device->attach(this);
    device->start(this);
    
    return true;
    
end:
    IOFree(cname, name_len+1);
    if (key) key->release();
    return false;
    
}


bool it_unbit_foohid::methodList(char *buf, UInt16 buf_len, UInt16 *needed, UInt16 *items) {
    
    if (buf_len == 0) return false;
    
    IOLog("building device list\n");
    
    UInt16 current_len = 0;
    *needed = 0;
    *items = 0;
    
    // managed hid device
    OSCollectionIterator *iter = OSCollectionIterator::withCollection(hid_devices);
    if (iter != NULL) {
        const OSString *key = NULL;
        while((key = (OSString *)iter->getNextObject()) != NULL) {
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