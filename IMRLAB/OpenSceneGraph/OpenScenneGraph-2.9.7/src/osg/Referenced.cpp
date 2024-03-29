/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <stdlib.h>

#include <osg/Referenced>
#include <osg/Notify>
#include <osg/ApplicationUsage>
#include <osg/Observer>

#include <typeinfo>
#include <memory>
#include <set>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#include <osg/DeleteHandler>

namespace osg
{

//#define ENFORCE_THREADSAFE
//#define DEBUG_OBJECT_ALLOCATION_DESTRUCTION

// specialized smart pointer, used to get round auto_ptr<>'s lack of the destructor reseting itself to 0.
template<typename T>
struct ResetPointer
{
    ResetPointer():
        _ptr(0) {}

    ResetPointer(T* ptr):
        _ptr(ptr) {}

    ~ResetPointer()
    {
        delete _ptr;
        _ptr = 0;
    }

    inline ResetPointer& operator = (T* ptr)
    {
        if (_ptr==ptr) return *this;
        delete _ptr;
        _ptr = ptr;
        return *this;
    }

    void reset(T* ptr)
    {
        if (_ptr==ptr) return;
        delete _ptr;
        _ptr = ptr;
    }

    inline T& operator*()  { return *_ptr; }

    inline const T& operator*() const { return *_ptr; }

    inline T* operator->() { return _ptr; }

    inline const T* operator->() const   { return _ptr; }

    T* get() { return _ptr; }

    const T* get() const { return _ptr; }

    T* _ptr;
};

typedef ResetPointer<DeleteHandler> DeleteHandlerPointer;
typedef ResetPointer<OpenThreads::Mutex> GlobalMutexPointer;

OpenThreads::Mutex* Referenced::getGlobalReferencedMutex()
{
    static GlobalMutexPointer s_ReferencedGlobalMutext = new OpenThreads::Mutex;
    return s_ReferencedGlobalMutext.get();
}

// we'll implement Observer::getGlobalObserverMutex() here for convinience as ResetPointer is available.
OpenThreads::Mutex* Observer::getGlobalObserverMutex()
{
    static GlobalMutexPointer s_ReferencedGlobalMutext = new OpenThreads::Mutex(OpenThreads::Mutex::MUTEX_RECURSIVE);
    return s_ReferencedGlobalMutext.get();
}

// helper class for forcing the global mutex to be constructed when the library is loaded. 
struct InitGlobalMutexes
{
    InitGlobalMutexes()
    {
        Referenced::getGlobalReferencedMutex();
        Observer::getGlobalObserverMutex();
    }
};
static InitGlobalMutexes s_initGlobalMutexes;


#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
static bool s_useThreadSafeReferenceCounting = getenv("OSG_THREAD_SAFE_REF_UNREF")!=0;
#endif
// static std::auto_ptr<DeleteHandler> s_deleteHandler(0);
static DeleteHandlerPointer s_deleteHandler(0);

static ApplicationUsageProxy Referenced_e0(ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_THREAD_SAFE_REF_UNREF","");

void Referenced::setThreadSafeReferenceCounting(bool enableThreadSafeReferenceCounting)
{
#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    s_useThreadSafeReferenceCounting = enableThreadSafeReferenceCounting;
#endif
}

bool Referenced::getThreadSafeReferenceCounting()
{
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    return true;
#else
    return s_useThreadSafeReferenceCounting;
#endif
}


void Referenced::setDeleteHandler(DeleteHandler* handler)
{
    s_deleteHandler.reset(handler);
}

DeleteHandler* Referenced::getDeleteHandler()
{
    return s_deleteHandler.get();
}

#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
OpenThreads::Mutex& getNumObjectMutex()
{
    static OpenThreads::Mutex s_numObjectMutex;
    return s_numObjectMutex;
}
static int s_numObjects = 0;
#endif

Referenced::Referenced():
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
#ifndef ENFORCE_THREADSAFE
    if (s_useThreadSafeReferenceCounting)
#endif
        _refMutex = new OpenThreads::Mutex;
#endif
        
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        OSG_NOTICE<<"Object created, total num="<<s_numObjects<<std::endl;
    }
#endif

}

Referenced::Referenced(bool threadSafeRefUnref):
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
#ifndef ENFORCE_THREADSAFE
    if (threadSafeRefUnref)
#endif
        _refMutex = new OpenThreads::Mutex;
#endif

#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        OSG_NOTICE<<"Object created, total num="<<s_numObjects<<std::endl;
    }
#endif
}

Referenced::Referenced(const Referenced&):
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    _observerSet(0),
    _refCount(0)
#else
    _refMutex(0),
    _refCount(0),
    _observerSet(0)
#endif
{
#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
#ifndef ENFORCE_THREADSAFE
    if (s_useThreadSafeReferenceCounting)
#endif
        _refMutex = new OpenThreads::Mutex;
#endif

#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        ++s_numObjects;
        OSG_NOTICE<<"Object created, total num="<<s_numObjects<<std::endl;
    }
#endif
}

Referenced::~Referenced()
{
#ifdef DEBUG_OBJECT_ALLOCATION_DESTRUCTION
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(getNumObjectMutex());
        --s_numObjects;
        OSG_NOTICE<<"Object deleted, total num="<<s_numObjects<<std::endl;
    }
#endif

    if (_refCount>0)
    {
        OSG_WARN<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        OSG_WARN<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }

    // signal observers that we are being deleted.
    signalObserversAndDelete(false, true, false);

    // delete the ObserverSet
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (_observerSet.get()) delete static_cast<ObserverSet*>(_observerSet.get());
#else
    if (_observerSet) delete static_cast<ObserverSet*>(_observerSet);
#endif
}

ObserverSet* Referenced::getOrCreateObserverSet() const
{
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());
    while (0 == observerSet) {
        ObserverSet* newObserverSet = new ObserverSet;
        if (!_observerSet.assign(newObserverSet, 0))
            delete newObserverSet;
        observerSet = static_cast<ObserverSet*>(_observerSet.get());
    }
    return observerSet;
#else
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex);
        if (!_observerSet) _observerSet = new ObserverSet;
        return static_cast<ObserverSet*>(_observerSet);
    }
    else
    {
        if (!_observerSet) _observerSet = new ObserverSet;
        return static_cast<ObserverSet*>(_observerSet);
    }
#endif
}

void Referenced::signalObserversAndDelete(bool signalUnreferened, bool signalDelete, bool doDelete) const
{
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet.get());
#else
    ObserverSet* observerSet = static_cast<ObserverSet*>(_observerSet);
#endif

    if (observerSet)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*(observerSet->getObserverSetMutex()));
        if (signalUnreferened)
        {
            observerSet->signalObjectUnreferenced(const_cast<Referenced*>(this));
        }

        if (_refCount!=0)
        {
            OSG_NOTICE<<"Referenced::signalObserversAndDelete(,,) disabling delete as _refCount="<<_refCount<<std::endl;
            return;
        }

        if (signalDelete)
        {
            observerSet->signalObjectDeleted(const_cast<Referenced*>(this));
        }
    }

    if (doDelete && _refCount==0)
    {
        //OSG_NOTICE<<"Referenced::signalObserversAndDelete(,,) doing delete as _refCount="<<_refCount<<std::endl;
        if (getDeleteHandler()) deleteUsingDeleteHandler();
        else delete this;
    }
}


void Referenced::setThreadSafeRefUnref(bool threadSafe)
{
#if !defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    if (threadSafe)
    {
        if (!_refMutex)
        {
            // we want thread safe ref()/unref() so assign a mutex
            _refMutex = new OpenThreads::Mutex;
        }
    }
    else
    {
        if (_refMutex)
        {
            // we don't want thread safe ref()/unref() so remove any assigned mutex
            OpenThreads::Mutex* tmpMutexPtr = _refMutex;
            _refMutex = 0;
            delete tmpMutexPtr;
        }
    }
#endif
}

void Referenced::unref_nodelete() const
{
#if defined(_OSG_REFERENCED_USE_ATOMIC_OPERATIONS)
    bool needUnreferencedSignal = ((--_refCount) == 0);
#else
    bool needUnreferencedSignal = false;
    if (_refMutex)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_refMutex);
        needUnreferencedSignal = ((--_refCount) == 0);
    }
    else
    {
        needUnreferencedSignal = ((--_refCount) == 0);
    }
#endif

    if (needUnreferencedSignal)
    {
        signalObserversAndDelete(true,false,false);
    }
}

void Referenced::deleteUsingDeleteHandler() const
{
    getDeleteHandler()->requestDelete(this);
}

} // end of namespace osg
