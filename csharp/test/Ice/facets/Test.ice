// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

["cs:namespace:Ice.facets"]
module Test
{

interface Empty
{
}

interface A
{
    string callA();
}

interface B extends A
{
    string callB();
}

interface C extends A
{
    string callC();
}

interface D extends B, C
{
    string callD();
}

interface E
{
    string callE();
}

interface F extends E
{
    string callF();
}

interface G
{
    void shutdown();
    string callG();
}

interface H extends G
{
    string callH();
}

}
