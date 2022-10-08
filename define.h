// Copyright 2022 Chunqing Shan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstddef>

static const size_t BLOCK_SIZE = 512;
static const size_t DICT_SIZE = 256 * 1024;

#ifdef BIGRAM
static const int Ratio = 192;
#else
static const int Ratio = 16;
#endif
