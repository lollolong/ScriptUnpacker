//
//	data/resourceheader.h
//

#ifndef _RESOURCEHEADER_H
#define _RESOURCEHEADER_H

namespace rage
{
	struct datResourceInfo
	{
		struct Flags
		{
			// https://github.com/Neodymium146/gta-toolkit/blob/master/RageLib.GTA5/Resources/PC/ResourceFile_GTA5_pc.cs#L79-L99

			unsigned LeafShift : 4;		// (flags >> 0) & 0b1111
			unsigned Mul16 : 1;			// (flags >> 4) & 0b1
			unsigned Mul8 : 2;			// (flags >> 5) & 0b11
			unsigned Mul4 : 4;			// (flags >> 7) & 0b1111
			unsigned Mul2 : 6;			// (flags >> 11) & 0b111111
			unsigned Mul1 : 7;			// (flags >> 17) & 0b1111111
			unsigned Div2 : 1;			// (flags >> 24) & 0b1
			unsigned Div4 : 1;			// (flags >> 25) & 0b1
			unsigned Div8 : 1;			// (flags >> 26) & 0b1
			unsigned Div16 : 1;			// (flags >> 27) & 0b1

			unsigned GetSize(unsigned leafSize) const
			{
				leafSize <<= LeafShift;

				unsigned size;

				size =	(leafSize * Mul16	* 16);
				size += (leafSize * Mul8	* 8);
				size += (leafSize * Mul4	* 4);
				size += (leafSize * Mul2	* 2);
				size += (leafSize * Mul1	* 1);
				size += (leafSize * Div2	/ 2);
				size += (leafSize * Div4	/ 4);
				size += (leafSize * Div8	/ 8);
				size += (leafSize * Div16	/ 16);

				return size;
			}
		};

		Flags m_VirtualFlags;
		Flags m_PhysicalFlags;

		unsigned GetVirtualSize(unsigned leafSize) const { return m_VirtualFlags.GetSize(leafSize); }
		unsigned GetPhysicalSize(unsigned leafSize) const { return m_PhysicalFlags.GetSize(leafSize); }
	};
	static_assert(sizeof(datResourceInfo) == 0x8, "sizeof(datResourceInfo) == 0x8");
	static_assert(sizeof(datResourceInfo::Flags) == 0x4, "sizeof(datResourceInfo::Flags) == 0x4");

	struct datResourceFileHeader
	{
		unsigned m_Magic;
		unsigned m_Version;

		datResourceInfo m_Info;

		bool IsValidResource() const
		{
			return m_Magic == 0x37435352; // RSC7
		}
	};
	static_assert(sizeof(datResourceFileHeader) == 0x10, "sizeof(datResourceFileHeader) == 0x10");
}

#endif // _RESOURCEHEADER_H