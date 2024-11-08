
#include <stdint.h>

// 1st Terrain Picture Index: 2
// Offset in LBX File to 1st Terrain Picture: 2 * 384 = 768
// Offset in LBX Entry to 1st Terrain Picture: 0xC0


// WCHAR szFileName[MAX_PATH] = L"TERRAIN.LBX";
// File Size: 681564
// Entries: 3
// Offsets:
// Lengths:

// 03 00 AD FE 00 00 05 00 
// 40 02 00 00
// 80 54 0A 00
// 68 60 0A 00
// 5C 66 0A 00
// 0x00000240
// 0x000A5480
// 0x000A6068
// 0x000A665C
// 000A5480 - 00000240 = A5240  676416
// 000A6068 - 000A5480 =   BE8    3048  3048 / 4 = 762 tiles, per plane
// 000A665C - 000A6068 =   5F4    1524  1524 / 2 = 762 tiles, per plane

// terrain pict
// (useless) 8 byte header
// width: 20
// height: 18
// 20 * 18 = 360
// 384 - 360 = 24
// ? 8 byte header ? + 8 0 pre-padding + 8 byte 0 post-padding ?
// 8 + 8 pre to match other image functions expectations that the header is 1 paragraph
// then 8 post to align to paragraph boundary

// 1761 picts
// 1761 * 360 = 633960
// 1761 * 384 = 676224

// Terrain - Sprites
// BOD: 0x00000300     768
// EOD: 0x000A5480  676992
// LOD: 0x000A5180  676224



uint8_t terrain_000[676416];
uint8_t terrain_001[3048];
uint8_t terrain_002[1524];
// 676224

struct s_TERRAIN_PICT
{
	// 8
	// 8
	// 360
	// 8
};



void Load_TERRAIN_LBX(void)
{
	// paFileData
	// cbFileData
	Load_File("TERRAIN.LBX", terrain_lbx, cbTerrainLbx);
	
}
void Load_TERRAIN_000(void)
{

}
void Load_TERRAIN_001(void)
{

}
void Load_TERRAIN_002(void)
{

}
void Load_Terrain_Picts(void)
{

}
void Load_Terrain_Picts_Palette(void)
{
	
}
