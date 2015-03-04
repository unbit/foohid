


#include <IOKit/IOService.h>

class it_unbit_foohid : public IOService {
    OSDeclareDefaultStructors(it_unbit_foohid)
    
private:
    
    OSDictionary *hid_devices;
    
public:
    
    virtual bool init(OSDictionary* dictionary = 0);
    virtual void free(void);
    virtual bool start(IOService* provider);
    virtual void stop(IOService* provider);
    
    virtual bool methodCreate(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len);
    virtual bool methodDestroy(char *name, UInt8 name_len);
    virtual bool methodSend(char *name, UInt8 name_len, unsigned char *report_descriptor, UInt16 report_descriptor_len);
    virtual bool methodList(char *buf, UInt16 buf_len, UInt16 *needed, UInt16 *items);
    
};

