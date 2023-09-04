//
//	main/main.cpp
//

#include "main.h"

#include "data/resourceheader.h"
#include "data/stringhash.h"

using namespace rage;


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

	const bool allowedType =	!strcmp(fExt, ".xsc") ||	// Xenon
								!strcmp(fExt, ".csc") ||	// Cell
								!strcmp(fExt, ".ysc") ||	// x64
								!strcmp(fExt, ".osc") ||	// Orbis
								!strcmp(fExt, ".psc");		// Prospero

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

	fseek(pFile, 0, SEEK_END);
	long fSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	datResourceFileHeader header;
	fread_s(&header, sizeof(header), sizeof(header), 1, pFile);

	if (!header.IsValidResource()) {
		printf("File is not a valid resource.\n");

		fclose(pFile);
		pFile = nullptr;
		return 1;
	}

	size_t bufSize = (fSize - sizeof(header));
	char* pBuffer = system_new char[bufSize];
	fread_s(pBuffer, bufSize, bufSize, 1, pFile);

	//memset(pBufferDecrypted, 0, bufSize);

	fclose(pFile);
	pFile = nullptr;



	unsigned keyIndex;
	keyIndex = atStringHash(fName);
	keyIndex += fSize;
	keyIndex %= 101;

	aes256_context_t ctx;

	aes256_init(&ctx, (aes256_key_t*)KEYS[keyIndex]);

	for (size_t i = 0; i < (bufSize - (bufSize % 16)); i += 16)
	{
		aes256_decrypt_ecb(&ctx, (aes256_blk_t*)(pBuffer + i));
	}
	aes256_done(&ctx);

	size_t decompressedSize = header.m_Info.GetPhysicalSize(8192) + header.m_Info.GetVirtualSize(8192);
	char* pDecompressed = system_new char[decompressedSize];

	z_stream zStream;
	memset(&zStream, 0, sizeof(zStream));

	if (inflateInit2(&zStream, -MAX_WBITS) < 0) {
		printf("Error in inflateInit2");

		system_delete_array(pDecompressed);
		return 1;
	}
	zStream.next_in = (unsigned char*)pBuffer;
	zStream.avail_in = (uInt)bufSize;
	zStream.next_out = (unsigned char*)pDecompressed;
	zStream.avail_out = (uInt)decompressedSize;

	int zerr;
	do {
		zerr = inflate(&zStream, Z_SYNC_FLUSH);
		if (zerr < 0) {
			printf("Error in inflate: %s\n", zStream.msg);
			inflateEnd(&zStream);
			return 1;
		}

		if (zerr == Z_STREAM_END) {
			printf("Decompression successful!\n");
			break;
		}
	} while (zStream.avail_in > 0);
	inflateEnd(&zStream);


	system_delete_array(pBuffer);
	system_delete_array(pDecompressed);

	printf("GetPhysicalSize() = %u\n", header.m_Info.GetPhysicalSize(8192));
	printf("GetVirtualSize() = %u\n", header.m_Info.GetVirtualSize(8192));

	return 0;
}