#pragma once
#include <vector>
#include <string>
#include <cstdint>

class ChaCha20 {
public:
    // Key must be 32 bytes (256 bits)
    // Nonce must be 12 bytes (96 bits)
    ChaCha20(const std::string& key, const std::string& nonce, uint32_t counter = 1);

    // Encrypt/Decrypt data (XOR with keystream)
    // Updates internal state, so subsequent calls continue the stream
    void process(std::vector<uint8_t>& data);
    void process(uint8_t* data, size_t len);
    
    // Static helper for one-shot encryption/decryption
    // Returns nonce + ciphertext
    static std::string encrypt(const std::string& plaintext, const std::string& key);
    
    // Expects nonce + ciphertext
    // Returns plaintext
    static std::string decrypt(const std::string& ciphertext, const std::string& key);

private:
    uint32_t state[16];
    uint8_t keystream[64];
    size_t keystream_idx = 64; // Force generation on first call

    void generate_keystream();
    static uint32_t rotl(uint32_t a, uint32_t b);
    static void quarter_round(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d);
};
