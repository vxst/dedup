#include "algo.h"
#include <cstdio>
#include <exception>
#include <algorithm>

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
        uint64_t hash = 0;
        data += 32;
        for(size_t i = 0; i < 8; i++)
            hash |= (uint64_t)data[i] << (i * 8);
        vector<uint8_t>* result = new vector<uint8_t>(data_map.at(hash));
        return make_pair(result, data + 8);
    }else{
        return make_pair(nullptr, data + BLOCK_SIZE);
    }
}

void builder::reduce_hash_map(){
    vector<pair<uint64_t, uint64_t>> hash_count_vec;
    for(auto &p : hash_count)
        hash_count_vec.push_back(make_pair(p.second, p.first));
    std::sort(hash_count_vec.begin(), hash_count_vec.end(), greater<pair<uint64_t, uint64_t>>());
    for(int i = DICT_SIZE; i < hash_count_vec.size(); i++)
        hash_count.erase(hash_count_vec[i].second);
    assert(hash_count.size() <= DICT_SIZE);
}


void encode(FILE* infile, FILE* outfile){
    builder b;
    uint8_t data[BLOCK_SIZE];
    while(fread(data, 1, BLOCK_SIZE, infile) == BLOCK_SIZE){
        b.prescan_block(data);
    }
    fseek(infile, 0, SEEK_SET);
    b.reduce_hash_map();
    while(fread(data, 1, BLOCK_SIZE, infile) == BLOCK_SIZE){
        b.scan_block(data);
    }
    fseek(infile, 0, SEEK_SET);
    auto header = b.get_header();
    fwrite(header.data(), 1, header.size(), outfile);
    int count = 0;
    while((count = fread(data, 1, BLOCK_SIZE, infile)) == BLOCK_SIZE){
        auto deduped_data = b.get_deduped_data(data);
        if(deduped_data == nullptr){
            fwrite(data, 1, BLOCK_SIZE, outfile);
        }else{
            fwrite(deduped_data->data(), 1, deduped_data->size(), outfile);
            delete deduped_data;
        }
    }
    fwrite(data, 1, count, outfile);
}
void decode(FILE* infile, FILE* outfile){
    uint8_t* header_data = new uint8_t[BLOCK_SIZE * DICT_SIZE + BLOCK_SIZE];
    fread(header_data, 1, BLOCK_SIZE * DICT_SIZE + BLOCK_SIZE, infile);
    builder b;
    const uint8_t* pos_data = b.build_data_map(header_data);
    size_t pos = pos_data - header_data;
    delete[] header_data;
    uint8_t data[BLOCK_SIZE];
    fseek(infile, pos, SEEK_SET);
    while(1){
        memset(data, 0, BLOCK_SIZE);
        long pos = ftell(infile);
        long size = fread(data, 1, BLOCK_SIZE, infile);
        bool is_magic = check_magic(data, b.magic);

        if(is_magic){
            auto decoded_data = b.decode_data(data);
            fwrite(decoded_data.first->data(), 1, decoded_data.first->size(), outfile);
            delete decoded_data.first;
            fseek(infile, pos + 40, SEEK_SET);
        }else{
            fwrite(data, 1, size, outfile);
        }
        if(size != BLOCK_SIZE && !is_magic)
            break;
    }
}