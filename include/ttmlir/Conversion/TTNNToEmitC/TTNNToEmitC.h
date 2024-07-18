// SPDX-FileCopyrightText: (c) 2024 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#ifndef TTMLIR_CONVERSION_TTNNTOEMITC_TTNNTOEMITC_H
#define TTMLIR_CONVERSION_TTNNTOEMITC_TTNNTOEMITC_H

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir::tt {

std::unique_ptr<OperationPass<func::FuncOp>> createConvertTTNNToEmitCPass();

} // namespace mlir::tt

#endif // TTMLIR_CONVERSION_TTNNTOEMITC_TTNNTOEMITC_H