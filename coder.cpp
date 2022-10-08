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

#include <cstddef>
#include <cstdint>

#include "./deduplicator.h"
#include "./define.h"
#include "./coder.h"

void encode(FILE* infile, FILE* outfile){
    deduplicator d;
    uint8_t data[BLOCK_SIZE];
    int32_t in_count = 0;

    while(fread(data, 1, BLOCK_SIZE, infile) == BLOCK_SIZE){
        d.prescan_block(data);
        in_count++;
        if(in_count >= DICT_SIZE * Ratio * 3){
            d.reduce_hash_map(DICT_SIZE * Ratio);
            in_count = 0;
        }
    }

    d.reduce_hash_map(DICT_SIZE);

    for(fseek(infile, 0, SEEK_SET);
        fread(data, 1, BLOCK_SIZE, infile) == BLOCK_SIZE;
        d.scan_block(data))
        continue;

    auto header = d.get_header();
    fwrite(header.data(), 1, header.size(), outfile);

    uint8_t *blkdata;
    size_t blklen, count;

    for(
            fseek(infile, 0, SEEK_SET);
            (count = fread(data, 1, BLOCK_SIZE, infile));
            fwrite(blkdata, 1, blklen, outfile)
        ){
        if (count != BLOCK_SIZE){
            blkdata = data;
            blklen = count;
            continue;
        }
        auto deduped_data = d.dedupe_block(data);
        if(deduped_data == nullptr){
            blkdata = data;
            blklen = BLOCK_SIZE;
        }else{
            blkdata = deduped_data->data();
            blklen = deduped_data->size();
        }
    }
}

void decode(FILE* infile, FILE* outfile){
    uint8_t* header_data = new uint8_t[BLOCK_SIZE * DICT_SIZE + BLOCK_SIZE];
    fread(header_data, 1, BLOCK_SIZE * DICT_SIZE + BLOCK_SIZE, infile);
    deduplicator d(header_data);
    size_t pos = d.get_header_size();
    delete[] header_data;

    uint8_t data[BLOCK_SIZE];
    fseek(infile, pos, SEEK_SET);

    while(1){
        data[0] = 0;
        int64_t size = fread(data, 1, BLOCK_SIZE, infile);
        bool is_magic = d.check_magic(data);

        if(is_magic){
            auto decoded_data = d.decode_data(data).first;
            fwrite(decoded_data, 1, BLOCK_SIZE, outfile);
            fseek(infile, pos + 40, SEEK_SET);
            pos += 40;
        }else{
            fwrite(data, 1, size, outfile);
            pos += size;
        }
        if(size != BLOCK_SIZE && !is_magic)
            break;
    }
}
