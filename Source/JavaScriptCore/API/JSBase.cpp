/*
 * Copyright (C) 2006, 2007, 2013, 2016 Apple Inc. All rights reserved.
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
#include "JSBase.h"
#include "JSBaseInternal.h"
#include "JSBasePrivate.h"

#include "APICast.h"
#include "ArgList.h"
#include "Completion.h"
#include "DeferredWorkTimer.h"
#include "GCActivityCallback.h"
#include "JSCInlines.h"
#include "JSAPIGlobalObject.h"
#include "JSLock.h"
#include "JSModuleLoader.h"
#include "JSPromise.h"
#include "ObjectConstructor.h"
#include "OpaqueJSString.h"
#include "ScriptFetchParameters.h"
#include "SourceCode.h"
#include "SyntheticModuleRecord.h"
#include <span>
#include <wtf/HashSet.h>
#include <wtf/StdLibExtras.h>

#if ENABLE(REMOTE_INSPECTOR)
#include "JSGlobalObjectInspectorController.h"
#endif

using namespace JSC;

JSValueRef JSEvaluateScriptInternal(const JSLockHolder&, JSContextRef ctx, JSObjectRef thisObject, const SourceCode& source, JSValueRef* exception)
{
    JSObject* jsThisObject = toJS(thisObject);

    // evaluate sets "this" to the global object if it is NULL
    JSGlobalObject* globalObject = toJS(ctx);
    NakedPtr<Exception> evaluationException;
    JSValue returnValue = profiledEvaluate(globalObject, ProfilingReason::API, source, jsThisObject, evaluationException);

    if (evaluationException) {
        if (exception)
            *exception = toRef(globalObject, evaluationException->value());
#if ENABLE(REMOTE_INSPECTOR)
        // FIXME: If we have a debugger attached we could learn about ParseError exceptions through
        // ScriptDebugServer::sourceParsed and this path could produce a duplicate warning. The
        // Debugger path is currently ignored by inspector.
        // NOTE: If we don't have a debugger, this SourceCode will be forever lost to the inspector.
        // We could stash it in the inspector in case an inspector is ever opened.
        protect(globalObject->inspectorController())->reportAPIException(globalObject, evaluationException);
#endif
        return nullptr;
    }

    if (returnValue)
        return toRef(globalObject, returnValue);

    // happens, for example, when the only statement is an empty (';') statement
    return toRef(globalObject, jsUndefined());
}

JSValueRef JSEvaluateScript(JSContextRef ctx, JSStringRef script, JSObjectRef thisObject, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }
    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    startingLineNumber = std::max(1, startingLineNumber);

    auto sourceURL = sourceURLString ? URL({ }, sourceURLString->string()) : URL();
    SourceCode source = makeSource(script->string(), SourceOrigin { sourceURL }, SourceTaintedOrigin::Untainted, sourceURL.string(), TextPosition(OrdinalNumber::fromOneBasedInt(startingLineNumber), OrdinalNumber()));

    return JSEvaluateScriptInternal(locker, ctx, thisObject, source, exception);
}

namespace {

static bool setCaughtException(JSGlobalObject* globalObject, ThrowScope& scope, JSValueRef* exception)
{
    if (Exception* caughtException = scope.exception()) {
        if (exception)
            *exception = toRef(globalObject, caughtException->value());
        bool didClear = scope.tryClearException();
        ASSERT_UNUSED(didClear, didClear);
        return true;
    }
    return false;
}

static JSValueRef setTypeError(JSGlobalObject* globalObject, JSValueRef* exception, const String& message)
{
    if (exception)
        *exception = toRef(createTypeError(globalObject, message));
    return nullptr;
}

static SourceCode makeAPIModuleSource(JSStringRef module, JSStringRef sourceURLString, int startingLineNumber)
{
    startingLineNumber = std::max(1, startingLineNumber);
    auto sourceURL = sourceURLString ? URL({ }, sourceURLString->string()) : URL();
    return makeSource(module->string(), SourceOrigin { sourceURL }, SourceTaintedOrigin::Untainted, sourceURL.string(), TextPosition(OrdinalNumber::fromOneBasedInt(startingLineNumber), OrdinalNumber()), SourceProviderSourceType::Module);
}

static JSValueRef promiseToRefOrSetException(JSGlobalObject* globalObject, ThrowScope& scope, JSPromise* promise, JSValueRef* exception)
{
    if (setCaughtException(globalObject, scope, exception))
        return nullptr;

    if (!promise)
        return setTypeError(globalObject, exception, "Module operation did not return a promise"_s);

    return toRef(globalObject, promise);
}

} // namespace

JSModuleSourceRef JSModuleSourceCreateJavaScript(JSStringRef source)
{
    if (!source)
        return nullptr;

    return new OpaqueJSModuleSource(OpaqueJSModuleSource::Type::JavaScript, String { source->string() });
}

JSModuleSourceRef JSModuleSourceCreateJSON(JSStringRef source)
{
    if (!source)
        return nullptr;

    return new OpaqueJSModuleSource(OpaqueJSModuleSource::Type::JSON, String { source->string() });
}

JSModuleSourceRef JSModuleSourceCreateWebAssembly(const uint8_t* bytes, size_t byteLength)
{
    if (!bytes && byteLength)
        return nullptr;

    Vector<uint8_t> copiedBytes;
    if (byteLength)
        copiedBytes.append(std::span<const uint8_t> { bytes, byteLength });
    return new OpaqueJSModuleSource(WTF::move(copiedBytes));
}

void JSModuleSourceRelease(JSModuleSourceRef source)
{
    delete source;
}

void JSModuleLoaderSetCallbacks(JSContextRef ctx, JSAPIModuleLoader moduleLoader)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    auto* apiGlobalObject = dynamicDowncast<JSAPIGlobalObject>(globalObject);
    if (!apiGlobalObject) {
        ASSERT_NOT_REACHED();
        return;
    }

    apiGlobalObject->setAPIModuleLoader(moduleLoader);
}

void JSSetAPIModuleLoader(JSContextRef ctx, JSAPIModuleLoader moduleLoader)
{
    JSModuleLoaderSetCallbacks(ctx, moduleLoader);
}

JSValueRef JSModuleLoadFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    if (!ctx || !module) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    SourceCode source = makeAPIModuleSource(module, sourceURLString, startingLineNumber);
    JSPromise* promise = loadModule(globalObject, WTF::move(source), nullptr);
    return promiseToRefOrSetException(globalObject, scope, promise, exception);
}

JSValueRef JSModuleLoadAndEvaluateFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    if (!ctx || !module) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    SourceCode source = makeAPIModuleSource(module, sourceURLString, startingLineNumber);
    JSPromise* promise = loadAndEvaluateModule(globalObject, WTF::move(source), nullptr);
    return promiseToRefOrSetException(globalObject, scope, promise, exception);
}

JSValueRef JSModuleLoad(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception)
{
    if (!ctx || !moduleKey) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSPromise* promise = loadModule(globalObject, Identifier::fromString(vm, moduleKey->string()), nullptr, nullptr);
    return promiseToRefOrSetException(globalObject, scope, promise, exception);
}

JSValueRef JSModuleLoadAndEvaluate(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception)
{
    if (!ctx || !moduleKey) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSPromise* promise = loadAndEvaluateModule(globalObject, moduleKey->string(), nullptr, nullptr);
    return promiseToRefOrSetException(globalObject, scope, promise, exception);
}

JSValueRef JSModuleLinkAndEvaluate(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception)
{
    if (!ctx || !moduleKey) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSPromise* promise = linkAndEvaluateModule(globalObject, Identifier::fromString(vm, moduleKey->string()), nullptr);
    return promiseToRefOrSetException(globalObject, scope, promise, exception);
}

JSValueRef JSSyntheticModuleCreate(JSContextRef ctx, JSStringRef moduleKey, size_t exportCount, const JSStringRef exportNames[], const JSValueRef exportValues[], JSValueRef* exception)
{
    if (!ctx || !moduleKey) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (exportCount && (!exportNames || !exportValues))
        return setTypeError(globalObject, exception, "Synthetic module export names and values are required"_s);

    Vector<Identifier, 4> exportIdentifiers;
    MarkedArgumentBuffer exportValueBuffer;
    HashSet<String> seenExportNames;

    for (size_t index = 0; index < exportCount; ++index) {
        JSStringRef exportNameRef = exportNames[index];
        JSValueRef exportValueRef = exportValues[index];
        if (!exportNameRef || !exportValueRef)
            return setTypeError(globalObject, exception, "Synthetic module export names and values cannot contain null entries"_s);

        String exportName = exportNameRef->string();
        if (!seenExportNames.add(exportName).isNewEntry)
            return setTypeError(globalObject, exception, makeString("Duplicate synthetic module export name '"_s, exportName, "'."_s));

        if (exportName == "default"_s)
            exportIdentifiers.append(vm.propertyNames->defaultKeyword);
        else
            exportIdentifiers.append(Identifier::fromString(vm, exportName));

        exportValueBuffer.append(toJS(globalObject, exportValueRef));
    }

    if (exportValueBuffer.hasOverflowed())
        return setTypeError(globalObject, exception, "Synthetic module export buffer overflowed"_s);

    Identifier key = Identifier::fromString(vm, moduleKey->string());
    auto* moduleRecord = SyntheticModuleRecord::createWithExportNamesAndValues(globalObject, key, exportIdentifiers, exportValueBuffer);
    if (setCaughtException(globalObject, scope, exception))
        return nullptr;

    bool didRegister = globalObject->moduleLoader()->provideModule(globalObject, key, ScriptFetchParameters::Type::JavaScript, moduleRecord);
    if (setCaughtException(globalObject, scope, exception))
        return nullptr;

    if (!didRegister)
        return setTypeError(globalObject, exception, makeString("Module '"_s, moduleKey->string(), "' is already registered."_s));

    return toRef(globalObject, moduleRecord);
}

void JSRunMicrotasks(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    vm.drainMicrotasks();
}

void JSRunDeferredWork(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    vm.deferredWorkTimer->runRunLoop();
}

void JSLoadAndEvaluateModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    (void)JSModuleLoadAndEvaluateFromSource(ctx, module, sourceURLString, startingLineNumber, exception);
}

void JSLoadModule(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception)
{
    (void)JSModuleLoad(ctx, moduleKey, exception);
}

void JSLoadModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    (void)JSModuleLoadFromSource(ctx, module, sourceURLString, startingLineNumber, exception);
}

JSValueRef JSLinkAndEvaluateModule(JSContextRef ctx, JSStringRef moduleKey)
{
    return JSModuleLinkAndEvaluate(ctx, moduleKey, nullptr);
}

void JSLoadAndEvaluateModule(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception)
{
    (void)JSModuleLoadAndEvaluate(ctx, moduleKey, exception);
}

bool JSCheckScriptSyntax(JSContextRef ctx, JSStringRef script, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return false;
    }
    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    startingLineNumber = std::max(1, startingLineNumber);

    auto sourceURL = sourceURLString ? URL({ }, sourceURLString->string()) : URL();
    SourceCode source = makeSource(script->string(), SourceOrigin { sourceURL }, SourceTaintedOrigin::Untainted, sourceURL.string(), TextPosition(OrdinalNumber::fromOneBasedInt(startingLineNumber), OrdinalNumber()));

    JSValue syntaxException;
    bool isValidSyntax = checkSyntax(globalObject, source, &syntaxException);

    if (!isValidSyntax) {
        if (exception)
            *exception = toRef(globalObject, syntaxException);
#if ENABLE(REMOTE_INSPECTOR)
        Exception* exception = Exception::create(vm, syntaxException);
        protect(globalObject->inspectorController())->reportAPIException(globalObject, exception);
#endif
        return false;
    }

    return true;
}

void JSGarbageCollect(JSContextRef ctx)
{
    // We used to recommend passing NULL as an argument here, which caused the only heap to be collected.
    // As there is no longer a shared heap, the previously recommended usage became a no-op (but the GC
    // will happen when the context group is destroyed).
    // Because the function argument was originally ignored, some clients may pass their released context here,
    // in which case there is a risk of crashing if another thread performs GC on the same heap in between.
    if (!ctx)
        return;

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    vm.heap.reportAbandonedObjectGraph();
}

void JSReportExtraMemoryCost(JSContextRef ctx, size_t size)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return;
    }
    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    vm.heap.deprecatedReportExtraMemory(size);
}

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);
extern "C" JS_EXPORT void JSSynchronousEdenCollectForDebugging(JSContextRef);

void JSSynchronousGarbageCollectForDebugging(JSContextRef ctx)
{
    if (!ctx)
        return;

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    vm.heap.collectNow(Sync, CollectionScope::Full);
}

void JSSynchronousEdenCollectForDebugging(JSContextRef ctx)
{
    if (!ctx)
        return;

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);
    vm.heap.collectSync(CollectionScope::Eden);
}

void JSDisableGCTimer(void)
{
    GCActivityCallback::s_shouldCreateGCTimer = false;
}

#if !OS(DARWIN) && !OS(WINDOWS)
bool JSConfigureSignalForGC(int signal)
{
    if (g_wtfConfig.isThreadSuspendResumeSignalConfigured)
        return false;
    g_wtfConfig.sigThreadSuspendResume = signal;
    g_wtfConfig.isUserSpecifiedThreadSuspendResumeSignalConfigured = true;
    return true;
}
#endif

JSObjectRef JSGetMemoryUsageStatistics(JSContextRef ctx)
{
    if (!ctx) {
        ASSERT_NOT_REACHED();
        return nullptr;
    }

    JSGlobalObject* globalObject = toJS(ctx);
    VM& vm = globalObject->vm();
    JSLockHolder locker(vm);

    auto typeCounts = vm.heap.objectTypeCounts();
    JSObject* objectTypeCounts = constructEmptyObject(globalObject);
    for (auto& it : typeCounts)
        objectTypeCounts->putDirect(vm, Identifier::fromString(vm, it.key), jsNumber(it.value));

    JSObject* object = constructEmptyObject(globalObject);
    object->putDirect(vm, Identifier::fromString(vm, "heapSize"_s), jsNumber(vm.heap.size()));
    object->putDirect(vm, Identifier::fromString(vm, "heapCapacity"_s), jsNumber(vm.heap.capacity()));
    object->putDirect(vm, Identifier::fromString(vm, "extraMemorySize"_s), jsNumber(vm.heap.extraMemorySize()));
    object->putDirect(vm, Identifier::fromString(vm, "objectCount"_s), jsNumber(vm.heap.objectCount()));
    object->putDirect(vm, Identifier::fromString(vm, "protectedObjectCount"_s), jsNumber(vm.heap.protectedObjectCount()));
    object->putDirect(vm, Identifier::fromString(vm, "globalObjectCount"_s), jsNumber(vm.heap.globalObjectCount()));
    object->putDirect(vm, Identifier::fromString(vm, "protectedGlobalObjectCount"_s), jsNumber(vm.heap.protectedGlobalObjectCount()));
    object->putDirect(vm, Identifier::fromString(vm, "objectTypeCounts"_s), objectTypeCounts);

    return toRef(object);
}
