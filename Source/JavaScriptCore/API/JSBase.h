/*
 * Copyright (C) 2006-2026 Apple Inc. All rights reserved.
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

#ifndef JSBase_h
#define JSBase_h

#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <stddef.h> /* for size_t */
#include <stdint.h> /* for uint8_t */

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

/* JavaScript engine interface */

/*! @typedef JSContextGroupRef A group that associates JavaScript contexts with one another. Contexts in the same group may share and exchange JavaScript objects. */
typedef const struct OpaqueJSContextGroup* JSContextGroupRef;

/*! @typedef JSContextRef A JavaScript execution context. Holds the global object and other execution state. */
typedef const struct OpaqueJSContext* JSContextRef;

/*! @typedef JSGlobalContextRef A global JavaScript execution context. A JSGlobalContext is a JSContext. */
typedef struct OpaqueJSContext* JSGlobalContextRef;

/*! @typedef JSStringRef A UTF16 character buffer. The fundamental string representation in JavaScript. */
typedef struct OpaqueJSString* JSStringRef;

/*! @typedef JSClassRef A JavaScript class. Used with JSObjectMake to construct objects with custom behavior. */
typedef struct OpaqueJSClass* JSClassRef;

/*! @typedef JSPropertyNameArrayRef An array of JavaScript property names. */
typedef struct OpaqueJSPropertyNameArray* JSPropertyNameArrayRef;

/*! @typedef JSPropertyNameAccumulatorRef An ordered set used to collect the names of a JavaScript object's properties. */
typedef struct OpaqueJSPropertyNameAccumulator* JSPropertyNameAccumulatorRef;

/*! @typedef JSTypedArrayBytesDeallocator A function used to deallocate bytes passed to a Typed Array constructor. The function should take two arguments. The first is a pointer to the bytes that were originally passed to the Typed Array constructor. The second is a pointer to additional information desired at the time the bytes are to be freed. */
typedef void (*JSTypedArrayBytesDeallocator)(void* bytes, void* deallocatorContext);

/* JavaScript data types */

/*! @typedef JSValueRef A JavaScript value. The base type for all JavaScript values, and polymorphic functions on them. */
typedef const struct OpaqueJSValue* JSValueRef;

/*! @typedef JSObjectRef A JavaScript object. A JSObject is a JSValue. */
typedef struct OpaqueJSValue* JSObjectRef;

/*! @typedef JSModuleSourceRef A fetched module source object. */
typedef struct OpaqueJSModuleSource* JSModuleSourceRef;

/* Clang's __has_declspec_attribute emulation */
/* https://clang.llvm.org/docs/LanguageExtensions.html#has-declspec-attribute */

#ifndef __has_declspec_attribute
#define __has_declspec_attribute(x) 0
#endif

/* JavaScript symbol exports */
/* These rules should stay the same as in WebKit/Shared/API/c/WKDeclarationSpecifiers.h */

#undef JS_EXPORT
#if defined(JS_NO_EXPORT)
#define JS_EXPORT
#elif defined(WIN32) || defined(_WIN32) || defined(__CC_ARM) || defined(__ARMCC__) || (__has_declspec_attribute(dllimport) && __has_declspec_attribute(dllexport))
#if defined(BUILDING_JavaScriptCore) || defined(STATICALLY_LINKED_WITH_JavaScriptCore)
#define JS_EXPORT __declspec(dllexport)
#else
#define JS_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#define JS_EXPORT __attribute__((visibility("default")))
#else /* !defined(JS_NO_EXPORT) */
#define JS_EXPORT
#endif /* defined(JS_NO_EXPORT) */

#ifdef __cplusplus
extern "C" {
#endif

/* Script Module */

/*!
@typedef JSModuleLoaderResolve
@abstract The callback invoked when resolving a module specifier.
@param ctx The execution context to use.
@param keyValue A JSValue containing the module specifier to resolve.
@param referrerValue A JSValue containing the referrer URL.
@param scriptFetcher Reserved for future script fetcher support. Currently undefined for API callbacks.
@result A JSString containing the resolved module specifier.
*/
typedef JSStringRef
(*JSModuleLoaderResolve) (JSContextRef ctx, JSValueRef keyValue, JSValueRef referrerValue, JSValueRef scriptFetcher);


/*!
@typedef JSModuleLoaderEvaluate
@abstract Reserved legacy callback slot. Synthetic modules should be created with JSSyntheticModuleCreate.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier to evaluate.
@result A JSValue reserved for legacy embedders.
*/
typedef JSValueRef
(*JSModuleLoaderEvaluate) (JSContextRef ctx, JSValueRef key);

/*!
@typedef JSModuleLoaderFetch
@abstract The callback invoked when fetching a module.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier to fetch.
@param attributesValue A JSValue string describing the requested import type. It is "json" for JSON imports, "javascript" for JavaScript imports, "webassembly" for WebAssembly imports, and undefined when no type is available.
@param scriptFetcher Reserved for future script fetcher support. Currently undefined for API callbacks.
@result A JSStringRef containing the fetched module.
*/
typedef JSStringRef
(*JSModuleLoaderFetch) (JSContextRef ctx, JSValueRef key, JSValueRef attributesValue, JSValueRef scriptFetcher);

/*!
@typedef JSModuleLoaderFetchSource
@abstract The callback invoked when fetching a typed module source.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier to fetch.
@param attributesValue A JSValue string describing the requested import type. It is "json" for JSON imports, "javascript" for JavaScript imports, "webassembly" for WebAssembly imports, and undefined when no type is available.
@param scriptFetcher Reserved for future script fetcher support. Currently undefined for API callbacks.
@result A JSModuleSourceRef containing the fetched module. Ownership is transferred to JavaScriptCore when returned from this callback.
*/
typedef JSModuleSourceRef
(*JSModuleLoaderFetchSource) (JSContextRef ctx, JSValueRef key, JSValueRef attributesValue, JSValueRef scriptFetcher);

/*!
@typedef JSModuleLoaderCreateImportMetaProperties
@abstract The callback invoked when creating import meta properties.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier.
@param scriptFetcher Reserved for future script fetcher support. Currently undefined for API callbacks.
@result A JSObjectRef containing the import meta properties.
*/
typedef JSObjectRef
(*JSModuleLoaderCreateImportMetaProperties) (JSContextRef ctx, JSValueRef key, JSValueRef scriptFetcher);

/*!
@struct JSAPIModuleLoader
@abstract The callbacks used to load and evaluate modules.
@field moduleLoaderResolve The callback used to resolve a module specifier.
@field moduleLoaderEvaluate Reserved legacy callback slot. Prefer JSSyntheticModuleCreate for synthetic modules.
@field moduleLoaderFetch Legacy callback used to fetch text modules. It cannot return WebAssembly bytes.
@field moduleLoaderFetchSource Preferred callback used to fetch typed JavaScript, JSON, or WebAssembly module sources.
@field moduleLoaderCreateImportMetaProperties The callback used to create import.meta properties.
*/
typedef struct {
    JSModuleLoaderResolve moduleLoaderResolve;
    JSModuleLoaderEvaluate moduleLoaderEvaluate;
    JSModuleLoaderFetch moduleLoaderFetch;
    JSModuleLoaderFetchSource moduleLoaderFetchSource;
    JSModuleLoaderCreateImportMetaProperties moduleLoaderCreateImportMetaProperties;
} JSAPIModuleLoader;

/*!
@function JSModuleSourceCreateJavaScript
@abstract Creates a JavaScript module source object by copying the provided string.
@param source A JSString containing JavaScript module source text.
@result A module source object, or NULL when source is NULL. Release with JSModuleSourceRelease unless returning it from JSModuleLoaderFetchSource.
*/
JS_EXPORT JSModuleSourceRef JSModuleSourceCreateJavaScript(JSStringRef source);

/*!
@function JSModuleSourceCreateJSON
@abstract Creates a JSON module source object by copying the provided string.
@param source A JSString containing raw JSON module source text.
@result A module source object, or NULL when source is NULL. Release with JSModuleSourceRelease unless returning it from JSModuleLoaderFetchSource.
*/
JS_EXPORT JSModuleSourceRef JSModuleSourceCreateJSON(JSStringRef source);

/*!
@function JSModuleSourceCreateWebAssembly
@abstract Creates a WebAssembly module source object by copying the provided bytes.
@param bytes A pointer to WebAssembly binary bytes. May be NULL only when byteLength is zero.
@param byteLength The number of bytes to copy.
@result A module source object, or NULL when bytes is NULL and byteLength is non-zero. Release with JSModuleSourceRelease unless returning it from JSModuleLoaderFetchSource.
*/
JS_EXPORT JSModuleSourceRef JSModuleSourceCreateWebAssembly(const uint8_t* bytes, size_t byteLength);

/*!
@function JSModuleSourceRelease
@abstract Releases a module source object created with JSModuleSourceCreateJavaScript, JSModuleSourceCreateJSON, or JSModuleSourceCreateWebAssembly.
@param source The source object to release. Passing NULL is allowed.
*/
JS_EXPORT void JSModuleSourceRelease(JSModuleSourceRef source);

/*!
@typedef JSUncaughtExceptionAtEventLoop
@abstract The callback invoked when an exception is not caught in the event loop.
@param ctx The execution context to use.
@param exception A JSValue containing the uncaught exception.
*/
typedef void (*JSUncaughtExceptionAtEventLoop) (JSContextRef ctx, JSValueRef exception);

/*!
@typedef JSUncaughtExceptionHandler
@abstract The callback invoked when an exception is not caught.
@param ctx The execution context to use.
@param filename Reserved source-name slot. JavaScriptCore passes an empty string when no source name is available.
@param exception A JSValue containing the uncaught exception.
*/
typedef void (*JSUncaughtExceptionHandler) (JSContextRef ctx, JSStringRef filename, JSValueRef exception);

/*!
@function JSSetAPIModuleLoader
@abstract Sets the moduleLoader used to load and evaluate modules.
@param ctx The execution context to use.
@param moduleLoader A JSAPIModuleLoader structure containing the callbacks to use.
*/
JS_EXPORT void JSSetAPIModuleLoader(JSContextRef ctx, JSAPIModuleLoader moduleLoader);

/*!
@function JSModuleLoaderSetCallbacks
@abstract Sets the module loader callbacks used by module operations.
@param ctx The execution context to use.
@param moduleLoader A JSAPIModuleLoader structure containing the callbacks to use.
*/
JS_EXPORT void JSModuleLoaderSetCallbacks(JSContextRef ctx, JSAPIModuleLoader moduleLoader);

/* Script Evaluation */

/*!
@function JSEvaluateScript
@abstract Evaluates a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to evaluate.
@param thisObject The object to use as "this," or NULL to use the global object as "this."
@param sourceURL A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result The JSValue that results from evaluating script, or NULL if an exception is thrown.
*/
JS_EXPORT JSValueRef JSEvaluateScript(JSContextRef ctx, JSStringRef script, JSObjectRef thisObject, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

/* Module Evaluation */

/*!
@function JSModuleLoadAndEvaluate
@abstract Starts loading, linking, and evaluating a module by key using the registered API module loader callbacks.
@param ctx The execution context to use.
@param moduleKey A JSString containing the resolved module key to evaluate.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
@result A Promise for module evaluation, or NULL if evaluation could not start.
*/
JS_EXPORT JSValueRef JSModuleLoadAndEvaluate(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSLoadAndEvaluateModule
@abstract Deprecated compatibility wrapper for JSModuleLoadAndEvaluate. It starts module work and does not drain microtasks.
@param ctx The execution context to use.
@param moduleKey A JSString containing the resolved module key to evaluate.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadAndEvaluateModule(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSModuleLoadAndEvaluateFromSource
@abstract Starts loading, linking, and evaluating a string of JavaScript as a module.
@param ctx The execution context to use.
@param module A JSString containing the module code to evaluate.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
@result A Promise for module evaluation, or NULL if evaluation could not start.
*/
JS_EXPORT JSValueRef JSModuleLoadAndEvaluateFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSLoadAndEvaluateModuleFromSource
@abstract Deprecated compatibility wrapper for JSModuleLoadAndEvaluateFromSource. It starts module work and does not drain microtasks.
@param ctx The execution context to use.
@param module A JSString containing the module code to evaluate.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadAndEvaluateModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSModuleLoad
@abstract Starts loading a module.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to load.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
@result A Promise for module loading, or NULL if loading could not start.
*/
JS_EXPORT JSValueRef JSModuleLoad(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSLoadModule
@abstract Deprecated compatibility wrapper for JSModuleLoad. It starts module work and does not drain microtasks.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to load.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadModule(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSModuleLoadFromSource
@abstract Starts loading a module from a string of JavaScript.
@param ctx The execution context to use.
@param module A JSString containing the module code to load.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
@result A Promise for module loading, or NULL if loading could not start.
*/
JS_EXPORT JSValueRef JSModuleLoadFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSLoadModuleFromSource
@abstract Deprecated compatibility wrapper for JSModuleLoadFromSource. It starts module work and does not drain microtasks.
@param ctx The execution context to use.
@param module A JSString containing the module code to load.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSModuleLinkAndEvaluate
@abstract Starts linking and evaluating a loaded module.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to link and evaluate.
@param exception A pointer to a JSValueRef in which to store a startup exception, if any. Pass NULL if you do not care to store an exception.
@result A Promise for module evaluation, or NULL if evaluation could not start.
*/
JS_EXPORT JSValueRef JSModuleLinkAndEvaluate(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSLinkAndEvaluateModule
@abstract Deprecated compatibility wrapper for JSModuleLinkAndEvaluate.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to link and evaluate.
@result A Promise for module evaluation, or NULL if evaluation could not start.
*/
JS_EXPORT JSValueRef JSLinkAndEvaluateModule(JSContextRef ctx, JSStringRef moduleKey);

/*!
@typedef JSModuleEvaluationCallback
@abstract The callback invoked when a module has been evaluated.
@param ctx The execution context to use.
@param moduleNamespaceObject A JSValueRef containing the module namespace object.
@param exception A JSValueRef containing an exception, if any.
*/
typedef void (*JSModuleEvaluationCallback)(JSContextRef ctx, JSValueRef moduleNamespaceObject, JSValueRef exception);

/*!
@function JSSyntheticModuleCreate
@abstract Creates and registers a synthetic module with explicit exports.
@param ctx The execution context to use.
@param moduleKey The module key to register.
@param exportCount The number of exports.
@param exportNames Caller-owned JSString values containing export names. The strings are copied.
@param exportValues Caller-owned JSValue values containing export values. The values are retained by the created module record.
@param exception A pointer to a JSValueRef in which to store a creation exception, if any. Pass NULL if you do not care to store an exception.
@result The created synthetic module record, or NULL if creation failed.
*/
JS_EXPORT JSValueRef JSSyntheticModuleCreate(JSContextRef ctx, JSStringRef moduleKey, size_t exportCount, const JSStringRef exportNames[], const JSValueRef exportValues[], JSValueRef* exception);

/*!
@function JSRunMicrotasks
@abstract Runs pending JavaScript microtasks for the context's VM. This is an explicit host pump and module APIs do not call it implicitly.
@param ctx The execution context to use.
*/
JS_EXPORT void JSRunMicrotasks(JSContextRef ctx);

/*!
@function JSRunDeferredWork
@abstract Runs pending JavaScriptCore deferred work for the context's VM. This includes async WebAssembly compilation completions. Do not call while already executing JavaScriptCore callbacks.
@param ctx The execution context to use.
*/
JS_EXPORT void JSRunDeferredWork(JSContextRef ctx);

/*!
@function JSCheckScriptSyntax
@abstract Checks for syntax errors in a string of JavaScript.
@param ctx The execution context to use.
@param script A JSString containing the script to check for syntax errors.
@param sourceURL A JSString containing a URL for the script's source file. This is only used when reporting exceptions. Pass NULL if you do not care to include source file information in exceptions.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store a syntax error exception, if any. Pass NULL if you do not care to store a syntax error exception.
@result true if the script is syntactically correct, otherwise false.
*/
JS_EXPORT bool JSCheckScriptSyntax(JSContextRef ctx, JSStringRef script, JSStringRef sourceURL, int startingLineNumber, JSValueRef* exception);

/*!
@function JSGarbageCollect
@abstract Performs a JavaScript garbage collection.
@param ctx The execution context to use.
@discussion JavaScript values that are on the machine stack, in a register,
 protected by JSValueProtect, set as the global object of an execution context,
 or reachable from any such value will not be collected.

 During JavaScript execution, you are not required to call this function; the
 JavaScript engine will garbage collect as needed. JavaScript values created
 within a context group are automatically destroyed when the last reference
 to the context group is released.
*/
JS_EXPORT void JSGarbageCollect(JSContextRef ctx);


/*!
@function JSGetMemoryUsageStatistics
@abstract Returns the current memory usage of a context.
@param ctx The execution context to use.
@result A JSObjectRef containing the memory usage statistics.
*/
JS_EXPORT JSObjectRef JSGetMemoryUsageStatistics(JSContextRef ctx);

#ifdef __cplusplus
}
#endif

/* Enable the Objective-C API for platforms with a modern runtime. NOTE: This is duplicated in VM.h. */
#if !defined(JSC_OBJC_API_ENABLED)
#if (defined(__clang__) && defined(__APPLE__) && (defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)))
#define JSC_OBJC_API_ENABLED 1
#else
#define JSC_OBJC_API_ENABLED 0
#endif
#endif

#if JSC_OBJC_API_ENABLED
#if defined(__cplusplus)
// In C++, avoid the forward declaration pattern from CF_ENUM
// that triggers -Welaborated-enum-base warnings.
#define JSC_CF_ENUM(enumName, ...) \
    enum enumName : uint32_t { \
        __VA_ARGS__                \
    }
#else
// In Objective-C, use CF_ENUM for full Swift interop support.
#define JSC_CF_ENUM(enumName, ...)       \
    typedef CF_ENUM(uint32_t, enumName) { \
        __VA_ARGS__                       \
    }
#endif
#else
#define JSC_CF_ENUM(enumName, ...) \
    typedef enum {                  \
        __VA_ARGS__                 \
    } enumName
#endif

#if JSC_OBJC_API_ENABLED
#define JSC_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define JSC_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define JSC_ASSUME_NONNULL_BEGIN
#define JSC_ASSUME_NONNULL_END
#endif

#if JSC_OBJC_API_ENABLED
#define JSC_NULL_UNSPECIFIED _Null_unspecified
#define JSC_NULLABLE _Nullable
#define JSC_NONNULL _Nonnull
#else
#define JSC_NULL_UNSPECIFIED
#define JSC_NULLABLE
#define JSC_NONNULL
#endif

#endif /* JSBase_h */
