#include "encryption.h"
 
uint8_t gmul2(uint8_t a) {
    return (a << 1) ^ (((a >> 7) & 1) * 0x1b);
}

uint8_t gmul3(uint8_t a) {
    return gmul2(a) ^ a;
}

uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        uint8_t hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

void subBytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = sbox[state[i]];
    }
}

void invSubBytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = inv_sbox[state[i]];
    }
}

void shiftRows(uint8_t* state) {
    uint8_t temp;
    temp = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = temp;
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    temp = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = state[3]; state[3] = temp;
}

void invShiftRows(uint8_t* state) {
    uint8_t temp;
    temp = state[13]; state[13] = state[9]; state[9] = state[5]; state[5] = state[1]; state[1] = temp;
    temp = state[2]; state[2] = state[10]; state[10] = temp;
    temp = state[6]; state[6] = state[14]; state[14] = temp;
    temp = state[3]; state[3] = state[7]; state[7] = state[11]; state[11] = state[15]; state[15] = temp;
}

void mixColumns(uint8_t* state) {
    uint8_t temp[16];
    for (int i = 0; i < 4; i++) {
        int col = i * 4;
        temp[col] = gmul2(state[col]) ^ gmul3(state[col+1]) ^ state[col+2] ^ state[col+3];
        temp[col+1] = state[col] ^ gmul2(state[col+1]) ^ gmul3(state[col+2]) ^ state[col+3];
        temp[col+2] = state[col] ^ state[col+1] ^ gmul2(state[col+2]) ^ gmul3(state[col+3]);
        temp[col+3] = gmul3(state[col]) ^ state[col+1] ^ state[col+2] ^ gmul2(state[col+3]);
    }
    memcpy(state, temp, 16);
}

void invMixColumns(uint8_t* state) {
    uint8_t temp[16];
    for (int i = 0; i < 4; i++) {
        int col = i * 4;
        temp[col] = gmul(state[col], 0x0e) ^ gmul(state[col+1], 0x0b) ^ gmul(state[col+2], 0x0d) ^ gmul(state[col+3], 0x09);
        temp[col+1] = gmul(state[col], 0x09) ^ gmul(state[col+1], 0x0e) ^ gmul(state[col+2], 0x0b) ^ gmul(state[col+3], 0x0d);
        temp[col+2] = gmul(state[col], 0x0d) ^ gmul(state[col+1], 0x09) ^ gmul(state[col+2], 0x0e) ^ gmul(state[col+3], 0x0b);
        temp[col+3] = gmul(state[col], 0x0b) ^ gmul(state[col+1], 0x0d) ^ gmul(state[col+2], 0x09) ^ gmul(state[col+3], 0x0e);
    }
    memcpy(state, temp, 16);
}

void addRoundKey(uint8_t* state, uint8_t* roundKey) {
    for (int i = 0; i < 16; i++) {
        state[i] ^= roundKey[i];
    }
}

void keyExpansion(uint8_t* key, uint8_t* expandedKey) {
    memcpy(expandedKey, key, 16);
    int bytesGenerated = 16;
    int rconIteration = 1;
    uint8_t temp[4];
    while (bytesGenerated < 176) {
        for (int i = 0; i < 4; i++) {
            temp[i] = expandedKey[bytesGenerated - 4 + i];
        }
        if (bytesGenerated % 16 == 0) {
            uint8_t k = temp[0];
            temp[0] = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = k;
            temp[0] = sbox[temp[0]]; temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]]; temp[3] = sbox[temp[3]];
            temp[0] ^= rcon[rconIteration++];
        }
        for (int i = 0; i < 4; i++) {
            expandedKey[bytesGenerated] = expandedKey[bytesGenerated - 16] ^ temp[i];
            bytesGenerated++;
        }
    }
}

void aes_encrypt_block(uint8_t* input, uint8_t* output, uint8_t* expandedKey) {
    uint8_t state[16];
    memcpy(state, input, 16);
    addRoundKey(state, expandedKey);
    for (int round = 1; round < 10; round++) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, expandedKey + (round * 16));
    }
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, expandedKey + 160);
    memcpy(output, state, 16);
}

void aes_decrypt_block(uint8_t* input, uint8_t* output, uint8_t* expandedKey) {
    uint8_t state[16];
    memcpy(state, input, 16);
    addRoundKey(state, expandedKey + 160);
    for (int round = 9; round > 0; round--) {
        invShiftRows(state);
        invSubBytes(state);
        addRoundKey(state, expandedKey + (round * 16));
        invMixColumns(state);
    }
    invShiftRows(state);
    invSubBytes(state);
    addRoundKey(state, expandedKey);
    memcpy(output, state, 16);
}

int addPadding(uint8_t* data, int dataLen, uint8_t** paddedData) {
    int paddingLen = AES_BLOCK_SIZE - (dataLen % AES_BLOCK_SIZE);
    int totalLen = dataLen + paddingLen;
    *paddedData = (uint8_t*)malloc(totalLen);
    memcpy(*paddedData, data, dataLen);
    for (int i = dataLen; i < totalLen; i++) {
        (*paddedData)[i] = (uint8_t)paddingLen;
    }
    return totalLen;
}

int removePadding(uint8_t* data, int dataLen) {
    uint8_t paddingLen = data[dataLen - 1];
    if (paddingLen > AES_BLOCK_SIZE || paddingLen == 0) {
        return dataLen;
    }
    for (int i = dataLen - paddingLen; i < dataLen; i++) {
        if (data[i] != paddingLen) {
            return dataLen;
        }
    }
    return dataLen - paddingLen;
}

void passwordToKey(char* password, uint8_t* key) {
    int len = strlen(password);
    memset(key, 0, AES_KEY_SIZE);
    for (int i = 0; i < len; i++) {
        key[i % AES_KEY_SIZE] ^= (uint8_t)password[i];
    }
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        key[i] = (key[i] * 131 + 17) & 0xFF;
    }
}

uint8_t* aes_encrypt(uint8_t* plaintext, int plaintextLen, uint8_t* key, int* encryptedLen) {
    uint8_t* paddedData;
    int paddedLen = addPadding(plaintext, plaintextLen, &paddedData);
    uint8_t* encrypted = (uint8_t*)malloc(paddedLen);
    uint8_t expandedKey[176];
    keyExpansion(key, expandedKey);
    for (int i = 0; i < paddedLen; i += AES_BLOCK_SIZE) {
        aes_encrypt_block(paddedData + i, encrypted + i, expandedKey);
    }
    free(paddedData);
    *encryptedLen = paddedLen;
    return encrypted;
}

uint8_t* aes_decrypt(uint8_t* ciphertext, int ciphertextLen, uint8_t* key, int* decryptedLen) {
    uint8_t* decrypted = (uint8_t*)malloc(ciphertextLen);
    uint8_t expandedKey[176];
    keyExpansion(key, expandedKey);
    for (int i = 0; i < ciphertextLen; i += AES_BLOCK_SIZE) {
        aes_decrypt_block(ciphertext + i, decrypted + i, expandedKey);
    }
    *decryptedLen = removePadding(decrypted, ciphertextLen);
    return decrypted;
}

uint8_t* binaryStringToBytes(char* binaryStr, int* byteLen) {
    int bitLen = strlen(binaryStr);
    *byteLen = (bitLen + 7) / 8;
    uint8_t* bytes = (uint8_t*)calloc(*byteLen, sizeof(uint8_t));
    for (int i = 0; i < bitLen; i++) {
        if (binaryStr[i] == '1') {
            bytes[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    return bytes;
}

char* bytesToBinaryString(uint8_t* bytes, int byteLen, int originalBitLen) {
    char* binaryStr = (char*)malloc(originalBitLen + 1);
    for (int i = 0; i < originalBitLen; i++) {
        binaryStr[i] = (bytes[i / 8] & (1 << (7 - (i % 8)))) ? '1' : '0';
    }
    binaryStr[originalBitLen] = '\0';
    return binaryStr;
}

void printHex(uint8_t* data, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (len % 16 != 0) printf("\n");
}

