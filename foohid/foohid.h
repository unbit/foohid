#ifndef foohid_h
#define foohid_h

#include <IOKit/IOService.h>

class it_unbit_foohid : public IOService {
    OSDeclareDefaultStructors(it_unbit_foohid)
    
public:
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    /**
     *  Create a new virtual device.
     *
     *  @param name                  A unique device name.
     *  @param name_len              Length of 'name'.
     *  @param report_descriptor     A report descriptor for this device.
     *  @param report_descriptor_len Length of 'report_descriptor'.
     *  @param serial_number         A serial number for the device.
     *  @param serial_number_len     Length of 'serial_number'
     *  @param vendor_id             A vendor ID.
     *  @param product_id            A product ID.
     *
     *  @return True on success.
     */
    virtual bool methodCreate(char *name, UInt8 name_len,
                              unsigned char *report_descriptor, UInt16 report_descriptor_len,
                              char *serial_number = nullptr, UInt16 serial_number_len = 0,
                              UInt32 vendor_id = 0, UInt32 product_id = 0);
    
    /**
     *  Destroy a given device.
     *
     *  @param name     A unique device name.
     *  @param name_len Length of 'name'.
     *
     *  @return True on success.
     */
    virtual bool methodDestroy(char *name, UInt8 name_len);
    
    /**
     *  Send a report descriptor to the device.
     *
     *  @param name                  A unique device name.
     *  @param name_len              Length of 'name'.
     *  @param report_descriptor     A report descriptor for this device.
     *  @param report_descriptor_len Length of 'report_descriptor'.
     *
     *  @return True on success.
     */
    virtual bool methodSend(char *name, UInt8 name_len,
                            unsigned char *report_descriptor,
                            UInt16 report_descriptor_len);
    
    /**
     *  Return the names of the currently managed virtual devices, 
     *  separated by '\x00'.
     *
     *  @param buf     A buffer in which to store the virtual devices' names.
     *  @param buf_len Length of 'buf'.
     *  @param needed  A suggestion for the next size of 'buf', if currenly insufficient.
     *  @param items   The number of returned items.
     *
     *  @return True on success, False on insufficent buffer size.
     */
    virtual bool methodList(char *buf, UInt16 buf_len,
                            UInt16 *needed, UInt16 *items);

    /**
     *  Subscribe userclient to setReport calls on the given device.
     *
     *  @param name       A unique device name.
     *  @param name_len   Length of 'name'.
     *  @param userClient UserClient that is subscribing.
     *
     *  @return True on success.
     */
    virtual bool methodSubscribe(char *name, UInt8 name_len, IOService *userClient);

private:
    /**
     *  Keep track of managed/created HID devices.
     */
    OSDictionary *m_hid_devices;
};

#endif
