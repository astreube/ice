// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NESTED_I_H
#define NESTED_I_H

#include <Nested.h>

class NestedI : public Demo::Nested
{
public:

    NestedI(const Demo::NestedPrx&);
    virtual void nestedCall(IceE::Int, const Demo::NestedPrx&, const IceE::Current&);

private:

    Demo::NestedPrx _self;
};

#endif
