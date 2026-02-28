#include "chacha20.hpp"
#include <cstring>
#include <random>
#include <stdexcept>
#include <iostream>

static const uint32_t sigma[4] = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};

ChaCha20::ChaCha20(const std::string& key, const std::string& nonce, uint32_t counter) {
    if (key.size() != 32) {
        throw std::invalid_argument("Key must be 32 bytes");
    }
    if (nonce.size() != 12) {
        throw std::invalid_argument("Nonce must be 12 bytes");
    }

    // Set constants "expand 32-byte k"
    state[0] = sigma[0];
    state[1] = sigma[1];
    state[2] = sigma[2];
    state[3] = sigma[3];

    // Set key
    const uint32_t* k = reinterpret_cast<const uint32_t*>(key.data());
    for (int i = 0; i < 8; ++i) {
        state[4 + i] = k[i];
    }

    // Set counter
    state[12] = counter;

    // Set nonce
    const uint32_t* n = reinterpret_cast<const uint32_t*>(nonce.data());
    for (int i = 0; i < 3; ++i) {
        state[13 + i] = n[i];
    }
}

uint32_t ChaCha20::rotl(uint32_t a, uint32_t b) {
    return (a << b) | (a >> (32 - b));
}

void ChaCha20::quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b; d ^= a; d = rotl(d, 16);
    c += d; b ^= c; b = rotl(b, 12);
    a += b; d ^= a; d = rotl(d, 8);
    c += d; b ^= c; b = rotl(b, 7);
}

void ChaCha20::generate_keystream() {
    uint32_t working_state[16];
    std::memcpy(working_state, state, sizeof(state));

    for (int i = 0; i < 10; ++i) {
        quarter_round(working_state[0], working_state[4], working_state[8], working_state[12]);
        quarter_round(working_state[1], working_state[5], working_state[9], working_state[13]);
        quarter_round(working_state[2], working_state[6], working_state[10], working_state[14]);
        quarter_round(working_state[3], working_state[7], working_state[11], working_state[15]);
        quarter_round(working_state[0], working_state[5], working_state[10], working_state[15]);
        quarter_round(working_state[1], working_state[6], working_state[11], working_state[12]);
        quarter_round(working_state[2], working_state[7], working_state[8], working_state[13]);
        quarter_round(working_state[3], working_state[4], working_state[9], working_state[14]);
    }

    for (int i = 0; i < 16; ++i) {
        working_state[i] += state[i];
    }

    std::memcpy(keystream, working_state, sizeof(keystream));
    state[12]++; // Increment counter
    keystream_idx = 0;
}

void ChaCha20::process(uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (keystream_idx >= 64) {
            generate_keystream();
        }
        data[i] ^= keystream[keystream_idx++];
    }
}

void ChaCha20::process(std::vector<uint8_t>& data) {
    process(data.data(), data.size());
}

std::string ChaCha20::encrypt(const std::string& plaintext, const std::string& key) {
    // Generate random nonce
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;

    uint32_t nonce_u32[3];
    nonce_u32[0] = dis(gen);
    nonce_u32[1] = dis(gen);
    nonce_u32[2] = dis(gen);

    std::string nonce(reinterpret_cast<char*>(nonce_u32), 12);

    ChaCha20 cipher(key, nonce);
    
    std::string ciphertext = plaintext;
    cipher.process(reinterpret_cast<uint8_t*>(&ciphertext[0]), ciphertext.size());
    
    // Prepend nonce to ciphertext
    return nonce + ciphertext;
}

std::string ChaCha20::decrypt(const std::string& ciphertext, const std::string& key) {
    if (ciphertext.size() < 12) {
        return ""; // Or throw exception
    }
    
    std::string nonce = ciphertext.substr(0, 12);
    std::string actual_ciphertext = ciphertext.substr(12);
    
    ChaCha20 cipher(key, nonce);
    
    std::string plaintext = actual_ciphertext;
    cipher.process(reinterpret_cast<uint8_t*>(&plaintext[0]), plaintext.size());
    
    return plaintext;
}
