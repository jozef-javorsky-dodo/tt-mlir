// SPDX-FileCopyrightText: (c) 2024 Tenstorrent AI ULC
//
// SPDX-License-Identifier: Apache-2.0

#include "tt/runtime/detail/debug.h"

#if defined(TT_RUNTIME_DEBUG) && TT_RUNTIME_DEBUG == 1

namespace tt::runtime::debug {

Env const &Env::get(bool loadKernelsFromDisk) {
  static Env config(loadKernelsFromDisk);
  return config;
}

Hooks const &Hooks::get(
    std::optional<std::function<void(Binary, CallbackContext, OpContext)>>
        operatorCallback) {
  static Hooks config(operatorCallback);
  return config;
}

} // namespace tt::runtime::debug

#endif
