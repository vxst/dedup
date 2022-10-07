#!/bin/bash

# Copyright 2022 Chunqing Shan
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CXX=clang++
CFLAGS=-O3 -std=c++14

dedup: main.o algo.o
	$(CXX) $(CFLAGS) -o dedup main.o algo.o

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

algo.o: algo.cpp
	$(CXX) $(CFLAGS) -c algo.cpp

clean:
	rm -rf *.o dedup A B C

test: clean dedup
	xzcat test_data.xz > A
	./dedup -e A B
	./dedup -d B C
	sha1sum A C
