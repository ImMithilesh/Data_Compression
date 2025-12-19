#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compression.h"
#include "encryption.h"

int main() {
    char* input = (char*)malloc(10000 * sizeof(char));
    char password[256];
 
    printf("========================================\n");
    printf("INTEGRATED COMPRESSION + ENCRYPTION\n");
    printf("Huffman Coding + AES-128\n");
    printf("========================================\n\n");

    printf("Enter the data you want to secure:\n");
    printf("(Press Enter twice to finish)\n");
    printf("----------------------------------------\n");

    // Read input
    char line[1000];
    input[0] = '\0';
    while (fgets(line, sizeof(line), stdin)) {
        if (line[0] == '\n') break;
        strcat(input, line);
    }

    int inputLen = strlen(input);
    if (inputLen > 0 && input[inputLen-1] == '\n') {
        input[inputLen-1] = '\0';
        inputLen--;
    }

    if (inputLen == 0) {
        printf("Error: No input provided!\n");
        free(input);
        return 1;
    }

    printf("----------------------------------------\n");
    printf("\nEnter encryption password:\n");
    printf("> ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    if (strlen(password) == 0) {
        printf("Error: No password provided!\n");
        free(input);
        return 1;
    }

    // ============================================
    // STEP 1: COMPRESSION
    // ============================================
    printf("\n========================================\n");
    printf("  STEP 1: HUFFMAN COMPRESSION\n");
    printf("========================================\n");
    printf("Original Data: %s\n", input);
    printf("Original Size: %d bytes\n\n", inputLen);

    char data[256];
    int freq[256];
    int freqSize;
    calculateFrequency(input, data, freq, &freqSize);

    MinHeapNode* huffmanRoot = buildHuffmanTree(data, freq, freqSize);

    HuffmanCode codes[256];
    int codeSize;
    getCodes(huffmanRoot, codes, &codeSize);

    long compressedBitCount;
    char* compressedBinary = compressHuffman(input, codes, codeSize, &compressedBitCount);

    long compressedSizeInBytes = (compressedBitCount + 7) / 8;
    double compressionRatio = (double)inputLen / compressedSizeInBytes;
    double compressionPercent = ((double)(inputLen - compressedSizeInBytes) / inputLen) * 100;

    printf("Huffman tree built\n");
    printf("Data compressed\n");
    printf("Compressed Size: %ld bytes (%ld bits)\n", compressedSizeInBytes, compressedBitCount);
    printf("Compression Ratio: %.2f:1\n", compressionRatio);
    printf("Space Saved: %.2f%%\n", compressionPercent);

    // ============================================
    // STEP 2: ENCRYPTION
    // ============================================
    printf("\n========================================\n");
    printf("  STEP 2: AES-128 ENCRYPTION\n");
    printf("========================================\n");

    uint8_t aesKey[AES_KEY_SIZE];
    passwordToKey(password, aesKey);

    printf("Encryption key generated from password\n");

    // Convert compressed binary string to bytes
    int compressedByteArrayLen;
    uint8_t* compressedByteArray = binaryStringToBytes(compressedBinary, &compressedByteArrayLen);

    // Encrypt the compressed data
    int encryptedDataLen;
    uint8_t* encryptedData = aes_encrypt(compressedByteArray, compressedByteArrayLen, aesKey, &encryptedDataLen);

    printf("Compressed data encrypted with AES-128\n");
    printf("Encrypted Size: %d bytes\n", encryptedDataLen);

    printf("\nEncrypted Data (Hex - first 64 bytes):\n");
    printf("----------------------------------------\n");
    int hexDisplayLen = encryptedDataLen < 64 ? encryptedDataLen : 64;
    printHex(encryptedData, hexDisplayLen);
    if (encryptedDataLen > 64) {
        printf("... (%d more bytes)\n", encryptedDataLen - 64);
    }

    // ============================================
    // SUMMARY
    // ============================================
    printf("\n========================================\n");
    printf("       SECURE COMPRESSION SUMMARY\n");
    printf("========================================\n");
    printf("Original Size:        %d bytes\n", inputLen);
    printf("After Compression:    %ld bytes (%.2f%% saved)\n", compressedSizeInBytes, compressionPercent);
    printf("After Encryption:     %d bytes\n", encryptedDataLen);
    printf("Final Overhead:       %d bytes (AES padding)\n", encryptedDataLen - compressedByteArrayLen);
    printf("Data Status:          ✓ Compressed & Encrypted\n");
    printf("========================================\n");

    // ============================================
    // STEP 3: DECRYPTION
    // ============================================
    printf("\n========================================\n");
    printf("  STEP 3: AES-128 DECRYPTION\n");
    printf("========================================\n");
    printf("Decrypting data with password...\n");

    int decryptedDataLen;
    uint8_t* decryptedData = aes_decrypt(encryptedData, encryptedDataLen, aesKey, &decryptedDataLen);

    printf("Data decrypted successfully\n");
    printf("Recovered %d bytes\n", decryptedDataLen);

    // ============================================
    // STEP 4: DECOMPRESSION
    // ============================================
    printf("\n========================================\n");
    printf("  STEP 4: HUFFMAN DECOMPRESSION\n");
    printf("========================================\n");
    printf("Decompressing data...\n");

    // Convert decrypted bytes back to binary string
    char* decryptedBinaryStr = bytesToBinaryString(decryptedData, decryptedDataLen, compressedBitCount);

    // Decompress using Huffman tree
    char* finalDecompressed = decompressHuffman(decryptedBinaryStr, huffmanRoot, inputLen);

    printf("Data decompressed successfully\n");
    printf("Recovered %d bytes\n", (int)strlen(finalDecompressed));

    // ============================================
    // FINAL VERIFICATION
    // ============================================
    printf("\n========================================\n");
    printf("      FINAL VERIFICATION\n");
    printf("========================================\n");

    if (strcmp(input, finalDecompressed) == 0) {
        printf("SUCCESS: Complete cycle verified!\n");
        printf("Original->Compress->Encrypt->Decrypt->Decompress\n");
        printf("Original and final data are IDENTICAL\n");
        printf("No data loss occurred\n");
        printf("Secure compression working perfectly!\n");
    } else {
        printf("ERROR: Data mismatch!\n");
    }

    printf("\nFinal Decompressed Data:\n");
    printf("----------------------------------------\n");
    printf("%s\n", finalDecompressed);
    printf("----------------------------------------\n");
    printf("\nSecure Data Pipeline Complete!\n");
    printf("========================================\n");

    // Cleanup
    free(input);
    free(compressedBinary);
    free(compressedByteArray);
    free(encryptedData);
    free(decryptedData);
    free(decryptedBinaryStr);
    free(finalDecompressed);

    return 0;
}

