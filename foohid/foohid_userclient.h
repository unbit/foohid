#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

#include "foohid.h"

/*
 
    int foohid_create(name, report_descriptor, report_descriptor_len);
    int foohid_destroy(name);
    int foohid_list(char **, *int);
 
 */

enum {
    it_unbit_foohid_method_create,
    it_unbit_foohid_method_destroy,
    it_unbit_foohid_method_send,
    it_unbit_foohid_method_list,
    it_unbit_foohid_method_last
};

class it_unbit_foohid_userclient : public IOUserClient {
    OSDeclareDefaultStructors(it_unbit_foohid_userclient);
    
protected:
    it_unbit_foohid *hid_provider;
    static const IOExternalMethodDispatch methods[it_unbit_foohid_method_last];
    
public:
    
    virtual void stop(IOService* provider) override;
    virtual bool start(IOService* provider) override;
    
    virtual bool initWithTask(task_t owningTask, void* securityToken, UInt32 type, OSDictionary* properties) override;
    
protected:
    
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
                                    IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) override;
    
    static IOReturn methodCreate(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments);
    virtual IOReturn _methodCreate(IOExternalMethodArguments* arguments);
    
    static IOReturn methodDestroy(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments);
    virtual IOReturn _methodDestroy(IOExternalMethodArguments* arguments);
    
    static IOReturn methodSend(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments);
    virtual IOReturn _methodSend(IOExternalMethodArguments* arguments);
    
    static IOReturn methodList(it_unbit_foohid_userclient *target, void* reference, IOExternalMethodArguments* arguments);
    virtual IOReturn _methodList(IOExternalMethodArguments* arguments);

    
private:
    
    task_t owner;
};