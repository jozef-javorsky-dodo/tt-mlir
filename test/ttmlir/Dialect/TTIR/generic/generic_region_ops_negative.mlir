// RUN: not ttmlir-opt --split-input-file %s 2>&1 | FileCheck %s

#map = affine_map<(d0, d1) -> (d0, d1)>
#parallel = #tt.iterator_type<parallel>
#l1_alias = #tt.memory_space<l1>

func.func @reduce_dim_arg(%arg0: tensor<64x128xf32>, %arg1: tensor<64x128xf32>) -> tensor<64x128xf32> {
  %0 = ttir.empty() : tensor<64x128xf32>
  %1 = "ttir.generic"(%arg0, %arg1, %0) <{
      grid = #tt.grid<1x1>,
      indexing_maps = [#map, #map, #map],
      iterator_types = [#parallel, #parallel],
      threads = [#ttir.thread<compute>],
      operandSegmentSizes = array<i32: 2, 1>
      }> ({
      ^bb0(%arg2: memref<2x4x!tt.tile<32x32, f32>, #l1_alias>,
          %arg3: memref<2x4x!tt.tile<32x32, f32>, #l1_alias>,
          %arg4: memref<2x4x!tt.tile<32x32, f32>, #l1_alias>):
          linalg.generic {
              indexing_maps = [#map, #map],
              iterator_types = ["parallel", "parallel"]
              }
              ins(%arg2: memref<2x4x!tt.tile<32x32, f32>, #l1_alias>)
              outs(%arg4: memref<2x4x!tt.tile<32x32, f32>, #l1_alias>) {
              ^bb0(%a: !tt.tile<32x32, f32>, %b: !tt.tile<32x32, f32>):
                  // CHECK: error: 'ttir.tile_reduce_max' op requires attribute 'reduce_dim'
                  %4 = "ttir.tile_reduce_max" (%a, %b) : (!tt.tile<32x32, f32>, !tt.tile<32x32, f32>) -> !tt.tile<32x32, f32>
                  linalg.yield %4: !tt.tile<32x32, f32>
              }
      "ttir.yield"() : () -> ()
      }) : (tensor<64x128xf32>, tensor<64x128xf32>, tensor<64x128xf32>) -> tensor<64x128xf32>
  return %1 : tensor<64x128xf32>
}
