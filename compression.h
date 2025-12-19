#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_TREE_HT 256

typedef struct MinHeapNode {
    char data;
    unsigned freq;
    struct MinHeapNode *left, *right;
} MinHeapNode;

typedef struct MinHeap {
    unsigned size;
    unsigned capacity;
    MinHeapNode** array;
} MinHeap;

typedef struct {
    char data;
    char code[MAX_TREE_HT];
} HuffmanCode;

// Function declarations
MinHeapNode* createNode(char data, unsigned freq);
MinHeap* createMinHeap(unsigned capacity);
void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b);
void minHeapify(MinHeap* minHeap, int idx);
int isSizeOne(MinHeap* minHeap);
MinHeapNode* extractMin(MinHeap* minHeap);
void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode);
void buildMinHeap(MinHeap* minHeap);
int isLeaf(MinHeapNode* root);
MinHeap* createAndBuildMinHeap(char data[], int freq[], int size);
MinHeapNode* buildHuffmanTree(char data[], int freq[], int size);
void storeCodes(MinHeapNode* root, char arr[], int top, HuffmanCode codes[], int* codeIndex);
void getCodes(MinHeapNode* root, HuffmanCode codes[], int* size);
char* compressHuffman(char* input, HuffmanCode codes[], int codeSize, long* compressedBits);
char* decompressHuffman(char* compressed, MinHeapNode* root, long originalLength);
void calculateFrequency(char* input, char data[], int freq[], int* size);
