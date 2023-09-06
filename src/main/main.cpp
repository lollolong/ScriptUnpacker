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

	if (!LoadDecryptionKeys("decryption_keys.bin"))
	{
		if (!LoadDecryptionKeys("eboot.bin")) {
			return 1;
		}
		if (!SaveDecryptionKeys("decryption_keys.bin")) {
			printf("Failed to save the decryption keys.");
		}
	}

	size_t fileSize				= GetFileSize(fPath);
	size_t encryptedSize		= (fileSize - sizeof(header));
	size_t inflatedSize			= (header.m_Info.GetPhysicalSize() + header.m_Info.GetVirtualSize());

	unsigned char* pContent		= system_new unsigned char[encryptedSize];
	unsigned char* pInflated	= system_new unsigned char[inflatedSize];

	fread_s(pContent, encryptedSize, encryptedSize, 1, pFile);

	DecryptResource(GetKeyIndex(fName, fileSize), pContent, encryptedSize);

	if (!InflateResource(pContent, pInflated, encryptedSize, inflatedSize))
	{
		system_delete_array(pContent);
		system_delete_array(pInflated);

		fclose(pFile);
		pFile = nullptr;

		return 1;
	}

	char path[_MAX_PATH];
	strncpy_s(path, fPath, strnlen_s(fPath, _MAX_PATH) - 4);
	strcat_s(path, "_unpacked");
	strcat_s(path, fExt);

	SaveUnpackedResource(path, pInflated, inflatedSize);

	fclose(pFile);
	pFile = nullptr;

	system_delete_array(pContent);
	system_delete_array(pInflated);

	return 0;
}