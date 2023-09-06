//
//	data/util.h
//

#ifndef _UTIL_H_
#define _UTIL_H_

size_t GetFileSize(const char* path);
unsigned int GetKeyIndex(const char* filename, size_t size);

bool LoadDecryptionKeys(const char* filename);
bool SaveDecryptionKeys(const char* filename);

void DecryptResource(unsigned int keyIdx, unsigned char* pBuffer, size_t size);
bool InflateResource(unsigned char* pInput, unsigned char* pOutput, size_t inputSize, size_t outputSize);
bool SaveUnpackedResource(const char* filename, const unsigned char* pBuffer, size_t size);

#endif // _UTIL_H_