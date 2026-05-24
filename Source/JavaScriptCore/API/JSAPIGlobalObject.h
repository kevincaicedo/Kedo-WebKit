/**
 * Copyright (C) 2019-2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "InspectorAPI.h"
#include "JSBase.h"
#include "JSGlobalObject.h"
#include <inspector/InspectorFrontendChannel.h>
#include <wtf/RefPtr.h>
#include <memory>
#include <utility>

namespace Inspector { class FrontendChannel; }

OBJC_CLASS JSScript;

namespace JSC {

class ScriptFetcher;
class ScriptFetchParameters;

class JSAPIGlobalObject final : public JSGlobalObject {
public:
    using Base = JSGlobalObject;

    DECLARE_EXPORT_INFO;

    static constexpr DestructionMode needsDestruction = NeedsDestruction;
    template<typename CellType, SubspaceAccess mode>
    static GCClient::IsoSubspace* subspaceFor(VM& vm)
    {
        return vm.apiGlobalObjectSpace<mode>();
    }

    static JSAPIGlobalObject* create(VM&, Structure*);
    static Structure* createStructure(VM&, JSValue prototype);
    static void destroy(JSCell*);

    static void reportUncaughtExceptionAtEventLoop(JSGlobalObject*, Exception*);

    ~JSAPIGlobalObject();

    JSValue loadAndEvaluateJSScriptModule(const JSLockHolder&, JSScript*);

    void setAPIModuleLoader(JSAPIModuleLoader moduleLoader) { m_apiModuleLoader = moduleLoader; }
    const JSAPIModuleLoader& apiModuleLoader() const { return m_apiModuleLoader; }

    bool hasAPIModuleLoaderResolve() const { return !!m_apiModuleLoader.moduleLoaderResolve; }
    bool hasAPIModuleLoaderEvaluate() const { return !!m_apiModuleLoader.moduleLoaderEvaluate; }
    bool hasAPIModuleLoaderFetch() const { return !!m_apiModuleLoader.moduleLoaderFetch; }
    bool hasAPIModuleLoaderFetchSource() const { return !!m_apiModuleLoader.moduleLoaderFetchSource; }
    bool hasAPIModuleLoaderCreateImportMetaProperties() const { return !!m_apiModuleLoader.moduleLoaderCreateImportMetaProperties; }

    void setUncaughtExceptionAtEventLoop(JSUncaughtExceptionAtEventLoop callback) { m_uncaughtExceptionAtEventLoop = callback; }
    JSUncaughtExceptionAtEventLoop uncaughtExceptionAtEventLoop() const { return m_uncaughtExceptionAtEventLoop; }

    void setUncaughtExceptionHandler(JSUncaughtExceptionHandler handler) { m_uncaughtExceptionHandler = handler; }
    JSUncaughtExceptionHandler uncaughtExceptionHandler() const { return m_uncaughtExceptionHandler; }

    void setInspectorCallback(InspectorMessageCallback callback) { m_inspectorCallback = callback; }
    InspectorMessageCallback inspectorCallback() const { return m_inspectorCallback; }

    void setFrontendChannel(std::unique_ptr<Inspector::FrontendChannel> channel) { m_frontendChannel = std::move(channel); }
    Inspector::FrontendChannel* frontendChannel() const { return m_frontendChannel.get(); }
    void clearFrontendChannel() { m_frontendChannel = nullptr; }
    void disconnectInspectorFrontend();

    void setPauseEventCallback(InspectorPauseEventCallback callback) { m_pauseEventCallback = callback; }
    InspectorPauseEventCallback pauseEventCallback() const { return m_pauseEventCallback; }

private:
    static const GlobalObjectMethodTable* globalObjectMethodTable();
    JSAPIGlobalObject(VM&, Structure*);

    static JSPromise* moduleLoaderImportModule(JSGlobalObject*, JSModuleLoader*, JSString* moduleNameValue, RefPtr<ScriptFetchParameters>, const SourceOrigin&);
    static Identifier moduleLoaderResolve(JSGlobalObject*, JSModuleLoader*, JSValue keyValue, JSValue referrerValue, RefPtr<ScriptFetcher>, bool useImportMap);
    static JSPromise* moduleLoaderFetch(JSGlobalObject*, JSModuleLoader*, JSValue, RefPtr<ScriptFetchParameters>, RefPtr<ScriptFetcher>);
    static JSObject* moduleLoaderCreateImportMetaProperties(JSGlobalObject*, JSModuleLoader*, JSValue, JSModuleRecord*, RefPtr<ScriptFetcher>);
    static JSValue moduleLoaderEvaluate(JSGlobalObject*, JSModuleLoader*, JSValue, JSValue, RefPtr<ScriptFetcher>, JSValue, JSValue);

    JSAPIModuleLoader m_apiModuleLoader { };
    JSUncaughtExceptionAtEventLoop m_uncaughtExceptionAtEventLoop { nullptr };
    JSUncaughtExceptionHandler m_uncaughtExceptionHandler { nullptr };

    InspectorMessageCallback m_inspectorCallback { nullptr };
    std::unique_ptr<Inspector::FrontendChannel> m_frontendChannel;

    InspectorPauseEventCallback m_pauseEventCallback { nullptr };
};

}
