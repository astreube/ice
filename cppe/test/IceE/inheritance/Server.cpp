// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

class InheritanceTestApplication : public TestApplication
{
public:

    InheritanceTestApplication() :
        TestApplication("inheritance server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	IceE::PropertiesPtr properties = IceE::getDefaultProperties(argc, argv);
        properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");

        setCommunicator(IceE::initialize(argc, argv));

        IceE::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        IceE::ObjectPtr object = new InitialI(adapter);
        adapter->add(object, IceE::stringToIdentity("initial"));
        adapter->activate();

#ifndef _WIN32_WCE
	communicator()->waitForShutdown();
#endif

        return EXIT_SUCCESS;
    }

};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    InheritanceTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    InheritanceTestApplication app;
    return app.main(argc, argv);
}

#endif
