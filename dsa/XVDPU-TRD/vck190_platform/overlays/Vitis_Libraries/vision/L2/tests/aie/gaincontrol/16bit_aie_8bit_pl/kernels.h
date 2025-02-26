/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689

#include <adf/window/types.h>
#include <adf/stream/types.h>

template <int code>
void gaincontrol(input_window_int16* input, output_window_int16* output, const int16_t& rgain, const int16_t& bgain);
