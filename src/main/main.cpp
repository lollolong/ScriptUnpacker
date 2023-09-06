//
//	main/main.cpp
//

#include "main.h"

#include "data/resourceheader.h"
#include "data/stringhash.h"

using namespace rage;

const unsigned int OrbisCrc32[101]
{
	0x063d045a, 0x2c5d2338, 0x619adc5b, 0x4825ca62, 0xedbc90f1,
	0xa235ec58, 0x364500e3, 0x9eb0e06d, 0x7adb7629, 0x4ad8d570,
	0x555f0f58, 0xaf6b5ca4, 0x96ce8baf, 0x9658c0fe, 0x695f2f4f,
	0xb26cb558, 0x6774e7ca, 0x4a6d5b0e, 0x1815db20, 0xe207b352,
	0x34c961d6, 0xfd8af3c2, 0xa6c06fca, 0x50aeb8e1, 0xd4de5541,
	0x889c1aac, 0xc0335f83, 0x6cb29d9d, 0xcfa4cf7b, 0xe4b33626,
	0x6f769f4e, 0x7c62ed2c, 0xacb9a6f1, 0x93a28355, 0x931d5a21,
	0x05751b57, 0xf83f6ec7, 0xc6148599, 0xa1ba0945, 0xbaf85cff,
	0xdce0ba6f, 0xfd0bad67, 0xa356670f, 0xf6107d61, 0xa142b936,
	0x4164f2e2, 0x96facea6, 0x01468de3, 0xd23732b4, 0xa33ff140,
	0xe7c657bd, 0xcbc30d29, 0x34c44d73, 0x6ebe4edc, 0x2597cda1,
	0x40854111, 0x6af94254, 0x313e782f, 0xd475364b, 0x47da05f6,
	0xa69f44e5, 0x7b546771, 0xf3c7a700, 0x6d2351b8, 0xe78b15dd,
	0x10c9638b, 0x7b980f6c, 0xb823c10d, 0x018c1ba8, 0x4fcb6fdf,
	0x627675f0, 0x12caaecd, 0x168ccb58, 0x40cd8163, 0x62048133,
	0x85fa5c47, 0xecba0d01, 0xa5aac447, 0x4ca7feaf, 0x57378ddb,
	0x45205917, 0x78b1da4d, 0xdb94a5dd, 0xdac914e5, 0x86c0669d,
	0xe2c341db, 0xb58231ab, 0x4c946fe3, 0xd6437dde, 0xe021f78b,
	0x280fcc07, 0x4a20cca7, 0x242929ee, 0xc0d9c3cd, 0xbf7e3cc2,
	0xa8dbc46e, 0xb68d9385, 0x0b539ee3, 0xae0dbd1c, 0x57ee2ff7,
	0xf5508e8c
};



unsigned char g_DecryptionKeys[101][32] = {};

size_t GetFileSize(const char* path)
{
	struct stat st;
	return stat(path, &st) == 0 ? st.st_size : 0;
}

unsigned int GetKeyIndex(const char* filename, size_t size)
{
	return ((atStringHash(filename) + size) % 101);
}

void DecryptResourceFile(unsigned int keyIdx, unsigned char* pBuffer, size_t size)
{
	static const unsigned int AES_BLOCK_SIZE = 16;

	aes256_context_t ctx;
	aes256_init(&ctx, (aes256_key_t*)g_DecryptionKeys[keyIdx]);

	for (size_t off = 0; off < (size - (size % AES_BLOCK_SIZE)); off += AES_BLOCK_SIZE) {
		aes256_decrypt_ecb(&ctx, (aes256_blk_t*)(pBuffer + off));
	}

	aes256_done(&ctx);
}

bool InflateResourceFile(unsigned char* pInput, unsigned char* pOutput, size_t inputSize, size_t outputSize)
{
	z_stream zStream;
	memset(&zStream, 0, sizeof(zStream));

	if (inflateInit2(&zStream, -MAX_WBITS) < 0) {
		printf("Error in inflateInit2");
		return 1;
	}

	zStream.next_in		= pInput;
	zStream.avail_in	= (uInt)inputSize;
	zStream.next_out	= pOutput;
	zStream.avail_out	= (uInt)outputSize;

	int err;
	do {
		err = inflate(&zStream, Z_SYNC_FLUSH);
		if (err < 0) {
			printf("Error in inflate: %s\n", zStream.msg);
			inflateEnd(&zStream);
			return false;
		}
		if (err == Z_STREAM_END) {
			printf("Decompression successful!\n");
			break;
		}
	} while (zStream.avail_in > 0);

	inflateEnd(&zStream);
	return true;
}

int SaveContent(const char* filename, const unsigned char* pBuffer, size_t size)
{
	FILE* pFile;
	int err = fopen_s(&pFile, filename, "wb");

	if (err != 0) {
		printf("The output file could not be opened.\n");
		return err;
	}

	fwrite(pBuffer, size, 1, pFile);

	fclose(pFile);
	pFile = nullptr;

	return 0;
}

bool SaveKeys(const char* filename)
{
	FILE* pDecryptionKeys;
	const int err = fopen_s(&pDecryptionKeys, filename, "wb");

	if (err != 0) {
		printf("Unable to create %s\n", filename);
		return false;
	}

	for (int keyIdx = 0; keyIdx < 101; keyIdx++)
	{
		fwrite(g_DecryptionKeys[keyIdx], 32, 1, pDecryptionKeys);
	}

	fclose(pDecryptionKeys);
	pDecryptionKeys = nullptr;

	return true;
}

bool LoadDecryptionKeys(const char* filename)
{
	FILE* pDump;
	const int err = fopen_s(&pDump, filename, "rb");

	if (err != 0) 
	{
		if (!strcmp(filename, "eboot.bin")) {
			printf("Unable to open eboot.bin\n");
		}
		return false;
	}

	unsigned int keyCount = 0;
	unsigned char buffer[32];

	while (fread_s(buffer, sizeof(buffer), sizeof(buffer), 1, pDump))
	{
		for (int keyIdx = 0; keyIdx < 101; keyIdx++)
		{
			if (crc32(0, buffer, sizeof(buffer)) == OrbisCrc32[keyIdx])
			{
				//printf("Found Key %i (Crc: 0x%08x)\n", keyIdx + 1, OrbisCrc32[keyIdx]);
				memcpy(g_DecryptionKeys[keyIdx], buffer, sizeof(buffer));
				keyCount++;
				break;
			}
		}
	}

	fclose(pDump);
	pDump = nullptr;

	return keyCount == 101;
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("Usage: %s script.#sc\n", argv[0]);
		return 1;
	}

	const char* fPath = argv[1];
	const char* fName = strrchr(fPath, '\\') ? strrchr(fPath, '\\') + 1 : fPath;
	const char* fExt = strrchr(fName, '.');

	if (!fExt) {
		printf("No file extension.\n");
		return 1;
	}

	const bool allowedType =	/*!strcmp(fExt, ".xsc") ||*/	// Xenon
								/*!strcmp(fExt, ".csc") ||*/	// Cell
								/*!strcmp(fExt, ".ysc") ||*/	// x64
								!strcmp(fExt, ".osc") ||		// Orbis
								!strcmp(fExt, ".psc");			// Prospero

	if (!allowedType) {
		printf("Unknown input file format.\n");
		return 1;
	}

	FILE* pFile;
	const int err = fopen_s(&pFile, argv[1], "rb");

	if (err != 0) {
		printf("The specified file could not be opened.\n");
		return 1;
	}

	datResourceFileHeader header;
	fread_s(&header, sizeof(header), sizeof(header), 1, pFile);

	if (!header.IsValidResource()) {
		printf("File is not a valid resource.\n");

		fclose(pFile);
		pFile = nullptr;
		return 1;
	}

	if (!LoadDecryptionKeys("decryption_keys.bin"))
	{
		if (!LoadDecryptionKeys("eboot.bin")) {
			return 1;
		}
		if (!SaveKeys("decryption_keys.bin")) {
			printf("Failed to save the decryption keys.");
		}
	}

	size_t fileSize				= GetFileSize(fPath);
	size_t encryptedSize		= (fileSize - sizeof(header));
	size_t inflatedSize			= (header.m_Info.GetPhysicalSize() + header.m_Info.GetVirtualSize());

	unsigned char* pContent		= system_new unsigned char[encryptedSize];
	unsigned char* pInflated	= system_new unsigned char[inflatedSize];

	fread_s(pContent, encryptedSize, encryptedSize, 1, pFile);

	DecryptResourceFile(GetKeyIndex(fName, fileSize), pContent, encryptedSize);

	InflateResourceFile(pContent, pInflated, encryptedSize, inflatedSize);

	char path[_MAX_PATH];
	strncpy_s(path, fPath, strnlen_s(fPath, _MAX_PATH) - 4);
	strcat_s(path, "_unpacked");
	strcat_s(path, fExt);

	SaveContent(path, pInflated, inflatedSize);

	fclose(pFile);
	pFile = nullptr;

	system_delete_array(pContent);
	system_delete_array(pInflated);

	return 0;
}