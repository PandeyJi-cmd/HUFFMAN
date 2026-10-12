#include "Huffman.cpp"
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <string>
constexpr char MAGIC[4]={'H','U','F','1'};
constexpr size_t CHUNK_SIZE = 65536; // this is the data of the huffman tree.
uint8_t bitbuffer=0;
int bitcount=0;

void write_bit(std::ofstream&out ,uint8_t bit){
    bitbuffer = (bitbuffer << 1 ) | (bit&1);
    if(++bitcount == 8){
        out.put(static_cast<char>(bitbuffer));
        bitbuffer =0;
        bitcount=0;
    }
}
void write(std::ofstream &os,uint64_t code){
    int len = 63 - __builtin_clzll(code);
    for(int i=len-1;i>=0;i--){
        write_bit(os,(code>>i)&1);
    }
}
void flush(std::ofstream&os) {
        if (bitcount > 0) {
            bitbuffer <<= (8 - bitcount);
            os.put(static_cast<char>(bitbuffer));
            bitbuffer = 0;
            bitcount = 0;
        }
    }
void compress_stream(std::ifstream &in ,const std::string & out_path){
    uint64_t freq_buff[256]={0};
    std::vector<char> chunk(CHUNK_SIZE);
    uint64_t total_bytes = 0;
    in.clear(); // clear if the pointer is not at the begnining.
    in.seekg(0,std::ios::beg); //reset to begining.
    uint16_t unique_chars=0;


    while(in.read(chunk.data(),CHUNK_SIZE) || in.gcount()>0){
        std::streamsize n =in.gcount();
        total_bytes+=n;
        for(std::streamsize i =0; i<n; ++i){
            freq_buff[static_cast<uint8_t>(chunk[i])]++;
            if(freq_buff[static_cast<uint8_t>(chunk[i])] == 1) unique_chars++;
        }
    }



    std::array<uint64_t,256> codes = create_encoding(freq_buff);

    std::ofstream out(out_path,std::ios::binary);
    out.write(MAGIC,4);
    out.write(reinterpret_cast<const char*>(&total_bytes), sizeof(total_bytes));
    out.write(reinterpret_cast<const char*>(&unique_chars), sizeof(unique_chars));
    for(int i=0;i<256;i++){
        if(codes[i]){
            uint8_t ch =static_cast<uint8_t>(i);
            out.write(reinterpret_cast<const char*>(&ch),sizeof(ch));
            out.write(reinterpret_cast<const char*>(&codes[i]),sizeof(codes[i]));

        }
    }

    in.clear();
    in.seekg(0,std::ios::beg);
    while(in.read(chunk.data(),CHUNK_SIZE) || in.gcount()> 0){
        std::streamsize n =in.gcount();
        for(std::streamsize i=0;i<n;++i){
            write(out,codes[static_cast<uint8_t>(chunk[i])]);
        }
    }
    flush(out);
    std::cout<<"[+] Compressed Successfully -> "<<out_path<<"\n";
}

void decompress_stream(std::ifstream &in,const std::string& out_path){
    uint64_t total_bytes =0;
    uint16_t unique_chars = 0;
    in.read(reinterpret_cast<char*>(&total_bytes), sizeof(total_bytes));
    in.read(reinterpret_cast<char*>(&unique_chars), sizeof(unique_chars));

    // read code and char pairs and sort them
    std::vector<std::pair<uint64_t, uint8_t>> table;
    table.reserve(unique_chars);

    for (uint16_t i = 0; i < unique_chars; ++i) {
        uint8_t ch;
        uint64_t code;
        in.read(reinterpret_cast<char*>(&ch), sizeof(ch));
        in.read(reinterpret_cast<char*>(&code), sizeof(code));
        table.push_back({code, ch});
    }

    // sort by code
    std::sort(table.begin(), table.end());

    std::ofstream out(out_path, std::ios::binary);
    std::vector<char> chunk(CHUNK_SIZE);
    uint64_t decoded = 0;
    uint64_t cur = 1; // separator

    while (decoded < total_bytes && (in.read(chunk.data(), CHUNK_SIZE) || in.gcount() > 0)) {
        std::streamsize n = in.gcount();
        for (std::streamsize i = 0; i < n && decoded < total_bytes; ++i) {
            uint8_t byte = static_cast<uint8_t>(chunk[i]);
            for (int bit_pos = 7; bit_pos >= 0 && decoded < total_bytes; --bit_pos) {
                cur = (cur << 1) | ((byte >> bit_pos) & 1);

                auto it = std::lower_bound(
                    table.begin(), table.end(),
                    std::make_pair(cur, static_cast<uint8_t>(0))
                );

                if (it != table.end() && it->first == cur) {
                    out.put(static_cast<char>(it->second));
                    decoded++;
                    cur = 1; 
                }
            }
        }
    }
}




int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
        std::cerr << "[-] Error: Could not open " << argv[1] << "\n";
        return 1;
    }

    char magic_check[4] = {0};
    in.read(magic_check, 4);

    if (in.gcount() == 4 && std::memcmp(magic_check, MAGIC, 4) == 0) {
        std::cout << "[*] HUF1 signature detected. Decompressing...\n";
        decompress_stream(in, argv[2]);
    } else {
        std::cout << "[*] Raw file detected. Compressing...\n";
        compress_stream(in, argv[2]);
    }

    return 0;
}