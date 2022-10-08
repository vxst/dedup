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

#include <cstdio>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <cassert>
#include "./define.h"
#include "./deduplicator.h"
#include "./hash.h"

using std::vector, std::pair;
using std::domain_error;
using std::sort, std::make_pair, std::greater;


uint64_t deduplicator::hash_data(const uint8_t *data)
{
    const size_t &len = BLOCK_SIZE;
    uint32_t hash0 = hashw(data, len, 0);
    uint32_t hash1 = hashw(data, len, 0x740d023);
    uint64_t hash = hash0 | ((uint64_t)hash1 << 32);
    return hash;
}

deduplicator::deduplicator()
{
    FILE *random_file = fopen("/dev/urandom", "rb");
    // Magic first byte is not zero for faster decode
    for(*magic=0; (*magic & 0xff) == 0;)
        fread(magic, sizeof(uint64_t), 4, random_file);
    fclose(random_file);
}

deduplicator::deduplicator(const uint8_t *header){
    const uint8_t *p = build_data_map(header);
    header_size = p - header;
}

void deduplicator::prescan_block(const uint8_t *data)
{
    uint64_t hash = hash_data(data);
    if (hash_count.find(hash) == hash_count.end())
        hash_count[hash] = 1;
    else
        hash_count[hash]++;
}


void deduplicator::scan_block(const uint8_t *data)
{
    const size_t &len = BLOCK_SIZE;
    uint64_t hash = hash_data(data);
    if(hash_count.find(hash) == hash_count.end())
        return;
    if (hash_count[hash] > 1)
        data_map[hash] = vector<uint8_t>(data, data + len);
}

vector<uint8_t> deduplicator::get_header() const
{
    vector<uint8_t> buffer;

    // Filetype word
    buffer.push_back('D');
    buffer.push_back('E');
    buffer.push_back('D');
    buffer.push_back('U');
    buffer.push_back('P');
    buffer.push_back(0xb2);
    buffer.push_back(0xe1);
    buffer.push_back(0x7a);

    // Record the magic number
    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 8; j++)
            buffer.push_back(magic[i] >> (j * 8));

    // Record the dict map
    for (auto &p : data_map)
        // Hash can be recalculate from data, so we don't need to store it.
        buffer.insert(buffer.end(), p.second.begin(), p.second.end());

    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 8; j++)
            buffer.push_back(magic[i] >> (j * 8));
    return buffer;
}
bool deduplicator::check_magic(const uint8_t *data)const
{
    uint8_t first_byte = magic[0] & 0xff;
    if(*data != first_byte)
        return false;
    for (size_t i = 0; i < 4; i++)
    {
        uint64_t m = 0;
        for (size_t j = 0; j < 8; j++)
            m |= (uint64_t)data[i * 8 + j] << (j * 8);
        if (m != magic[i])
            return false;
    }
    return true;
}

const uint8_t* deduplicator::build_data_map(const uint8_t *data){
    auto err = domain_error("Invalid header");
    const size_t &len = BLOCK_SIZE;
    if(data[0] != 'D') throw err;
    if(data[1] != 'E') throw err;
    if(data[2] != 'D') throw err;
    if(data[3] != 'U') throw err;
    if(data[4] != 'P') throw err;
    if(data[5] != 0xb2) throw err;
    if(data[6] != 0xe1) throw err;
    if(data[7] != 0x7a) throw err;
    data += 8;
    for(size_t i = 0; i < 4; i++){
        magic[i] = 0;
        for(size_t j = 0; j < 8; j++)
            magic[i] |= (uint64_t)data[i * 8 + j] << (j * 8);
    }
    data += 32;

    while(!check_magic(data)){
        uint64_t hash = hash_data(data);
        data_map[hash] = vector<uint8_t>(data, data + len);
        data += len;
    }
    data += 32;

    return data;
}

size_t deduplicator::get_header_size()const{
    return header_size;
}

vector<uint8_t>* deduplicator::dedupe_block(const uint8_t *data) const
{
    uint64_t hash = hash_data(data);
    if (data_map.find(hash) == data_map.end()){
        return nullptr;
    }else{
        auto deduped_data = new vector<uint8_t>();
        deduped_data->reserve(32 + 8);
        // Magic number
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 8; j++)
                deduped_data->push_back(magic[i] >> (j * 8));

        // Hash
        for (size_t i = 0; i < 8; i++)
            deduped_data->push_back(hash >> (i * 8));
        return deduped_data;
    }
}

pair<const uint8_t*, const uint8_t*>
deduplicator::decode_data(const uint8_t* data)const{
    if(check_magic(data)){
        uint64_t hash = 0;
        data += 32;
        for(size_t i = 0; i < 8; i++)
            hash |= (uint64_t)data[i] << (i * 8);
        return make_pair(data_map.at(hash).data(), data + 8);
    }else{
        return make_pair(nullptr, data + BLOCK_SIZE);
    }
}

void deduplicator::reduce_hash_map(size_t threshold){
    vector<pair<uint32_t, uint64_t>> hash_count_vec;

    for(auto &p : hash_count)
        hash_count_vec.push_back(make_pair(p.second, p.first));
    sort(hash_count_vec.begin(), hash_count_vec.end(),
         greater<pair<uint32_t, uint32_t>>());

    for(int i = threshold; i < hash_count_vec.size(); i++)
        hash_count.erase(hash_count_vec[i].second);

    assert(hash_count.size() <= threshold);

    hash_count.reserve(threshold*3);
}
