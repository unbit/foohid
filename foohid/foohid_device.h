#include "IOKit/hid/IOHIDDevice.h"

class it_unbit_foohid_device : public IOHIDDevice {
    OSDeclareDefaultStructors(it_unbit_foohid_device)
    
public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual bool init(
                      OSDictionary *dictionary = 0,
                      uint32_t serialNumber = 0,
                      uint32_t vendorID = 0,
                      uint32_t productID = 0
                      );
    virtual void free(void) override;
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    virtual OSString* newProductString() const override;
    
    virtual OSNumber* newSerialNumber() const override;
    virtual OSNumber* newVendorIDNumber() const override;
    virtual OSNumber* newProductIDNumber() const override;
    
    unsigned char *reportDescriptor;
    UInt16 reportDescriptor_len;
    
    OSString *name;
    
    bool isMouse;
    
private:
    uint32_t m_SerialNumber;
    uint32_t m_VendorID;
    uint32_t m_ProductID;
};