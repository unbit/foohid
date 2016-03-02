#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>

#include "foohid.h"

/**
 The goal of this User Client is to expose to user space the following selector.
*/
enum {
    it_unbit_foohid_method_create,
    it_unbit_foohid_method_destroy,
    it_unbit_foohid_method_send,
    it_unbit_foohid_method_list,
    
    it_unbit_foohid_method_count  // Keep track of the length of this enum.
};

class it_unbit_foohid_userclient : public IOUserClient {
    OSDeclareDefaultStructors(it_unbit_foohid_userclient);
    
public:
    virtual bool initWithTask(task_t owningTask, void *securityToken,
                              UInt32 type, OSDictionary *properties) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn externalMethod(uint32_t selector,
                                    IOExternalMethodArguments *arguments,
                                    IOExternalMethodDispatch *dispatch,
                                    OSObject *target, void *reference) override;
    
protected:
    /**
     * The following methods unpack/handle the given arguments and 
     * call the related driver method.
     */
    virtual IOReturn methodCreate(IOExternalMethodArguments *arguments);
    virtual IOReturn methodDestroy(IOExternalMethodArguments *arguments);
    virtual IOReturn methodSend(IOExternalMethodArguments *arguments);
    virtual IOReturn methodList(IOExternalMethodArguments *arguments);
    
    /**
     *  The following static methods redirect the call to the 'target' instance.
     */
    static IOReturn sMethodCreate(it_unbit_foohid_userclient *target,
                                 void *reference,
                                 IOExternalMethodArguments *arguments);
    static IOReturn sMethodDestroy(it_unbit_foohid_userclient *target,
                                  void *reference,
                                  IOExternalMethodArguments *arguments);
    static IOReturn sMethodSend(it_unbit_foohid_userclient *target,
                               void *reference,
                               IOExternalMethodArguments *arguments);
    static IOReturn sMethodList(it_unbit_foohid_userclient *target,
                               void *reference,
                               IOExternalMethodArguments *arguments);
    
private:
    /**
     *  Method dispatch table.
     */
    static const IOExternalMethodDispatch s_methods[it_unbit_foohid_method_count];
    
    /**
     *  Driver provider.
     */
    it_unbit_foohid *m_hid_provider;
    
    /**
     *  Task owner.
     */
    task_t m_owner;
};