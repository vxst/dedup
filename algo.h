#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

// assume 512 bytes aligned(like tar/disk), deduped data is 32 bytes + 8 bytes
// there can be 256k of deduped items
// dict size is 512 * 256k = 128MB

const static size_t BLOCK_SIZE = 512;
const static size_t DICT_SIZE = 256 * 1024;
class builder
{
private:
    std::unordered_map<uint64_t, uint32_t> hash_count;
    std::unordered_map<uint64_t, std::vector<uint8_t>> data_map;
public:
    uint64_t magic[4]; // 256 bit magic number for hash leading value
    builder();
    void prescan_block(const uint8_t *data);
    void scan_block(const uint8_t *data);
    std::vector<uint8_t> get_header()const;
    const uint8_t* build_data_map(const uint8_t *data);
    void reduce_hash_map();
    std::vector<uint8_t>* get_deduped_data(const uint8_t *data)const;
    std::pair<std::vector<uint8_t>*, const uint8_t*> 
        decode_data(const uint8_t *data)const;
};

void encode(FILE* infile, FILE* outfile);
void decode(FILE* infile, FILE* outfile);