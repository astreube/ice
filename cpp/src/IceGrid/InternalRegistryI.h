// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_INTERNALREGISTRYI_H
#define ICE_GRID_INTERNALREGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class FileCache;
typedef IceUtil::Handle<FileCache> FileCachePtr;

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class ReplicaSessionManager;

class InternalRegistryI : public InternalRegistry
{
public:

    InternalRegistryI(const RegistryIPtr&, const DatabasePtr&, const ReapThreadPtr&, 
		      const WellKnownObjectsManagerPtr&, ReplicaSessionManager&);
    virtual ~InternalRegistryI();

    virtual NodeSessionPrx registerNode(const NodeInfo&, const NodePrx&, const Ice::Current&);
    virtual ReplicaSessionPrx registerReplica(const RegistryInfo&, const InternalRegistryPrx&, const Ice::Current&);

    virtual void registerWithReplica(const InternalRegistryPrx&, const Ice::Current&);

    virtual NodePrxSeq getNodes(const Ice::Current&) const;
    virtual InternalRegistryPrxSeq getReplicas(const Ice::Current&) const;

    virtual void shutdown(const Ice::Current&) const;

    virtual Ice::StringSeq readLines(const std::string&, Ice::Long, int, Ice::Long&, const Ice::Current&) const;

private:    

    const RegistryIPtr _registry;
    const DatabasePtr _database;
    const ReapThreadPtr _reaper;
    const WellKnownObjectsManagerPtr _wellKnownObjects;
    const FileCachePtr _fileCache;
    ReplicaSessionManager& _session;
    int _nodeSessionTimeout;
    int _replicaSessionTimeout;
};
    
};

#endif
