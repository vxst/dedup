#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

// assume 4KB aligned, deduped data is 32 bytes + 8 bytes
// there can be 128k of deduped items
// dict size is 4KB * 128k = 512MB

const static size_t BLOCK_SIZE = 4 * 1024;
const static size_t DICT_SIZE = 128 * 1024;
class builder
{
private:
    std::unordered_map<uint64_t, uint64_t> hash_count;
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