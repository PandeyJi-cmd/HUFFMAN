# HUF1: Streaming File Compressor in C++

A fast, two-pass binary file compressor and decompressor built from scratch in **C++20**. Instead of loading the entire file into RAM at once, it processes files in fixed **64 KB chunks**, keeping memory usage strictly constant ($O(1)$) regardless of file size.

## How It Works

* **Constant Memory Streaming (`64 KB` Chunks):** Reads and writes files in `65,536`-byte blocks. Whether you compress a `4 MB` file or a `10 GB` file, the program only uses around **128 KB of RAM**.
* **64-Bit Integer Codes (`__builtin_clzll`):** Instead of storing Huffman codes as slow `std::string` objects of `'0'` and `'1'`, each code is stored inside a single `uint64_t` integer with a leading `1` marker bit. The exact bit length is calculated in 1 CPU instruction using `63 - __builtin_clzll(code)`.
* **Custom `HUF1` Binary Header:**
  * **4 Bytes (`"HUF1"`):** Magic file signature used to automatically detect whether an input file needs to be compressed or decompressed.
  * **8 Bytes (`total_bytes`):** Stores the exact original file size so trailing padding bits in the last byte are ignored during decompression.
  * **2 Bytes (`unique_chars`):** Stores the count of unique bytes present in the file, followed by each `(byte, 64-bit code)` pair.
* **Fast Binary-Search Decoding:** During decompression, the `(code, byte)` table is sorted once, and incoming bits are matched using `std::lower_bound` binary search until all original bytes are restored.

## Results (4.0 MB Test File)

| Metric | Value |
| :--- | :--- |
| **Original File Size** | `4.0 MB` (`4,194,304` bytes) |
| **Compressed `.huf` Size** | `2.6 MB` (`2,726,298` bytes) |
| **Size Reduction** | **~35%** (`1.54x` smaller) |
| **RAM Usage** | **128 KB** (Constant $O(1)$ memory) |
| **Data Integrity** | **100% exact match** (`sha256sum` verified) |

## Build & Run

### 1. Compile
```bash
g++ -std=c++20 -O3 Create_file.cpp -o huf1
```

### 2. Compress & Decompress
The program automatically checks the first 4 bytes for the `HUF1` header:
* If you pass a normal file, it **compresses** it.
* If you pass a `.huf` file, it **decompresses** it.

```bash
# Compress sample.txt -> archive.huf
./huf1 sample.txt archive.huf

# Decompress archive.huf -> restored.txt
./huf1 archive.huf restored.txt

# Check that both files are 100% identical
sha256sum sample.txt restored.txt
```