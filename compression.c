#include "compression.h"

MinHeapNode* createNode(char data, unsigned freq) {
    MinHeapNode* temp = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;
    return temp;
}
 
MinHeap* createMinHeap(unsigned capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**)malloc(minHeap->capacity * sizeof(MinHeapNode*));
    return minHeap;
}

void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;
    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;
    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(MinHeap* minHeap) {
    return (minHeap->size == 1);
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

void insertMinHeap(MinHeap* minHeap, MinHeapNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;
    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    minHeap->array[i] = minHeapNode;
}

void buildMinHeap(MinHeap* minHeap) {
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int isLeaf(MinHeapNode* root) {
    return !(root->left) && !(root->right);
}

MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) {
    MinHeap* minHeap = createMinHeap(size);
    for (int i = 0; i < size; ++i)
        minHeap->array[i] = createNode(data[i], freq[i]);
    minHeap->size = size;
    buildMinHeap(minHeap);
    return minHeap;
}

MinHeapNode* buildHuffmanTree(char data[], int freq[], int size) {
    MinHeapNode *left, *right, *top;
    MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);
    while (!isSizeOne(minHeap)) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);
        top = createNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(minHeap, top);
    }
    return extractMin(minHeap);
}

void storeCodes(MinHeapNode* root, char arr[], int top, HuffmanCode codes[], int* codeIndex) {
    if (root->left) {
        arr[top] = '0';
        storeCodes(root->left, arr, top + 1, codes, codeIndex);
    }
    if (root->right) {
        arr[top] = '1';
        storeCodes(root->right, arr, top + 1, codes, codeIndex);
    }
    if (isLeaf(root)) {
        codes[*codeIndex].data = root->data;
        arr[top] = '\0';
        strcpy(codes[*codeIndex].code, arr);
        (*codeIndex)++;
    }
}

void getCodes(MinHeapNode* root, HuffmanCode codes[], int* size) {
    char arr[MAX_TREE_HT];
    int top = 0;
    *size = 0;
    storeCodes(root, arr, top, codes, size);
}

char* compressHuffman(char* input, HuffmanCode codes[], int codeSize, long* compressedBits) {
    long inputLen = strlen(input);
    long maxBits = inputLen * MAX_TREE_HT;
    char* compressed = (char*)malloc(maxBits + 1);
    compressed[0] = '\0';
    for (long i = 0; i < inputLen; i++) {
        for (int j = 0; j < codeSize; j++) {
            if (input[i] == codes[j].data) {
                strcat(compressed, codes[j].code);
                break;
            }
        }
    }
    *compressedBits = strlen(compressed);
    return compressed;
}

char* decompressHuffman(char* compressed, MinHeapNode* root, long originalLength) {
    char* decompressed = (char*)malloc(originalLength + 1);
    int index = 0;
    MinHeapNode* current = root;
    for (long i = 0; compressed[i] != '\0'; i++) {
        if (compressed[i] == '0')
            current = current->left;
        else
            current = current->right;
        if (isLeaf(current)) {
            decompressed[index++] = current->data;
            current = root;
        }
    }
    decompressed[index] = '\0';
    return decompressed;
}

void calculateFrequency(char* input, char data[], int freq[], int* size) {
    int count[256] = {0};
    for (long i = 0; input[i] != '\0'; i++) {
        count[(unsigned char)input[i]]++;
    }
    *size = 0;
    for (int i = 0; i < 256; i++) {
        if (count[i] > 0) {
            data[*size] = (char)i;
            freq[*size] = count[i];
            (*size)++;
        }
    }
}

