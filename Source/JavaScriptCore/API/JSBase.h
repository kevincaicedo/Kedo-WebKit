/*
 * Copyright (C) 2006 Apple Inc.  All rights reserved.
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
@param scriptFetcher A JSValue containing the script fetcher.
@result A JSString containing the resolved module specifier.
*/
typedef JSStringRef
(*JSModuleLoaderResolve) (JSContextRef ctx, JSValueRef keyValue, JSValueRef referrerValue, JSValueRef scriptFetcher);


/*!
@typedef JSModuleLoaderEvaluate
@abstract The callback invoked when evaluating a module.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier to evaluate.
@param scriptFetcher A JSValue containing the script fetcher.
@param sentValue A JSValue containing the value to send to the module.
@param resumeMode A JSValue containing the resume mode.
@result A JSValue containing the result of evaluating the module.
*/
typedef JSValueRef
(*JSModuleLoaderEvaluate) (JSContextRef ctx, JSValueRef key);

/*!
@typedef JSModuleLoaderFetch
@abstract The callback invoked when fetching a module.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier to fetch.
@param attributesValue A JSValue containing the attributes.
@param scriptFetcher A JSValue containing the script fetcher.
@result A JSStringRef containing the fetched module.
*/
typedef JSStringRef
(*JSModuleLoaderFetch) (JSContextRef ctx, JSValueRef key, JSValueRef attributesValue, JSValueRef scriptFetcher);

/*!
@typedef JSModuleLoaderCreateImportMetaProperties
@abstract The callback invoked when creating import meta properties.
@param ctx The execution context to use.
@param key A JSValue containing the module specifier.
@param scriptFetcher A JSValue containing the script fetcher.
@result A JSObjectRef containing the import meta properties.
*/
typedef JSObjectRef
(*JSModuleLoaderCreateImportMetaProperties) (JSContextRef ctx, JSValueRef key, JSValueRef scriptFetcher);

/*!
@struct JSAPIModuleLoader
@abstract The callbacks used to load and evaluate modules.
@field moduleLoaderResolve The callback used to resolve a module specifier.
@field moduleLoaderEvaluate The callback used to evaluate a module.
@field moduleLoaderFetch The callback used to fetch a module.
*/
typedef struct {
    bool disableBuiltinFileSystemLoader;
    JSModuleLoaderResolve moduleLoaderResolve;
    JSModuleLoaderEvaluate moduleLoaderEvaluate;
    JSModuleLoaderFetch moduleLoaderFetch;
    JSModuleLoaderCreateImportMetaProperties moduleLoaderCreateImportMetaProperties;
} JSAPIModuleLoader;

/*!
@function JSSetAPIModuleLoader
@abstract Sets the moduleLoader used to load and evaluate modules.
@param ctx The execution context to use.
@param moduleLoader A JSAPIModuleLoader structure containing the callbacks to use.
*/
JS_EXPORT void JSSetAPIModuleLoader(JSContextRef ctx, JSAPIModuleLoader moduleLoader);

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
@function JSLoadAndEvaluateModule
@abstract Evaluates a file containing JavaScript Code.
@param ctx The execution context to use.
@param filename A JSString containing the path to the module to evaluate.
*/
JS_EXPORT void JSLoadAndEvaluateModule(JSContextRef ctx, JSStringRef filename, JSValueRef* exception);

/*!
@function JSLoadAndEvaluateModuleFromSource
@abstract Evaluates a string of JavaScript as a module.
@param ctx The execution context to use.
@param module A JSString containing the module code to evaluate.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadAndEvaluateModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSLoadModule
@abstract Loads a module.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to load.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadModule(JSContextRef ctx, JSStringRef moduleKey, JSValueRef* exception);

/*!
@function JSLoadModuleFromSource
@abstract Loads a module from a string of JavaScript.
@param ctx The execution context to use.
@param module A JSString containing the module code to load.
@param sourceURLString A JSString containing a URL for the script's source file. This is used by debuggers and when reporting exceptions. Pass NULL if you do not care to include source file information.
@param startingLineNumber An integer value specifying the script's starting line number in the file located at sourceURL. This is only used when reporting exceptions. The value is one-based, so the first line is line 1 and invalid values are clamped to 1.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSLoadModuleFromSource(JSContextRef ctx, JSStringRef module, JSStringRef sourceURLString, int startingLineNumber, JSValueRef* exception);

/*!
@function JSLinkAndEvaluateModule
@abstract Links and evaluates a module.
@param ctx The execution context to use.
@param moduleKey A JSString containing the module key to link and evaluate.
@result The JSValue that results from evaluating the module, or NULL if an exception is thrown.
*/
JS_EXPORT JSValueRef JSLinkAndEvaluateModule(JSContextRef ctx, JSStringRef moduleKey);

/*!
@function JSSetSyntheticModuleKeys
@abstract Sets the synthetic module keys.
@param ctx The execution context to use.
@param argumentCount The number of keys.
@param keys An array of JSString containing the keys.
@param exception A pointer to a JSValueRef in which to store an exception, if any. Pass NULL if you do not care to store an exception.
*/
JS_EXPORT void JSSetSyntheticModuleKeys(JSContextRef ctx, size_t argumentCount, const JSStringRef keys[]);

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

#endif /* JSBase_h */
