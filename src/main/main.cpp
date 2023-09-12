//
//	main/main.cpp
//

// Project
#include "main.h"
#include "data/util.h"
#include "data/resourceheader.h"

// C/C++
#include <stdlib.h>
#include <string.h>

#define GAME_DUMP_FILE			"eboot.bin"
#define DECRYPTION_KEYS_FILE	"decryption_keys.bin"

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

	// First try to load the decryption keys from decryption_keys.bin
	if (!LoadDecryptionKeys(DECRYPTION_KEYS_FILE))
	{
		// Then try to locate a game dump and grab it from there. If this fails we exit.
		if (!LoadDecryptionKeys(GAME_DUMP_FILE)) {
			fclose(pFile);
			pFile = nullptr;

			return 1;
		}

		// Save the decryption keys for the next time after grabbing it from the game dump. We dont exit on failure here.
		if (!SaveDecryptionKeys(DECRYPTION_KEYS_FILE)) {
			printf("Failed to save the decryption keys.");
		}
	}
	printf("Loaded all decryption keys.\n");

	size_t fileSize				= GetFileSize(fPath);				// The entire file size
	size_t encryptedSize		= (fileSize - sizeof(header));		// Subtract the datResourceFileHeader for the encrypted size
	size_t inflatedSize			= (header.m_Info.GetPhysicalSize() + header.m_Info.GetVirtualSize()); // Decompressed file size

	unsigned char* pContent		= system_new unsigned char[encryptedSize];	// For storing encrypted/decrypted content
	unsigned char* pInflated	= system_new unsigned char[inflatedSize];	// For storing inflated (decompressed) content

	fread_s(pContent, encryptedSize, encryptedSize, 1, pFile);

	fclose(pFile);
	pFile = nullptr;

	DecryptResource(GetKeyIndex(fName, fileSize), pContent, encryptedSize); // Decrypt our script with AES-256-ECB

	// Now inflate our decrypted buffer. On failure we need to cleanup our memory!
	if (!InflateResource(pContent, pInflated, encryptedSize, inflatedSize))
	{
		system_delete_array(pContent);
		system_delete_array(pInflated);

		return 1;
	}

	char path[_MAX_PATH];
	strncpy_s(path, fPath, strnlen_s(fPath, _MAX_PATH) - 4);
	strcat_s(path, "_unpacked");
	strcat_s(path, fExt);

	// Save unpacked script
	const int success = SaveUnpackedResource(path, pInflated, inflatedSize) == true ? 0 : 1;

	// Final cleanup
	system_delete_array(pContent);
	system_delete_array(pInflated);

	return success;
}