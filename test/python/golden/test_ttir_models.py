# SPDX-FileCopyrightText: (c) 2024 Tenstorrent AI ULC
#
# SPDX-License-Identifier: Apache-2.0

# RUN: SYSTEM_DESC_PATH=%system_desc_path% %python %s

import inspect

from ttmlir.test_utils import compile_to_flatbuffer, set_output_path
from ttmlir.ttir_builder import Operand, TTIRBuilder


@compile_to_flatbuffer(
    [
        (1, 784),
        (784, 256),
        (1, 256),
        (256, 10),
        (1, 10),
    ],
    targets=["ttnn"],
)
def test_mnist(
    in0: Operand,  # Input 28x28 image
    in1: Operand,  # Weight 1
    in2: Operand,  # Bias 1
    in3: Operand,  # Weight 2
    in4: Operand,  # Bias 2
    builder: TTIRBuilder,
):
    matmul_1 = builder.matmul(in0, in1)
    add_2 = builder.add(matmul_1, in2)
    relu_3 = builder.relu(add_2)
    matmul_5 = builder.matmul(relu_3, in3)
    add_6 = builder.add(matmul_5, in4)
    return builder.softmax(add_6, dimension=1)


@compile_to_flatbuffer(
    [
        (1, 12, 3200),  # arg0
        (1, 1, 12, 12),  # arg1
        (1, 12),  # arg2
        (1, 50, 1),  # arg3
        (1, 32, 50, 100),  # arg4
        (1, 1),  # arg5
        (1, 32, 50, 100),  # arg6
        (1, 32, 50, 100),  # arg7
        (1, 1),  # arg8
        (1, 32, 50, 100),  # arg9
        (1, 1),  # arg10
        (3200, 3200),  # arg11
        (3200, 3200),  # arg12
        (3200, 3200),  # arg13
        (3200, 3200),  # arg14
    ],
    targets=["ttnn"],
    module_dump=True,
)
def test_llama_attention(
    arg0: Operand,
    arg1: Operand,
    arg2: Operand,
    arg3: Operand,
    arg4: Operand,
    arg5: Operand,
    arg6: Operand,
    arg7: Operand,
    arg8: Operand,
    arg9: Operand,
    arg10: Operand,
    arg11: Operand,
    arg12: Operand,
    arg13: Operand,
    arg14: Operand,
    builder: TTIRBuilder,
):

    output1 = builder.squeeze(arg0, 0)
    output3 = builder.matmul(output1, arg11)
    output5 = builder.reshape(output3, (1, 12, 32, 100))
    output7 = builder.transpose(output5, -3, -2)
    output9 = builder.unsqueeze(arg2, 1)
    output11 = builder.matmul(arg3, output9)
    output13 = builder.transpose(output11, -2, -1)
    output15 = builder.concat([output13, output13], -1)
    output17 = builder.cos(output15)
    output19 = builder.unsqueeze(output17, 1)
    output21 = builder.multiply(output7, output19)
    output23 = builder.transpose(output7, -2, -1)
    output25 = builder.matmul(arg4, output23)
    output27 = builder.transpose(output25, -2, -1)
    output29 = builder.multiply(output27, arg5)
    output31 = builder.transpose(output7, -2, -1)
    output33 = builder.matmul(arg6, output31)
    output35 = builder.transpose(output33, -2, -1)
    output37 = builder.concat([output29, output35], -1)
    output39 = builder.sin(output15)
    output41 = builder.unsqueeze(output39, 1)
    output43 = builder.multiply(output37, output41)
    output45 = builder.add(output21, output43)
    output47 = builder.squeeze(output45, 0)
    output49 = builder.matmul(output1, arg12)
    output51 = builder.reshape(output49, (1, 12, 32, 100))
    output53 = builder.transpose(output51, -3, -2)
    output55 = builder.multiply(output53, output19)
    output57 = builder.transpose(output53, -2, -1)
    output59 = builder.matmul(arg7, output57)
    output61 = builder.transpose(output59, -2, -1)
    output63 = builder.multiply(output61, arg8)
    output65 = builder.transpose(output53, -2, -1)
    output67 = builder.matmul(arg9, output65)
    output69 = builder.transpose(output67, -2, -1)
    output71 = builder.concat([output63, output69], -1)
    output73 = builder.multiply(output71, output41)
    output75 = builder.add(output55, output73)
    output77 = builder.squeeze(output75, 0)
    output79 = builder.transpose(output77, -2, -1)
    output81 = builder.matmul(output47, output79)
    output83 = builder.unsqueeze(output81, 0)
    output85 = builder.multiply(output83, arg10)
    output87 = builder.add(output85, arg1)
    output89 = builder.softmax(output87, -1)
    output91 = builder.squeeze(output89, 0)
    output93 = builder.matmul(output1, arg13)
    output95 = builder.reshape(output93, (1, 12, 32, 100))
    output97 = builder.transpose(output95, -3, -2)
    output99 = builder.transpose(output97, -2, -1)
    output101 = builder.squeeze(output99, 0)
    output103 = builder.transpose(output101, -2, -1)
    output105 = builder.matmul(output91, output103)
    output107 = builder.unsqueeze(output105, 0)
    output109 = builder.transpose(output107, -3, -2)
    output111 = builder.reshape(output109, (12, 3200))
    output113 = builder.matmul(output111, arg14)
    output115 = builder.unsqueeze(output113, 0)

    return output115


if __name__ == "__main__":
    import argparse, os

    parser = argparse.ArgumentParser(description="Run TTIR Builder Model tests")
    parser.add_argument(
        "--path",
        type=str,
        help="Optional output path for the flatbuffer. Creates path if supplied path doesn't exist",
    )
    args = parser.parse_args()

    if args.path and os.path.exists(args.path):
        if not os.path.exists(args.path):
            os.makedirs(args.path)
        set_output_path(args.path)

    test_functions = inspect.getmembers(
        inspect.getmodule(inspect.currentframe()), inspect.isfunction
    )

    for function_name, func in test_functions:
        if function_name.startswith("test_"):
            func()
