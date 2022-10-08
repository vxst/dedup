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

#include "./hash.h"
#include <cstring>

static inline uint32_t hashw_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t hashw(const uint8_t *data, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k;
    for (size_t i = len >> 2; i; i--)
    {
        memcpy(&k, data, sizeof(uint32_t));
        data += sizeof(uint32_t);
        h ^= hashw_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    k = 0;
    for (size_t i = len & 3; i; i--)
    {
        k <<= 8;
        k |= data[i - 1];
    }
    h ^= hashw_scramble(k);
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
