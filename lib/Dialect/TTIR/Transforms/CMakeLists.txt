add_mlir_dialect_library(MLIRTTIRTransforms
        Allocate.cpp
        Broadcast.cpp
        Constant.cpp
        GeneralizeNamedOps.cpp
        GenericLinearizeMemref.cpp
        GenericGenerateDatamovement.cpp
        GenericGenerateLoops.cpp
        GenericHWThreadSelection.cpp
        GenericLowerAffineDMAs.cpp
        AttachMetalLayout.cpp
        OptimizeTensorLayout.cpp
        HoistCPUOps.cpp
        Bufferization.cpp
        Layout.cpp
        Transforms.cpp

        ADDITIONAL_HEADER_DIRS
        ${PROJECT_SOURCE_DIR}/include/ttmlir

        DEPENDS
        MLIRTTIROpsIncGen
        MLIRTTIRPassesIncGen
        MLIRTTOpsIncGen
        )
