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
CFLAGS=-O3 -std=c++17 -Wall

dedup: main.o deduplicator.o hash.o coder.o
	$(CXX) $(CFLAGS) -o dedup main.o deduplicator.o hash.o coder.o

main.o: main.cpp
	$(CXX) $(CFLAGS) -c main.cpp

deduplicator.o: deduplicator.cpp
	$(CXX) $(CFLAGS) -c deduplicator.cpp

hash.o: hash.cpp
	$(CXX) $(CFLAGS) -c hash.cpp

coder.o: coder.cpp
	$(CXX) $(CFLAGS) -c coder.cpp

test: clean dedup
	xzcat test_data.xz > A
	./dedup -e A B
	./dedup -d B C
	ls -lh A B C
	diff A C

clean:
	rm -rf *.o dedup A B C
