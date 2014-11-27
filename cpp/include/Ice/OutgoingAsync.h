// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Timer.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/AsyncResult.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ObjectAdapterF.h>

namespace IceInternal
{

class RetryException;
class CollocatedRequestHandler;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
class ICE_API OutgoingAsyncBase : public Ice::AsyncResult
{
public:

    //
    // Those methods must be overriden if the invocation is sent
    // through a request handler.
    //
    virtual AsyncStatus send(const Ice::ConnectionIPtr&, bool, bool) { assert(false); return AsyncStatusQueued; }
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*) { assert(false); return AsyncStatusQueued; }

    virtual bool sent();
    virtual bool completed(const Ice::Exception&);
    virtual void retryException(const Ice::Exception&);

    // Those methods are public when called from an OutgoingAsyncBase reference.
    using Ice::AsyncResult::cancelable;
    using Ice::AsyncResult::invokeSent;
    using Ice::AsyncResult::invokeSentAsync;
    using Ice::AsyncResult::invokeCompleted;
    using Ice::AsyncResult::invokeCompletedAsync;

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getRemoteObserver(c, endpt, requestId, size));
    }
    
    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getCollocatedObserver(adapter, requestId, size));
    }

    BasicStream* getOs()
    {
        return &_os;
    }

protected:

    OutgoingAsyncBase(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&, const CallbackBasePtr&, 
                      const Ice::LocalObjectPtr&);

    bool sent(bool);
    bool finished(const Ice::Exception&);

    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    BasicStream _os;
};

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
class ICE_API ProxyOutgoingAsyncBase : public OutgoingAsyncBase, protected IceUtil::TimerTask
{
public:

    virtual Ice::ObjectPrx getProxy() const;

    using OutgoingAsyncBase::sent;
    virtual bool completed(const Ice::Exception&);
    virtual void retryException(const Ice::Exception&);
    virtual void cancelable(const CancellationHandlerPtr&);

    void retry();
    void abort(const Ice::Exception&);

protected:

    ProxyOutgoingAsyncBase(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, 
                           const Ice::LocalObjectPtr&);

    void invokeImpl(bool);

    bool sent(bool);
    bool finished(const Ice::Exception&);
    bool finished(bool);

    virtual void handleRetryException(const Ice::Exception&);
    virtual int handleException(const Ice::Exception&);
    virtual void runTimerTask();

    const Ice::ObjectPrx _proxy;
    RequestHandlerPtr _handler;
    Ice::OperationMode _mode;

private:

    int _cnt;
    bool _sent;
};

//
// Class for handling Slice operation invocations
//
class ICE_API OutgoingAsync : public ProxyOutgoingAsyncBase
{
public:

    OutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void prepare(const std::string&, Ice::OperationMode, const Ice::Context*);

    virtual bool sent();

    virtual AsyncStatus send(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void abort(const Ice::Exception&);

    void invoke();
    using ProxyOutgoingAsyncBase::completed;
    bool completed();

    BasicStream* startWriteParams(Ice::FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return &_os;
    }
    void endWriteParams()
    {
        _os.endWriteEncaps();
    }
    void writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }
    void writeParamEncaps(const ::Ice::Byte* encaps, ::Ice::Int size)
    {
        if(size == 0)
        {
            _os.writeEmptyEncaps(_encoding);
        }
        else
        {
            _os.writeEncaps(encaps, size);
        }
    }

    BasicStream* getIs()
    {
        return &_is;
    }

private:

    const Ice::EncodingVersion _encoding;
};

//
// Class for handling the proxy's begin_ice_flushBatchRequest request.
//
class ICE_API ProxyFlushBatch : public ProxyOutgoingAsyncBase
{
public:

    ProxyFlushBatch(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    virtual AsyncStatus send(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke();

private:

    virtual void handleRetryException(const Ice::Exception&);
    virtual int handleException(const Ice::Exception&);
};
typedef IceUtil::Handle<ProxyFlushBatch> ProxyFlushBatchPtr;

//
// Class for handling the proxy's begin_ice_getConnection request.
//
class ICE_API ProxyGetConnection :  public ProxyOutgoingAsyncBase
{
public:

    ProxyGetConnection(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    virtual AsyncStatus send(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke();
};
typedef IceUtil::Handle<ProxyGetConnection> ProxyGetConnectionPtr;

//
// Class for handling Ice::Connection::begin_flushBatchRequests
//
class ICE_API ConnectionFlushBatch : public OutgoingAsyncBase
{
public:

    ConnectionFlushBatch(const Ice::ConnectionIPtr&, const Ice::CommunicatorPtr&, const InstancePtr&,
                         const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);
    
    virtual Ice::ConnectionPtr getConnection() const;

    void invoke();

private:

    const Ice::ConnectionIPtr _connection;
};
typedef IceUtil::Handle<ConnectionFlushBatch> ConnectionFlushBatchPtr;

//
// Class for handling Ice::Communicator::begin_flushBatchRequests
//
class ICE_API CommunicatorFlushBatch : public Ice::AsyncResult
{
public:

    CommunicatorFlushBatch(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&,
                           const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void flushConnection(const Ice::ConnectionIPtr&);
    void ready();

private:

    void check(bool);

    int _useCount;
};

}

#endif
