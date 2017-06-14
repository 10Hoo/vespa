// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <memory>

namespace vespalib {

class Slime;

namespace slime { class Inserter; }

namespace tensor {

class Tensor;
class TensorBuilder;

/**
 * Class for serializing a tensor into a slime object.
 */
class SlimeBinaryFormat
{
public:
    static void serialize(slime::Inserter &inserter, const Tensor &tensor);
    static std::unique_ptr<Slime> serialize(const Tensor &tensor);
};

} // namespace vespalib::tensor
} // namespace vespalib
