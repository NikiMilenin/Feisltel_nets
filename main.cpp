#include <iostream>
#include <cstdint>
#include <random>
#include <fstream>
using namespace std;

#define NUM_OF_ROUNDS 8

uint64_t keygen();
uint64_t left_cycle_rotate_64(uint64_t, int);
uint64_t right_cycle_rotate_64(uint64_t, int);
uint32_t left_cycle_rotate_32(uint32_t, int);
uint32_t right_cycle_rotate_32(uint32_t, int);
void block_encrypt(uint64_t*, uint64_t);
void block_decrypt(uint64_t*, uint64_t);

int main() {
    ifstream input("input.txt", ios::binary);
    ofstream output("encrypted.txt", ios::binary);

    uint64_t key = keygen();
    char buffer[8];
    int added_bytes = 0;
    int len = 0;
    while (true) {
        input.read(buffer, 8);
        streamsize bytesRead = input.gcount();
        block_encrypt(reinterpret_cast<uint64_t*>(buffer), key);
        output.write(reinterpret_cast<char*>(buffer), sizeof(buffer));
        len += 8;
        if (bytesRead < 8) {
            added_bytes = 8 - bytesRead;
            break;
        }
    }
    input.close();
    output.close();

    input.open("encrypted.txt", ios::binary);
    output.open("decrypted.txt", ios::binary);
    int i = 0;
    while (true) {
        input.read(buffer, 8);
        streamsize bytesRead = input.gcount();
        i += 8;
        block_decrypt(reinterpret_cast<uint64_t*>(buffer), key);
        if (i >= len) {
            output.write(reinterpret_cast<char*>(buffer), sizeof(buffer) - added_bytes);
            break;
        }
        output.write(reinterpret_cast<char*>(buffer), sizeof(buffer));
    }

    input.close();
    output.close();
    return 0;
}

uint64_t keygen()
{
    random_device rand;
    mt19937 gen(rand());
    uniform_int_distribution<uint64_t> distribution(0, numeric_limits<uint64_t>::max());

    return distribution(gen);
}

uint32_t F(uint32_t key, uint32_t left)
{
    return left_cycle_rotate_32(left, 9) ^ (~(right_cycle_rotate_32(key, 11) ^ left));
}

uint64_t left_cycle_rotate_64(uint64_t num, int shift)
{
    return (num << shift) | (num >> (64 - shift));
}

uint64_t right_cycle_rotate_64(uint64_t num, int shift)
{
    return (num >> shift) | (num << (64 - shift));
}

uint32_t left_cycle_rotate_32(uint32_t num, int shift)
{
    return (num << shift) | (num >> (32 - shift));
}

uint32_t right_cycle_rotate_32(uint32_t num, int shift)
{
    return (num >> shift) | (num << (32 - shift));
}

void block_encrypt(uint64_t* block, uint64_t key)
{
    auto left = (uint32_t*) block;
    auto right = left + 1;

    for(int i=0; i<NUM_OF_ROUNDS; ++i)
    {
        uint64_t round_key = right_cycle_rotate_64(key, i*3);
        uint32_t temp = *right ^ F((uint32_t)round_key, *left);
        if (i < NUM_OF_ROUNDS - 1)
        {
            *right = *left;
            *left = temp;
        }
        else
        {
            *right = temp;
        }
    }
}

void block_decrypt(uint64_t* block, uint64_t key)
{
    auto left = (uint32_t*) block;
    auto right = left + 1;

    for(int i=NUM_OF_ROUNDS - 1; i >= 0; --i)
    {
        uint64_t round_key = right_cycle_rotate_64(key, i*3);
        uint32_t temp = *right ^ F((uint32_t)round_key, *left);
        if (i > 0)
        {
            *right = *left;
            *left = temp;
        }
        else
        {
            *right = temp;
        }
    }
}