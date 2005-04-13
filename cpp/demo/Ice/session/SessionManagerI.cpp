// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HelloSessionManagerI.h>

using namespace std;
using namespace Demo;

ReapThread::ReapThread(const SessionManagerIPtr& manager, const IceUtil::Time& timeout) :
    _destroy(false),
    _timeout(timeout),
    _manager(manager)
{
}

ReapThread::~ReapThread()
{
}

void
ReapThread::run()
{
    Lock sync(*this);
    while(!_destroy)
    {
	timedWait(_timeout);
	if(_destroy)
	{
	    break;
	}
    	_manager->reap();
    }
}

void
ReapThread::destroy()
{
    Lock sync(*this);
    _destroy = true;
    notify();
}

SessionManagerI::SessionManagerI(const Ice::CommunicatorPtr& communicator) :
    _timeout(IceUtil::Time::seconds(10)),
    _reapThread(new ReapThread(this, _timeout)),
    _logger(communicator->getLogger()),
    _destroy(false)
{
    _reapThread->start();
}

SessionManagerI::~SessionManagerI()
{
    assert(_sessions.size() == 0);
    assert(_destroy);
}

void
SessionManagerI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}

void
SessionManagerI::destroy()
{
    Lock sync(*this);

    assert(!_destroy);
    _destroy = true;
    _reapThread->destroy();
    _reapThread->getThreadControl().join();

    //
    // Destroy each session.
    //
    for(map<Ice::Identity, pair< IceUtil::Time, SessionPrx> >::iterator p = _sessions.begin();
	p != _sessions.end();
	++p)
    {
	try
	{
	    p->second.second->destroyed();
	}
	catch(const Ice::Exception& ex)
	{
	    Ice::Warning warn(_logger);
	    warn << "Session::destroyed failed: " << ex;
	}
    }
    _sessions.clear();
}

void
SessionManagerI::add(const SessionPrx& session)
{
    Lock sync(*this);

    Ice::Trace trace(_logger, "SessionManagerI");
    trace << "add: " << Ice::identityToString(session->ice_getIdentity());

    assert(!_destroy);
    _sessions.insert(make_pair(session->ice_getIdentity(), make_pair(IceUtil::Time::now(), session)));
}

void
SessionManagerI::remove(const Ice::Identity& id)
{
    Lock sync(*this);

    assert(!_destroy);
    map<Ice::Identity, pair< IceUtil::Time, SessionPrx> >::iterator p = _sessions.find(id);
    if(p != _sessions.end())
    {
	try
	{
	    p->second.second->destroyed();
	}
	catch(const Ice::Exception& ex)
	{
	    Ice::Warning warn(_logger);
	    warn << "Session::destroyed failed: " << ex;
	}
	_sessions.erase(p);
    }

    Ice::Trace trace(_logger, "SessionManagerI");
    trace << "SessionManagerI::remove: " << Ice::identityToString(id);
}

void
SessionManagerI::refresh(const Ice::Identity& id)
{
    Lock sync(*this);
    map<Ice::Identity, pair< IceUtil::Time, SessionPrx> >::iterator p = _sessions.find(id);
    if(p != _sessions.end())
    {
	p->second.first = IceUtil::Time::now();
    }
    // Its possible the code reaches here if a session times out and
    // is removed at the same time as it calls refresh.
}

void
SessionManagerI::reap()
{
    Lock sync(*this);

    //
    // Run through the sessions destroying those which have not
    // been refreshed in the _timeout interval.
    //
    IceUtil::Time now = IceUtil::Time::now();
    map<Ice::Identity, pair< IceUtil::Time, SessionPrx> >::iterator p = _sessions.begin();
    while(p != _sessions.end())
    {
	if((now - p->second.first) > _timeout)
	{
	    Ice::Trace trace(_logger, "SessionManagerI");
	    trace << "SessionManagerI::reaping: " << Ice::identityToString(p->first);

	    try
	    {
		p->second.second->destroyed();
	    }
	    catch(const Ice::Exception& ex)
	    {
		Ice::Warning warn(_logger);
		warn << "Session::destroyed failed: " << ex;
	    }
	    map<Ice::Identity, pair< IceUtil::Time, SessionPrx> >::iterator tmp = p;
	    ++p;
	    _sessions.erase(tmp);
	}
	else
	{
	    ++p;
	}
    }
}
