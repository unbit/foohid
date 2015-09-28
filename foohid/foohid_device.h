#include "IOKit/hid/IOHIDDevice.h"

class it_unbit_foohid_device : public IOHIDDevice {
    OSDeclareDefaultStructors(it_unbit_foohid_device)
    
public:
    virtual void free(void) override;
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    virtual OSString* newProductString() const override;
    
    unsigned char *reportDescriptor;
    UInt16 reportDescriptor_len;
    
    OSString *name;
    
    bool isMouse;
};