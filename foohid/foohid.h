#include <IOKit/IOService.h>

class it_unbit_foohid : public IOService {
    OSDeclareDefaultStructors(it_unbit_foohid)
    
private:
    
    OSDictionary *hid_devices;
    
public:
    
    virtual bool init(OSDictionary* dictionary = 0) override;
    virtual void free(void) override;
    virtual bool start(IOService* provider) override;
    virtual void stop(IOService* provider) override;
    
    virtual bool methodCreate(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len, char *serialNumber = NULL, UInt16 serialNumber_len = 0, UInt32 vendorID = 0, UInt32 productID = 0);
    virtual bool methodDestroy(char *name, UInt8 name_len);
    virtual bool methodSend(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len);
    virtual bool methodList(char *buf, UInt16 buf_len, UInt16 *needed, UInt16 *items);
};