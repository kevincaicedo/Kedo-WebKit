/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DebuggerCallFrame.h"

#include "JSFunction.h"
#include "CodeBlock.h"
#include "Machine.h"
#include "Parser.h"

namespace JSC {

const UString* DebuggerCallFrame::functionName() const
{
    if (!m_codeBlock)
        return 0;

    JSFunction* function = static_cast<JSFunction*>(m_registers[RegisterFile::Callee].getJSValue());
    if (!function)
        return 0;
    return &function->name(m_exec);
}

DebuggerCallFrame::Type DebuggerCallFrame::type() const
{
    if (m_registers[RegisterFile::Callee].getJSValue())
        return FunctionType;

    return ProgramType;
}

JSObject* DebuggerCallFrame::thisObject() const
{
    if (!m_codeBlock)
        return 0;

    return static_cast<JSObject*>(m_registers[m_codeBlock->thisRegister].getJSValue());
}

JSValue* DebuggerCallFrame::evaluate(const UString& script, JSValue*& exception) const
{
    if (!m_codeBlock)
        return 0;

    ExecState newExec(m_scopeChain->globalObject(), m_registers);

    int sourceId;
    int errLine;
    UString errMsg;
    RefPtr<SourceProvider> sourceProvider = UStringSourceProvider::create(script);
    RefPtr<EvalNode> evalNode = newExec.parser()->parse<EvalNode>(&newExec, UString(), 1, sourceProvider, &sourceId, &errLine, &errMsg);
    if (!evalNode)
        return Error::create(&newExec, SyntaxError, errMsg, errLine, sourceId, 0);

    return newExec.machine()->execute(evalNode.get(), &newExec, thisObject(), m_scopeChain, &exception);
}

} // namespace JSC
