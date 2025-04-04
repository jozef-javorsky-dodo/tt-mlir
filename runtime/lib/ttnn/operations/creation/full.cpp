// SPDX-FileCopyrightText: (c) 2024 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#include "operations/creation/full.h"
#include "tt/runtime/detail/logger.h"
#include "tt/runtime/detail/ttnn.h"

#include "tt/runtime/ttnn/operations/utils.h"
#include "tt/runtime/ttnn/utils.h"
#include "tt/runtime/workarounds.h"

namespace tt::runtime::ttnn::operations::creation {
struct FullTensorConfig {
  ::ttnn::Shape shape;
  ::ttnn::DataType dtype;
  ::ttnn::Layout layout;
  float fillValue;
  uint32_t numShards;
  const ::tt::target::ttnn::DistributionStrategy *strategy = nullptr;
  std::optional<::ttnn::MemoryConfig> memoryConfig = std::nullopt;

  FullTensorConfig(const ::tt::target::ttnn::FullOp *op)
      : shape(::tt::runtime::ttnn::operations::utils::toTTNNShape(
            *op->out()->desc()->shape())),
        dtype(::tt::runtime::ttnn::operations::utils::getDataType(op->out())),
        layout(::tt::runtime::ttnn::utils::inferLayoutFromTileShape(op->out())),
        fillValue(op->fill_value()), numShards(op->num_shards()),
        strategy(op->strategy()) {

    if (!::tt::runtime::ttnn::utils::inSystemMemory(op->out())) {
      memoryConfig = ::tt::runtime::ttnn::utils::createMemoryConfigIfNeeded(
          ::tt::runtime::ttnn::utils::getTensorRefMemoryConfig(op->out()));
      LOG_ASSERT(memoryConfig.has_value(),
                 "Memory config must exist for device tensors");
    }
    validate();
  }

  void validate() const {
    LOG_ASSERT(strategy, "Strategy must be provided");
    LOG_ASSERT(numShards > 0, "Number of shards must be greater than 0");
    LOG_ASSERT(numShards == 1 ||
                   strategy->strategy_type() !=
                       ::tt::target::ttnn::DistributedTensorConfig::NONE,
               "Strategy must be provided when num shards is greater than 1");
    LOG_ASSERT(strategy->strategy_type() !=
                   ::tt::target::ttnn::DistributedTensorConfig::AllGatherTensor,
               "AllGatherTensor is not supported");
  }

  ::tt::tt_metal::DistributedTensorConfig distributedTensorConfig() const {
    return ::tt::runtime::ttnn::operations::utils::
        distributedTensorConfigFromFlatbuffer(strategy);
  }
};

static ::ttnn::Tensor
createFullOnMultiDevice(ProgramContext &context, FullTensorConfig &config,
                        const ::tt::target::DeviceRef *deviceRef) {
  ::tt::tt_metal::DistributedTensorConfig strategy =
      config.distributedTensorConfig();
  std::vector<::ttnn::Tensor> tensorShards;
  tensorShards.reserve(config.numShards);
  std::generate_n(std::back_inserter(tensorShards), config.numShards,
                  [&config]() -> ::ttnn::Tensor {
                    return ::ttnn::full(config.shape, config.fillValue,
                                        config.dtype, config.layout);
                  });
  ::ttnn::Tensor out = ::ttnn::distributed::create_multi_device_tensor(
      tensorShards, ::ttnn::StorageType::MULTI_DEVICE_HOST, strategy);
  if (deviceRef) {
    ::ttnn::MeshDevice &meshDevice = context.getSubMesh(deviceRef->global_id());
    LOG_ASSERT(config.numShards == meshDevice.num_devices());
    out = ::ttnn::to_device(out, &meshDevice, config.memoryConfig);
  }
  return out;
}

static ::ttnn::Tensor
createFullOnSingleDevice(ProgramContext &context, FullTensorConfig &config,
                         const ::tt::target::DeviceRef *deviceRef) {
  std::optional<std::reference_wrapper<::ttnn::IDevice>> device = std::nullopt;
  if (deviceRef) {
    ::ttnn::MeshDevice &subMesh = context.getSubMesh(deviceRef->global_id());
    LOG_ASSERT(subMesh.num_devices() == 1);
    LOG_ASSERT(config.memoryConfig.has_value());
    device = std::make_optional(std::ref(*subMesh.get_device(
        ::tt::tt_metal::distributed::MeshCoordinate(0, 0))));
  }
  return ::ttnn::full(config.shape, config.fillValue, config.dtype,
                      config.layout, device, config.memoryConfig);
}

void run(const ::tt::target::ttnn::FullOp *op, ProgramContext &context) {
  ProgramTensorPool &tensorPool = context.getTensorPool();
  FullTensorConfig config(op);
  ::ttnn::Tensor out;
  const ::tt::target::DeviceRef *deviceRef =
      !::tt::runtime::ttnn::utils::inSystemMemory(op->out()) ? op->device()
                                                             : nullptr;

  if (config.numShards == 1) {
    out = createFullOnSingleDevice(context, config, deviceRef);
  } else if (config.numShards > 1) {
    out = createFullOnMultiDevice(context, config, deviceRef);
  } else {
    LOG_FATAL("Unsupported num shards");
  }

  tensorPool.insertTTNNTensorAndValidate(op->out(), out);
}
} // namespace tt::runtime::ttnn::operations::creation
