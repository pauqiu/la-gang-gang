#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace crypto {

// Clave de cifrado (debe ser igual en todos los nodos)
constexpr uint8_t KEY[] = {0x4C, 0x41, 0x47, 0x41, 0x4E, 0x47}; // "LAGANG"
constexpr size_t KEY_LEN = sizeof(KEY);

// Cifra/descifra datos in-place usando XOR
inline void xor_transform(void* data, size_t size) {
    uint8_t* bytes = static_cast<uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        bytes[i] ^= KEY[i % KEY_LEN];
    }
}

// Cifra datos en un nuevo buffer
inline void encrypt(const void* src, void* dst, size_t size) {
    std::memcpy(dst, src, size);
    xor_transform(dst, size);
}

// Descifra datos en un nuevo buffer
inline void decrypt(const void* src, void* dst, size_t size) {
    std::memcpy(dst, src, size);
    xor_transform(dst, size);
}

}
