#include "algo.h"
#include <cstdio>
#include <exception>

using namespace std;

static inline uint32_t murmur_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}
static uint32_t murmur3_32(const uint8_t *data, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k;
    for (size_t i = len >> 2; i; i--)
    {
        memcpy(&k, data, sizeof(uint32_t));
        data += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    k = 0;
    for (size_t i = len & 3; i; i--)
    {
        k <<= 8;
        k |= data[i - 1];
    }
    h ^= murmur_32_scramble(k);
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}
static uint64_t hash_data(const uint8_t *data)
{
    const size_t &len = BLOCK_SIZE;
    uint32_t hash0 = murmur3_32(data, len, 0);
    uint32_t hash1 = murmur3_32(data, len, 0x740d023);
    uint64_t hash = hash0 | ((uint64_t)hash1 << 32);
    return hash;
}
builder::builder()
{
    FILE *random_file = fopen("/dev/urandom", "rb");
    fread(magic, sizeof(uint64_t), 4, random_file);
    fclose(random_file);
}

void builder::prescan_block(const uint8_t *data)
{
    uint64_t hash = hash_data(data);
    if (hash_count.find(hash) == hash_count.end())
        hash_count[hash] = 1;
    else
        hash_count[hash]++;
}

void builder::scan_block(const uint8_t *data)
{
    const size_t &len = BLOCK_SIZE;
    uint64_t hash = hash_data(data);
    if (hash_count[hash] > 1)
        data_map[hash] = vector<uint8_t>(data, data + len);
}

vector<uint8_t> builder::get_header() const
{
    vector<uint8_t> dict;

    // Filetype word
    dict.push_back('D');
    dict.push_back('E');
    dict.push_back('D');
    dict.push_back('U');
    dict.push_back('P');
    dict.push_back(0xb2);
    dict.push_back(0xe1);
    dict.push_back(0x7a);

    // Record the magic number
    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 8; j++)
            dict.push_back(magic[i] >> (j * 8));
    
    // Record the dict map
    for (auto &p : data_map)
        // Hash can be recalculate from data, so we don't need to store it.
        dict.insert(dict.end(), p.second.begin(), p.second.end());

    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 8; j++)
            dict.push_back(magic[i] >> (j * 8));
    return dict;
}
static bool check_magic(const uint8_t *data, const uint64_t *magic)
{
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
const uint8_t* builder::build_data_map(const uint8_t *data){
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

    while(!check_magic(data, magic)){
        uint64_t hash = hash_data(data);
        data_map[hash] = vector<uint8_t>(data, data + len);
        data += len;
    }
    data += 32;

    return data;
}

vector<uint8_t>* builder::get_deduped_data(const uint8_t *data) const
{
    const size_t &len = BLOCK_SIZE;
    uint64_t hash = hash_data(data);
    if (data_map.find(hash) == data_map.end())
        return nullptr;
    else{
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

pair<vector<uint8_t>*, const uint8_t*> builder::decode_data(const uint8_t* data)const{
    if(check_magic(data, magic)){
        uint64_t hash;
        data += 32;
        for(size_t i = 0; i < 8; i++)
            hash |= (uint64_t)data[i] << (i * 8);
        vector<uint8_t>* result = new vector<uint8_t>(data_map.at(hash));
        return make_pair(result, data + 8);
    }else{
        return make_pair(nullptr, data + BLOCK_SIZE);
    }
}

