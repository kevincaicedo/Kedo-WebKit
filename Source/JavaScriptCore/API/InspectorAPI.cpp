#include "config.h"
#include "InspectorAPI.h"
#include "JSContextRefInternal.h"
#include "APICast.h"
#include <inspector/InspectorFrontendChannel.h>
#include <memory>

#include "JSAPIGlobalObject.h"
#include "JSGlobalObject.h"
#include "JSGlobalObjectDebuggable.h"
#include "JSCInlines.h"
#include "JSLock.h"
#include "JSRemoteInspector.h"
#include "JSRemoteInspectorServer.h"

#include <cstdio>
#include <inspector/JSGlobalObjectInspectorController.h>
#include <inspector/remote/RemoteInspector.h>
#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace Inspector {

class RustFrontendChannel final : public FrontendChannel {
public:
    explicit RustFrontendChannel(JSC::JSAPIGlobalObject& global)
        : m_global(global)
    {
    }
    virtual ~RustFrontendChannel() = default;

private:
    FrontendChannel::ConnectionType connectionType() const override { return FrontendChannel::ConnectionType::Remote; }
    void sendMessageToFrontend(const WTF::String& message) override
    {
        if (auto callback = m_global.inspectorCallback()) {
            auto utf8 = message.utf8();
            callback(utf8.data(), utf8.length());
        }
    }
    JSC::JSAPIGlobalObject& m_global;
};

} // namespace Inspector

extern "C" {

void JSInspectorSetCallback(JSGlobalContextRef context, InspectorMessageCallback callback)
{
    if (!context)
        return;

    JSC::JSGlobalObject* globalObject = toJS(context);
    if (!globalObject)
        return;

    JSC::VM& vm = globalObject->vm();
    JSC::JSLockHolder locker(vm);

    JSC::JSAPIGlobalObject* apiGlobal = dynamicDowncast<JSC::JSAPIGlobalObject>(globalObject);
    if (!apiGlobal)
        return;

    apiGlobal->disconnectInspectorFrontend();

    if (callback) {
        apiGlobal->setInspectorCallback(callback);
        auto channel = std::make_unique<Inspector::RustFrontendChannel>(*apiGlobal);
        globalObject->inspectorDebuggable().connect(*channel, false, false);
        apiGlobal->setFrontendChannel(std::move(channel));
        globalObject->setInspectable(true);
    }

    // Note: We intentionally don't call JSRemoteInspectorStart() here because
    // it starts a global remote inspector server that is not needed for direct frontend channel communication.
}

void JSInspectorSendMessage(JSGlobalContextRef context, const char* message)
{
    if (!context || !message)
        return;

    JSC::JSGlobalObject* globalObject = toJS(context);
    if (!globalObject)
        return;

    JSC::VM& vm = globalObject->vm();
    JSC::JSLockHolder locker(vm);

    JSC::JSAPIGlobalObject* apiGlobal = dynamicDowncast<JSC::JSAPIGlobalObject>(globalObject);
    if (!apiGlobal)
        return;

    globalObject->inspectorDebuggable().dispatchMessageFromRemote(WTF::String::fromUTF8(message));
}

void JSInspectorDisconnect(JSGlobalContextRef context)
{
    if (!context)
        return;

    JSC::JSGlobalObject* globalObject = toJS(context);
    if (!globalObject)
        return;

    JSC::VM& vm = globalObject->vm();
    JSC::JSLockHolder locker(vm);

    JSC::JSAPIGlobalObject* apiGlobal = dynamicDowncast<JSC::JSAPIGlobalObject>(globalObject);
    if (!apiGlobal)
        return;

    apiGlobal->disconnectInspectorFrontend();
}

bool JSInspectorIsConnected(JSGlobalContextRef context)
{
    if (!context)
        return false;

    JSC::JSGlobalObject* globalObject = toJS(context);
    if (!globalObject)
        return false;

    JSC::VM& vm = globalObject->vm();
    JSC::JSLockHolder locker(vm);

    JSC::JSAPIGlobalObject* apiGlobal = dynamicDowncast<JSC::JSAPIGlobalObject>(globalObject);
    if (!apiGlobal)
        return false;

    return apiGlobal->frontendChannel() != nullptr;
}

void JSInspectorSetPauseEventCallback(
    JSGlobalContextRef context,
    InspectorPauseEventCallback callback
) {
    if (!context)
        return;

    JSC::JSGlobalObject* globalObject = toJS(context);
    if (!globalObject)
        return;

    JSC::VM& vm = globalObject->vm();
    JSC::JSLockHolder locker(vm);

    JSC::JSAPIGlobalObject* apiGlobal = dynamicDowncast<JSC::JSAPIGlobalObject>(globalObject);
    if (!apiGlobal)
        return;

    apiGlobal->setPauseEventCallback(callback);
}

} // extern "C"
