/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef JSTestObj_h
#define JSTestObj_h

#include "JSDOMBinding.h"
#include <runtime/JSGlobalObject.h>
#include <runtime/ObjectPrototype.h>

namespace WebCore {

class TestObj;

class JSTestObj : public DOMObjectWithGlobalPointer {
    typedef DOMObjectWithGlobalPointer Base;
public:
    JSTestObj(NonNullPassRefPtr<JSC::Structure>, JSDOMGlobalObject*, PassRefPtr<TestObj>);
    virtual ~JSTestObj();
    static JSC::JSObject* createPrototype(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertyDescriptor&);
    virtual void put(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::JSValue, JSC::PutPropertySlot&);
    virtual const JSC::ClassInfo* classInfo() const { return &s_info; }
    static const JSC::ClassInfo s_info;

    static PassRefPtr<JSC::Structure> createStructure(JSC::JSValue prototype)
    {
        return JSC::Structure::create(prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount);
    }

    static JSC::JSValue getConstructor(JSC::ExecState*, JSC::JSGlobalObject*);

    // Custom attributes
    JSC::JSValue customAttr(JSC::ExecState*) const;
    void setCustomAttr(JSC::ExecState*, JSC::JSValue);

    // Custom functions
    JSC::JSValue customMethod(JSC::ExecState*, const JSC::ArgList&);
    JSC::JSValue customMethodWithArgs(JSC::ExecState*, const JSC::ArgList&);
    TestObj* impl() const { return m_impl.get(); }

private:
    RefPtr<TestObj> m_impl;
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
};

JSC::JSValue toJS(JSC::ExecState*, JSDOMGlobalObject*, TestObj*);
TestObj* toTestObj(JSC::JSValue);

class JSTestObjPrototype : public JSC::JSObject {
    typedef JSC::JSObject Base;
public:
    static JSC::JSObject* self(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual const JSC::ClassInfo* classInfo() const { return &s_info; }
    static const JSC::ClassInfo s_info;
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier&, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier&, JSC::PropertyDescriptor&);
    static PassRefPtr<JSC::Structure> createStructure(JSC::JSValue prototype)
    {
        return JSC::Structure::create(prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount);
    }
    JSTestObjPrototype(NonNullPassRefPtr<JSC::Structure> structure) : JSC::JSObject(structure) { }
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
};

// Functions

JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionVoidMethod(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionVoidMethodWithArgs(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionIntMethod(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionIntMethodWithArgs(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionObjMethod(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionObjMethodWithArgs(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionMethodWithException(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionCustomMethod(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionCustomMethodWithArgs(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionCustomArgsAndException(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithDynamicFrame(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithDynamicFrameAndArg(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithDynamicFrameAndOptionalArg(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithDynamicFrameAndUserGesture(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithDynamicFrameAndUserGestureASAD(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithScriptStateVoid(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionWithScriptStateObj(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionMethodWithOptionalArg(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionMethodWithNonOptionalArgAndOptionalArg(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
JSC::JSValue JSC_HOST_CALL jsTestObjPrototypeFunctionMethodWithNonOptionalArgAndTwoOptionalArgs(JSC::ExecState*, JSC::JSObject*, JSC::JSValue, const JSC::ArgList&);
// Attributes

JSC::JSValue jsTestObjReadOnlyIntAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTestObjReadOnlyStringAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTestObjReadOnlyTestObjAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTestObjIntAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjIntAttr(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjStringAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjStringAttr(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjTestObjAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjTestObjAttr(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjAttrWithException(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjAttrWithException(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjAttrWithSetterException(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjAttrWithSetterException(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjAttrWithGetterException(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjAttrWithGetterException(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjCustomAttr(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTestObjCustomAttr(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTestObjConstructor(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);

} // namespace WebCore

#endif
