#pragma once

#include <unordered_map>
#include <vector>
#include <cstdint>

const static size_t BLOCK_SIZE = 4 * 1024 * 1024;
class builder
{
private:
    uint64_t magic[4]; // 256 bit magic number for hash leading value
    std::unordered_map<uint64_t, uint32_t> hash_count;
    std::unordered_map<uint64_t, std::vector<uint8_t>> data_map;
public:
    builder();
    void prescan_block(const uint8_t *data);
    void scan_block(const uint8_t *data);
    vector<uint8_t> get_header()const;
    const uint8_t* build_data_map(const uint8_t *data);
    vector<uint8_t>* get_deduped_data(const uint8_t *data)const;
    std::pair<vector<uint8_t>*, const uint8_t*> decode_data(const uint8_t *data)const;
};

void encode(FILE* infile, FILE* outfile);
void decode(FILE* infile, FILE* outfile);