# Deduplication tool

Remove duplicate blocks for a file.

 * Extremely fast, 1 GB/s for most computer core
 * Small memory footprint, less than 1 GB memory is required for TB level files
 * Aligned to block, suitable to deduplicate tar file

## Build

make && mv dedup ./your/bin/dir

## Usage

Build deduplicated file: 

```bash
./dedup -e input_file output_file
```

Restore a file from a deduplicated file:

```bash
./dedup -d input_file output_file
```

## License
```
 Copyright 2022 Chunqing Shan
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
     http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
```
