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

#include "config.h"
#include "JSAPIGlobalObject.h"

#include "APICast.h"
#include "Completion.h"
#include "GlobalObjectMethodTable.h"
#include "JSBaseInternal.h"
#include "JSCellInlines.h"
#include "JSGlobalObjectDebuggable.h"
#include "JSModuleLoader.h"
#include "JSPromise.h"
#include "JSSourceCode.h"
#include "JSString.h"
#include "ObjectConstructor.h"
#include "OpaqueJSString.h"
#include "SourceCode.h"
#include "SourceProvider.h"
#include "StructureCreateInlines.h"

#include <wtf/URL.h>

#if !JSC_OBJC_API_ENABLED

namespace {

static inline JSC::SourceCode apiModuleSource(const String& source, const JSC::SourceOrigin& sourceOrigin, String sourceURL, JSC::SourceProviderSourceType sourceType)
{
    return JSC::SourceCode(JSC::StringSourceProvider::create(source, sourceOrigin, WTF::move(sourceURL), JSC::SourceTaintedOrigin::Untainted, TextPosition(), sourceType));
}

static JSC::JSValue apiImportAttributesValue(JSC::VM& vm, RefPtr<JSC::ScriptFetchParameters> attributes)
{
    if (!attributes)
        return JSC::jsUndefined();

    switch (attributes->type()) {
    case JSC::ScriptFetchParameters::Type::None:
        return JSC::jsUndefined();
    case JSC::ScriptFetchParameters::Type::JavaScript:
        return JSC::jsNontrivialString(vm, "javascript"_s);
    case JSC::ScriptFetchParameters::Type::WebAssembly:
        return JSC::jsNontrivialString(vm, "webassembly"_s);
    case JSC::ScriptFetchParameters::Type::JSON:
        return JSC::jsNontrivialString(vm, "json"_s);
    }

    ASSERT_NOT_REACHED();
    return JSC::jsUndefined();
}

} // namespace

#endif // !JSC_OBJC_API_ENABLED

namespace JSC {

const ClassInfo JSAPIGlobalObject::s_info = { "GlobalObject"_s, &Base::s_info, nullptr, nullptr, CREATE_METHOD_TABLE(JSAPIGlobalObject) };

#if !JSC_OBJC_API_ENABLED

const GlobalObjectMethodTable* JSAPIGlobalObject::globalObjectMethodTable()
{
    static constexpr GlobalObjectMethodTable table {
        &supportsRichSourceInfo,
        &shouldInterruptScript,
        &javaScriptRuntimeFlags,
        &shouldInterruptScriptBeforeTimeout,
        &moduleLoaderImportModule,
        &moduleLoaderResolve,
        &moduleLoaderFetch,
        &moduleLoaderCreateImportMetaProperties,
        &moduleLoaderEvaluate,
        &promiseRejectionTracker,
        &reportUncaughtExceptionAtEventLoop,
        &currentScriptExecutionOwner,
        &scriptExecutionStatus,
        &reportViolationForUnsafeEval,
        nullptr, // defaultLanguage
        nullptr, // compileStreaming
        nullptr, // instantiateStreaming
        nullptr, // deriveShadowRealmGlobalObject
        &codeForEval,
        &canCompileStrings,
        &trustedScriptStructure,
    };
    return &table;
}

void JSAPIGlobalObject::reportUncaughtExceptionAtEventLoop(JSGlobalObject* globalObject, Exception* exception)
{
    auto* globalObjectImpl = dynamicDowncast<JSAPIGlobalObject>(globalObject);
    if (globalObjectImpl) {
        JSContextRef contextRef = toRef(globalObject);
        JSValueRef exceptionRef = toRef(globalObject, exception->value());

        if (auto handler = globalObjectImpl->uncaughtExceptionHandler()) {
            auto filename = OpaqueJSString::create();
            handler(contextRef, filename.ptr(), exceptionRef);
        }

        if (auto callback = globalObjectImpl->uncaughtExceptionAtEventLoop())
            callback(contextRef, exceptionRef);
    }
    Base::reportUncaughtExceptionAtEventLoop(globalObject, exception);
}

#endif // !JSC_OBJC_API_ENABLED

JSAPIGlobalObject::JSAPIGlobalObject(VM& vm, Structure* structure)
    : Base(vm, structure, globalObjectMethodTable())
{
}

JSAPIGlobalObject::~JSAPIGlobalObject()
{
    disconnectInspectorFrontend();
}

void JSAPIGlobalObject::destroy(JSCell* cell)
{
    static_cast<JSAPIGlobalObject*>(cell)->JSAPIGlobalObject::~JSAPIGlobalObject();
}

void JSAPIGlobalObject::disconnectInspectorFrontend()
{
#if ENABLE(REMOTE_INSPECTOR)
    if (auto* channel = frontendChannel())
        inspectorDebuggable().disconnect(*channel);
#endif

    clearFrontendChannel();
    setInspectorCallback(nullptr);
    setPauseEventCallback(nullptr);
    setInspectable(false);
}

JSAPIGlobalObject* JSAPIGlobalObject::create(VM& vm, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<JSAPIGlobalObject>(vm)) JSAPIGlobalObject(vm, structure);
    object->finishCreation(vm);
    return object;
}

Structure* JSAPIGlobalObject::createStructure(VM& vm, JSValue prototype)
{
    auto* result = Structure::create(vm, nullptr, prototype, TypeInfo(GlobalObjectType, StructureFlags), info());
    result->setTransitionWatchpointIsLikelyToBeFired(true);
    return result;
}

#if !JSC_OBJC_API_ENABLED

JSPromise* JSAPIGlobalObject::moduleLoaderImportModule(JSGlobalObject* globalObject, JSModuleLoader*, JSString* moduleNameValue, RefPtr<ScriptFetchParameters> fetchParams, const SourceOrigin& sourceOrigin)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto rejectWithCaughtException = [&]() -> JSPromise* {
        auto* promise = JSPromise::create(vm, globalObject->promiseStructure());
        return promise->rejectWithCaughtException(globalObject, scope);
    };

    auto specifier = moduleNameValue->value(globalObject);
    if (scope.exception()) [[unlikely]]
        return rejectWithCaughtException();

    auto* result = JSC::importModule(globalObject, Identifier::fromString(vm, specifier), Identifier::fromString(vm, sourceOrigin.url().string()), WTF::move(fetchParams), nullptr);
    if (scope.exception()) [[unlikely]]
        return rejectWithCaughtException();

    return result;
}

Identifier JSAPIGlobalObject::moduleLoaderResolve(JSGlobalObject* globalObject, JSModuleLoader*, JSValue keyValue, JSValue referrerValue, RefPtr<ScriptFetcher>, bool)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    const Identifier key = keyValue.toPropertyKey(globalObject);
    RETURN_IF_EXCEPTION(scope, { });

    if (key.isSymbol())
        return key;

    auto* apiGlobalObject = dynamicDowncast<JSAPIGlobalObject>(globalObject);
    if (!apiGlobalObject)
        return key;

    if (!apiGlobalObject->hasAPIModuleLoaderResolve())
        return key;

    JSContextRef contextRef = toRef(globalObject);
    JSStringRef resolved = apiGlobalObject->apiModuleLoader().moduleLoaderResolve(contextRef, toRef(globalObject, keyValue), toRef(globalObject, referrerValue), toRef(globalObject, jsUndefined()));
    if (!resolved) {
        throwTypeError(globalObject, scope, "Module resolver returned null"_s);
        return { };
    }

    Identifier resolvedKey = Identifier::fromString(vm, resolved->string());
    resolved->deref();
    return resolvedKey;
}

JSValue JSAPIGlobalObject::moduleLoaderEvaluate(JSGlobalObject* globalObject, JSModuleLoader* moduleLoader, JSValue key, JSValue moduleRecordValue, RefPtr<ScriptFetcher> scriptFetcher, JSValue sentValue, JSValue resumeMode)
{
    return moduleLoader->evaluateNonVirtual(globalObject, key, moduleRecordValue, WTF::move(scriptFetcher), sentValue, resumeMode);
}

JSPromise* JSAPIGlobalObject::moduleLoaderFetch(JSGlobalObject* globalObject, JSModuleLoader*, JSValue key, RefPtr<ScriptFetchParameters> attributes, RefPtr<ScriptFetcher>)
{
    VM& vm = globalObject->vm();
    JSPromise* promise = JSPromise::create(vm, globalObject->promiseStructure());

    auto scope = DECLARE_THROW_SCOPE(vm);

    auto rejectWithError = [&](JSValue error) {
        promise->reject(vm, globalObject, error);
        return promise;
    };

    String moduleKey = key.toWTFString(globalObject);
    RETURN_IF_EXCEPTION(scope, promise->rejectWithCaughtException(globalObject, scope));

    URL moduleURL({ }, moduleKey);
    auto* apiGlobalObject = dynamicDowncast<JSAPIGlobalObject>(globalObject);
    if (!apiGlobalObject)
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, "API module fetch requested on a non-API global object"_s)));

    JSContextRef contextRef = toRef(globalObject);
    JSValue attributesValue = apiImportAttributesValue(vm, attributes);
    JSModuleSourceRef rawModuleSource = nullptr;

    if (apiGlobalObject->hasAPIModuleLoaderFetchSource()) {
        rawModuleSource = apiGlobalObject->apiModuleLoader().moduleLoaderFetchSource(contextRef, toRef(globalObject, key), toRef(globalObject, attributesValue), toRef(globalObject, jsUndefined()));
        if (scope.exception()) [[unlikely]] {
            JSModuleSourceRelease(rawModuleSource);
            return promise->rejectWithCaughtException(globalObject, scope);
        }
    } else if (apiGlobalObject->hasAPIModuleLoaderFetch()) {
        if (attributes && attributes->type() == ScriptFetchParameters::Type::WebAssembly)
            RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, "Legacy text module fetch callback cannot return WebAssembly module bytes"_s)));

        JSStringRef sourceRef = apiGlobalObject->apiModuleLoader().moduleLoaderFetch(contextRef, toRef(globalObject, key), toRef(globalObject, attributesValue), toRef(globalObject, jsUndefined()));
        if (scope.exception()) [[unlikely]]
            return promise->rejectWithCaughtException(globalObject, scope);

        if (!sourceRef)
            RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, "Module fetcher returned null"_s)));

        rawModuleSource = attributes && attributes->type() == ScriptFetchParameters::Type::JSON
            ? JSModuleSourceCreateJSON(sourceRef)
            : JSModuleSourceCreateJavaScript(sourceRef);
        sourceRef->deref();
    } else
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, makeString("No API module fetch callback registered for module '"_s, moduleKey, "'."_s))));

    std::unique_ptr<OpaqueJSModuleSource, void (*)(JSModuleSourceRef)> moduleSource(rawModuleSource, JSModuleSourceRelease);
    if (!moduleSource)
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, "Module fetcher returned null"_s)));

    JSSourceCode* sourceCode = nullptr;
    switch (moduleSource->type) {
    case OpaqueJSModuleSource::Type::JavaScript:
        sourceCode = JSSourceCode::create(vm, apiModuleSource(moduleSource->source, SourceOrigin { moduleURL }, String { moduleKey }, SourceProviderSourceType::Module));
        break;
    case OpaqueJSModuleSource::Type::JSON:
        sourceCode = JSSourceCode::create(vm, apiModuleSource(moduleSource->source, SourceOrigin { moduleURL }, String { moduleKey }, SourceProviderSourceType::JSON));
        break;
    case OpaqueJSModuleSource::Type::WebAssembly:
#if ENABLE(WEBASSEMBLY)
        sourceCode = JSSourceCode::create(vm, SourceCode(WebAssemblySourceProvider::create(WTF::move(moduleSource->bytes), SourceOrigin { moduleURL }, String { moduleKey })));
#else
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, "WebAssembly modules are not enabled in this JavaScriptCore build"_s)));
#endif
        break;
    }

    if (scope.exception()) [[unlikely]]
        return promise->rejectWithCaughtException(globalObject, scope);

    scope.release();
    promise->resolve(globalObject, vm, sourceCode);
    return promise;
}

JSObject* JSAPIGlobalObject::moduleLoaderCreateImportMetaProperties(JSGlobalObject* globalObject, JSModuleLoader*, JSValue key, JSModuleRecord*, RefPtr<ScriptFetcher>)
{
    VM& vm = globalObject->vm();

    auto* apiGlobalObject = dynamicDowncast<JSAPIGlobalObject>(globalObject);
    if (apiGlobalObject && apiGlobalObject->hasAPIModuleLoaderCreateImportMetaProperties()) {
        JSContextRef contextRef = toRef(globalObject);
        JSObjectRef object = apiGlobalObject->apiModuleLoader().moduleLoaderCreateImportMetaProperties(contextRef, toRef(globalObject, key), toRef(globalObject, jsUndefined()));
        if (object)
            return toJS(object);
    }

    return constructEmptyObject(vm, globalObject->nullPrototypeObjectStructure());
}

#endif // !JSC_OBJC_API_ENABLED

JSValue JSAPIGlobalObject::loadAndEvaluateJSScriptModule(const JSLockHolder&, JSScript *script)
{
    UNUSED_PARAM(script);
    // FIXME: Implement JSScript module evaluation for the C API.
    return jsUndefined();
}

}
