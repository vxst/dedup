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

#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <utility>

// assume 512 bytes aligned(like tar/disk), deduped data is 32 bytes + 8 bytes
// there can be 256k of deduped items
// dict size is 512 * 256k = 128MB

class deduplicator
{
 private:
  std::unordered_map<uint64_t, uint32_t> hash_count;
  std::unordered_map<uint64_t, std::vector<uint8_t>> data_map;
  size_t header_size;

  const uint8_t *build_data_map(const uint8_t *data);
  static uint64_t hash_data(const uint8_t *data);
  uint64_t magic[4];  // 256 bit magic number for hash leading value

 public:
  deduplicator();
  explicit deduplicator(const uint8_t *);

  void prescan_block(const uint8_t *data);
  void scan_block(const uint8_t *data);
  std::vector<uint8_t> *dedupe_block(const uint8_t *data) const;

  void reduce_hash_map(size_t threshold);

  std::vector<uint8_t> get_header() const;
  size_t get_header_size() const;
  bool check_magic(const uint8_t *data) const;

  std::pair<const uint8_t *, const uint8_t *>
  decode_data(const uint8_t *data) const;
};

void encode(FILE *infile, FILE *outfile);
void decode(FILE *infile, FILE *outfile);
