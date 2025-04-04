// SPDX-FileCopyrightText: (c) 2024 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0
#include "operations/eltwise/unary/unary_composite.h"
#include "tt/runtime/detail/logger.h"
#include "tt/runtime/detail/ttnn.h"

#include "tt/runtime/ttnn/operations/eltwise/unary/utils.h"
#include "tt/runtime/ttnn/operations/utils.h"
#include "tt/runtime/ttnn/utils.h"
#include "ttnn/operations/eltwise/unary/unary_composite.hpp"

namespace tt::runtime::ttnn::operations::unary::composite {

static void runEltwiseUnaryCompositeOp(
    const ::tt::target::ttnn::EltwiseOp *op, ProgramTensorPool &tensorPool,
    const std::function<::ttnn::Tensor(
        const ::ttnn::Tensor &, const std::optional<::ttnn::MemoryConfig> &)>
        &ttnnOp) {

  ::ttnn::Tensor *in = nullptr;

  getEltwiseUnaryOpInputTensor(op, tensorPool, &in);

  std::optional<::ttnn::MemoryConfig> outputMemoryConfig =
      ::tt::runtime::ttnn::utils::createMemoryConfigIfNeeded(
          ::tt::runtime::ttnn::utils::getTensorRefMemoryConfig(op->out()));
  LOG_ASSERT(::tt::runtime::ttnn::utils::inSystemMemory(op->out()) ||
                 outputMemoryConfig.has_value(),
             "Memory config must exist for device tensors");

  ::ttnn::Tensor out = ttnnOp(*in, outputMemoryConfig);

  tensorPool.insertTTNNTensorAndValidate(op->out(), out);
}

static void
runEltwiseUnaryCompositeClampScalarOp(const ::tt::target::ttnn::EltwiseOp *op,
                                      ProgramTensorPool &tensorPool) {
  ::ttnn::Tensor *in = nullptr;

  getEltwiseUnaryOpInputTensor(op, tensorPool, &in);

  float min = op->params_as_ClampScalarOpParams()->min();
  float max = op->params_as_ClampScalarOpParams()->max();

  std::optional<::ttnn::MemoryConfig> outputMemoryConfig =
      ::tt::runtime::ttnn::utils::createMemoryConfigIfNeeded(
          ::tt::runtime::ttnn::utils::getTensorRefMemoryConfig(op->out()));
  LOG_ASSERT(::tt::runtime::ttnn::utils::inSystemMemory(op->out()) ||
                 outputMemoryConfig.has_value(),
             "Memory config must exist for device tensors");

  ::ttnn::Tensor out = ::ttnn::clamp(*in, min, max, outputMemoryConfig);

  tensorPool.insertTTNNTensorAndValidate(op->out(), out);
}

static void
runEltwiseUnaryCompositeClampTensorOp(const ::tt::target::ttnn::EltwiseOp *op,
                                      ProgramTensorPool &tensorPool) {
  ::ttnn::Tensor *in = nullptr;

  getEltwiseUnaryOpInputTensor(op, tensorPool, &in);

  ::ttnn::Tensor min = tensorPool.getTTNNTensorAndValidate(
      op->params_as_ClampTensorOpParams()->min());
  ::ttnn::Tensor max = tensorPool.getTTNNTensorAndValidate(
      op->params_as_ClampTensorOpParams()->max());

  std::optional<::ttnn::MemoryConfig> outputMemoryConfig =
      ::tt::runtime::ttnn::utils::createMemoryConfigIfNeeded(
          ::tt::runtime::ttnn::utils::getTensorRefMemoryConfig(op->out()));
  LOG_ASSERT(::tt::runtime::ttnn::utils::inSystemMemory(op->out()) ||
                 outputMemoryConfig.has_value(),
             "Memory config must exist for device tensors");

  ::ttnn::Tensor out = ::ttnn::clamp(*in, min, max, outputMemoryConfig);

  tensorPool.insertTTNNTensorAndValidate(op->out(), out);
}

void run(const ::tt::target::ttnn::EltwiseOp *op, ProgramContext &context) {
  ProgramTensorPool &tensorPool = context.getTensorPool();
  switch (op->type()) {
  case ::tt::target::ttnn::EltwiseOpType::Cbrt: {
    runEltwiseUnaryCompositeOp(op, tensorPool, ::ttnn::cbrt);
    break;
  }
  case ::tt::target::ttnn::EltwiseOpType::ClampScalar: {
    runEltwiseUnaryCompositeClampScalarOp(op, tensorPool);
    break;
  }
  case ::tt::target::ttnn::EltwiseOpType::ClampTensor: {
    runEltwiseUnaryCompositeClampTensorOp(op, tensorPool);
    break;
  }
  case ::tt::target::ttnn::EltwiseOpType::Log1p: {
    runEltwiseUnaryCompositeOp(op, tensorPool, ::ttnn::log1p);
    break;
  }
  default:
    LOG_FATAL("Unsupported Eltwise Binary Composite operation");
  }
}

} // namespace tt::runtime::ttnn::operations::unary::composite
