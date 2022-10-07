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

#include "algo.h"
#include <cstdio>
#include <cstring>

int main(int argc, char **argv)
{
    if (argc < 3){
        printf("Usage: %s [-e|-d] <input file> <output file>", argv[0]);
        return 1;
    }

    int is_encode = 0;
    if(strcmp(argv[1], "-e") == 0){
        is_encode = 1;
    }else if(strcmp(argv[1], "-d") == 0){
        is_encode = 0;
    }else{
        printf("Usage: %s [-e|-d] <input file> <output file>", argv[0]);
        return 1;
    }

    if(is_encode){
        FILE *infile = fopen(argv[2], "rb");
        FILE *outfile = fopen(argv[3], "wb");
        encode(infile, outfile);
        fclose(infile);
        fclose(outfile);
    }else{
        FILE *infile = fopen(argv[2], "rb");
        FILE *outfile = fopen(argv[3], "wb");
        decode(infile, outfile);
        fclose(infile);
        fclose(outfile);
    }
    return 0;
}