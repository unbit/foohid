
#include "IOKit/hid/IOHIDDevice.h"

class it_unbit_foohid_device : public IOHIDDevice {
    OSDeclareDefaultStructors(it_unbit_foohid_device)
    
public:
    virtual void free(void);
    virtual bool init(OSDictionary *dictionary = 0);
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const;
    virtual OSString* newProductString() const;
    
    unsigned char *reportDescriptor;
    UInt16 reportDescriptor_len;
    
    OSString *name;
    
    bool isMouse;
    
};