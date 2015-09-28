#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>

#include "foohid_userclient.h"
#include <string.h>

OSDefineMetaClassAndStructors(it_unbit_foohid_userclient, IOUserClient)
#define super IOUserClient

bool it_unbit_foohid_userclient::initWithTask(task_t owningTask, void* securityToken, UInt32 type, OSDictionary* properties) {
    IOLog("it_unbit_foohid_userclient::initWithTask()\n");
    owner = owningTask;
    bool ret = super::initWithTask(owningTask, securityToken, type, properties);
    return ret;
}

bool it_unbit_foohid_userclient::start(IOService* provider) {
    IOLog("it_unbit_foohid_userclient::start()\n");
    hid_provider = OSDynamicCast(it_unbit_foohid, provider);
    if (hid_provider == NULL) {
        return false;
    }
    return super::start(provider);
}

void it_unbit_foohid_userclient::stop(IOService *provider) {
    IOLog("it_unbit_foohid_userclient::stop()\n");
    super::stop(provider);
}

const IOExternalMethodDispatch it_unbit_foohid_userclient::methods[it_unbit_foohid_method_last] = {
    {
        (IOExternalMethodAction) &it_unbit_foohid_userclient::methodCreate,	// Method pointer.
        7,																		// No scalar input values.
        0,																		// No struct input value.
        1,																		// No scalar output values.
        0																		// No struct output value.
    },
    {
        (IOExternalMethodAction) &it_unbit_foohid_userclient::methodDestroy,	// Method pointer.
        2,																		// No scalar input values.
        0,																		// No struct input value.
        1,																		// No scalar output values.
        0																		// No struct output value.
    },
    {
        (IOExternalMethodAction) &it_unbit_foohid_userclient::methodSend,	// Method pointer.
        4,																		// No scalar input values.
        0,																		// No struct input value.
        1,																		// No scalar output values.
        0																		// No struct output value.
    },
    {
        (IOExternalMethodAction) &it_unbit_foohid_userclient::methodList,	// Method pointer.
        2,																		// No scalar input values.
        0,																		// No struct input value.
        2,																		// No scalar output values.
        0																		// No struct output value.
    },
};

IOReturn it_unbit_foohid_userclient::externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
                                                   IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) {
    IOLog("it_unbit_foohid_userclient calling %d\n", selector);
    if (selector < it_unbit_foohid_method_last) {
        dispatch = (IOExternalMethodDispatch *) &methods[selector];
        target = this;
    }
    return super::externalMethod(selector, arguments, dispatch, target, reference);
}

IOReturn it_unbit_foohid_userclient::methodCreate(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments) {
    IOLog("ready to call\n");
    return target->_methodCreate(arguments);
}

IOReturn it_unbit_foohid_userclient::methodDestroy(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments) {
    IOLog("ready to call\n");
    return target->_methodDestroy(arguments);
}

IOReturn it_unbit_foohid_userclient::methodSend(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments) {
    IOLog("ready to call\n");
    return target->_methodSend(arguments);
}

IOReturn it_unbit_foohid_userclient::methodList(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments) {
    IOLog("ready to call\n");
    return target->_methodList(arguments);
}


IOReturn it_unbit_foohid_userclient::_methodDestroy(IOExternalMethodArguments* arguments) {
    
    IOMemoryDescriptor *user_buf = NULL;
 
    IOMemoryMap *map = NULL;
    
    
    char *ptr = NULL;
    
    bool ret = false;
    
    UInt8 *name_ptr = (UInt8 *) arguments->scalarInput[0];
    UInt8 name_len = (UInt8) arguments->scalarInput[1];
   
    
    user_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) name_ptr, name_len, kIODirectionOut, owner);
    if (!user_buf) goto nomem;
    if (user_buf->prepare() != kIOReturnSuccess) goto nomem;
    
      map = user_buf->map();
    if (!map) goto nomem;
    
  
    
    ptr = (char *) map->getAddress();
    if (!ptr) goto nomem;
      ret = hid_provider->methodDestroy(ptr, name_len);
    
    user_buf->complete();
    
    map->release();
    
    user_buf->release();
    if (ret) {
        arguments->scalarOutput[0] = 0;
        return kIOReturnSuccess;
    }
    return kIOReturnDeviceError;
    
nomem:
    if (map) map->release();
    if (user_buf) user_buf->release();
    return kIOReturnNoMemory;
}

IOReturn it_unbit_foohid_userclient::_methodList(IOExternalMethodArguments* arguments) {
    
    IOLog("listing devices\n");
    
    IOMemoryDescriptor *user_buf = NULL;
    
    IOMemoryMap *map = NULL;
    
    UInt16 needed = 0, items = 0;
    
    
    char *ptr = NULL;
    bool ret = false;
    
    UInt8 *name_ptr = (UInt8 *) arguments->scalarInput[0];
    UInt16 name_len = (UInt16) arguments->scalarInput[1];
    
    user_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) name_ptr, name_len, kIODirectionIn, owner);
    if (!user_buf) goto nomem;
    if (user_buf->prepare() != kIOReturnSuccess) goto nomem;
    
    map = user_buf->map();
    if (!map) goto nomem;
    
    ptr = (char *) map->getAddress();
    if (!ptr) goto nomem;
    
    ret = hid_provider->methodList(ptr, name_len, &needed, &items);
    
    user_buf->complete();
    
    map->release();
    
    user_buf->release();
    if (ret) {
        arguments->scalarOutput[0] = needed;
        arguments->scalarOutput[1] = items;
        return kIOReturnSuccess;
    }
    return kIOReturnDeviceError;
    
nomem:
    if (map) map->release();
    if (user_buf) user_buf->release();
    return kIOReturnNoMemory;
}


IOReturn it_unbit_foohid_userclient::_methodCreate(IOExternalMethodArguments* arguments) {
    
    IOMemoryDescriptor *user_buf = NULL;
    IOMemoryDescriptor *descriptor_buf = NULL;
    IOMemoryMap *map = NULL;
    IOMemoryMap *map2 = NULL;
    
    char *ptr = NULL;
    unsigned char *ptr2 = NULL;
    
    bool ret = false;
    
    UInt8 *name_ptr = (UInt8 *) arguments->scalarInput[0];
    UInt8 name_len = (UInt8) arguments->scalarInput[1];
    UInt8 *descriptor_ptr = (UInt8 *) arguments->scalarInput[2];
    UInt16 descriptor_len = (UInt16) arguments->scalarInput[3];
    
    UInt32 serialNumber = (UInt32) arguments->scalarInput[4];
    UInt32 vendorID = (UInt32) arguments->scalarInput[5];
    UInt32 productID = (UInt32) arguments->scalarInput[6];
    
    user_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) name_ptr, name_len, kIODirectionOut, owner);
    if (!user_buf) goto nomem;
    if (user_buf->prepare() != kIOReturnSuccess) goto nomem;
    
    descriptor_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) descriptor_ptr, descriptor_len, kIODirectionOut, owner);
    if (!descriptor_buf) goto nomem;
    if (descriptor_buf->prepare() != kIOReturnSuccess) goto nomem;
    
    
    map = user_buf->map();
    if (!map) goto nomem;
    
    map2 = descriptor_buf->map();
    if (!map2) goto nomem;
    
    ptr = (char *) map->getAddress();
    if (!ptr) goto nomem;
    
    ptr2 = (unsigned char *) map2->getAddress();
    if (!ptr2) goto nomem;
    
    ret = hid_provider->methodCreate(ptr, name_len, ptr2, descriptor_len, serialNumber, vendorID, productID);
    
    user_buf->complete();
    descriptor_buf->complete();
    
    map->release();
    map2->release();
    
    user_buf->release();
    descriptor_buf->release();
    
    if (ret) {
        arguments->scalarOutput[0] = 0;
        return kIOReturnSuccess;
    }
    return kIOReturnDeviceError;

nomem:
    if (map) map->release();
    if (map2) map2->release();
    if (user_buf) user_buf->release();
    if (descriptor_buf) descriptor_buf->release();
    return kIOReturnNoMemory;
}

IOReturn it_unbit_foohid_userclient::_methodSend(IOExternalMethodArguments* arguments) {
    
    IOMemoryDescriptor *user_buf = NULL;
    IOMemoryDescriptor *descriptor_buf = NULL;
    IOMemoryMap *map = NULL;
    IOMemoryMap *map2 = NULL;
    
    char *ptr = NULL;
    unsigned char *ptr2 = NULL;
    
    bool ret = false;
    
    UInt8 *name_ptr = (UInt8 *) arguments->scalarInput[0];
    UInt8 name_len = (UInt8) arguments->scalarInput[1];
    UInt8 *descriptor_ptr = (UInt8 *) arguments->scalarInput[2];
    UInt16 descriptor_len = (UInt16) arguments->scalarInput[3];
    
    user_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) name_ptr, name_len, kIODirectionOut, owner);
    if (!user_buf) goto nomem;
    if (user_buf->prepare() != kIOReturnSuccess) goto nomem;
    
    descriptor_buf = IOMemoryDescriptor::withAddressRange((vm_address_t) descriptor_ptr, descriptor_len, kIODirectionOut, owner);
    if (!descriptor_buf) goto nomem;
    if (descriptor_buf->prepare() != kIOReturnSuccess) goto nomem;
    
    
    map = user_buf->map();
    if (!map) goto nomem;
    
    map2 = descriptor_buf->map();
    if (!map2) goto nomem;
    
    ptr = (char *) map->getAddress();
    if (!ptr) goto nomem;
    
    ptr2 = (unsigned char *) map2->getAddress();
    if (!ptr2) goto nomem;
    
    ret = hid_provider->methodSend(ptr, name_len, ptr2, descriptor_len);
    
    user_buf->complete();
    descriptor_buf->complete();
    
    map->release();
    map2->release();
    
    user_buf->release();
    descriptor_buf->release();
    
    if (ret) {
        arguments->scalarOutput[0] = 0;
        return kIOReturnSuccess;
    }
    return kIOReturnDeviceError;
    
nomem:
    if (map) map->release();
    if (map2) map2->release();
    if (user_buf) user_buf->release();
    if (descriptor_buf) descriptor_buf->release();
    return kIOReturnNoMemory;
}