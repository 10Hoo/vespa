// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/storage/bucketdb/bucketdatabase.h>
#include <vespa/storage/distributor/maintenance/prioritizedbucket.h>
#include <boost/iterator/iterator_facade.hpp>
#include <vespa/vespalib/util/linkedptr.h>

namespace storage {
namespace distributor {

class BucketPriorityDatabase
{
protected:
    class ConstIteratorImpl
    {
    public:
        virtual ~ConstIteratorImpl() { }
        virtual void increment() = 0;

        virtual bool equal(const ConstIteratorImpl& other) const = 0;

        virtual PrioritizedBucket dereference() const = 0;
    };

    typedef vespalib::LinkedPtr<ConstIteratorImpl> ConstIteratorImplPtr;
public:
    class ConstIterator
        : public boost::iterator_facade<
              ConstIterator,
              PrioritizedBucket const,
              boost::forward_traversal_tag,
              PrioritizedBucket
        >
    {
        ConstIteratorImplPtr _impl;
    public:
        ConstIterator(const ConstIteratorImplPtr& impl)
            : _impl(impl)
        {}

        virtual ~ConstIterator() {}
    private:
        friend class boost::iterator_core_access;

        void increment() {
            _impl->increment();
        }

        bool equal(const ConstIterator& other) const {
            return _impl->equal(*other._impl);
        }

        PrioritizedBucket dereference() const {
            return _impl->dereference();
        }
    };

    typedef ConstIterator const_iterator;

    virtual ~BucketPriorityDatabase() { }
    
    virtual const_iterator begin() const = 0;

    virtual const_iterator end() const  = 0;

    virtual void setPriority(const PrioritizedBucket&) = 0;
};

}
}



