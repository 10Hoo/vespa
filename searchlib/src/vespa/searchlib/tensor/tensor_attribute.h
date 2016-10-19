// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/searchlib/attribute/not_implemented_attribute.h>
#include "tensor_store.h"
#include <vespa/searchlib/common/rcuvector.h>
#include <vespa/vespalib/tensor/tensor_mapper.h>

namespace vespalib { namespace tensor { class Tensor; } }

namespace search {

namespace attribute {

/**
 * Attribute vector class used to store tensors for all documents in memory.
 */
class TensorAttribute : public NotImplementedAttribute
{
protected:
    using RefType = TensorStore::RefType;
    using RefVector = RcuVectorBase<RefType>;

    RefVector _refVector; // docId -> ref in data store for serialized tensor
    TensorStore &_tensorStore; // data store for serialized tensors
    std::unique_ptr<vespalib::tensor::TensorMapper> _tensorMapper; // mapper to our tensor type
    uint64_t    _compactGeneration; // Generation when last compact occurred

    void compactWorst();
    void setTensorRef(DocId docId, RefType ref);
public:
    using RefCopyVector = vespalib::Array<RefType>;
    using Tensor = vespalib::tensor::Tensor;
    TensorAttribute(const vespalib::stringref &baseFileName, const Config &cfg,
                    TensorStore &tensorStore);
    virtual ~TensorAttribute();
    virtual uint32_t clearDoc(DocId docId) override;
    virtual void onCommit() override;
    virtual void onUpdateStat() override;
    virtual void removeOldGenerations(generation_t firstUsed) override;
    virtual void onGenerationChange(generation_t generation) override;
    virtual bool addDoc(DocId &docId) override;
    std::unique_ptr<Tensor> getEmptyTensor() const;
    virtual void clearDocs(DocId lidLow, DocId lidLimit) override;
    virtual void onShrinkLidSpace() override;
    virtual uint32_t getVersion() const override;
    RefCopyVector getRefCopy() const;
    virtual void setTensor(DocId docId, const Tensor &tensor) = 0;
    virtual std::unique_ptr<Tensor> getTensor(DocId docId) const = 0;
};


}  // namespace search::attribute

}  // namespace search
