
/*
BUILDDAT.LBX

Entries:   1
Type:      5  data

Record Count:  36
Record Size:   52

File Size: 	2420

offset
0x220  544
entry length
0x974  2420 - 544 = 1876
*/

#include <stdint.h>
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), frite(), fseek(); */



// MOX_BASE.H
#define LEN_BUILDING_NAME 20

// MOM_Data.H
// sizeof:  34h  52d
#pragma pack(push)
#pragma pack(2)
struct s_BLDG_DATA
{
    /* 00 */  char name[LEN_BUILDING_NAME];
    /* 14 */  // int16_t reqd_bldg_1;       // bldg_idx  >= 100 is Terrain Type
    union
    {
        int16_t reqd_bldg_1;                // bldg_idx
        int16_t reqd_terrain;
    };
    /* 16 */  int16_t reqd_bldg_2;          // bldg_idx
    /* 18 */  int16_t replace_bldg;         // bldg_idx
    /* 1A */  int16_t Grants20XP;
    /* 1C */  int16_t Grants60XP;
    /* 1E */  int16_t Alchemist;
    /* 20 */  int16_t maintenance_cost;  // in gold units  AKA upkeep cost
    /* 22 */  int16_t AIVal_Growth;
    /* 24 */  int16_t Unknown_24h;
    /* 26 */  int16_t Unknown_26h;
    /* 28 */  int16_t AIVal_Religion;
    /* 2A */  int16_t AIVal_Research;
    /* 2C */  int16_t construction_cost;  // in production units
    /* 2E */  int16_t Unknown_2Eh;
    /* 30 */  int16_t G_Animation;  // ¿ 'size & position' for Cityscape ?
    /* 32 */  int16_t Category;
    /* 34 */
};
#pragma pack(pop)


int main(int argc, char * argv[])
{
	FILE * file_handle = NULL;
	size_t ret_code = 0;
	int itr = 0;
	int entry_offset = 544;
	int entry_length = 1876;
	int record_count = 36;
	int record_size = 52;
	uint8_t lbx_entry_data[(1876 - 4)] = { 0 };
	struct s_BLDG_DATA * bldg_data_table = NULL;
	
	printf("BEGIN\n");
	
	printf("sizeof(struct s_BLDG_DATA): %zu\n", sizeof(struct s_BLDG_DATA));
	
	file_handle = fopen("BUILDDAT.LBX", "rb");
	if(file_handle == NULL)
	{
		printf("ERROR: fopen()\n");
	}

	fseek(file_handle, (entry_offset + 4), 0);
	
	ret_code = fread(lbx_entry_data, 1, (entry_length - 4), file_handle);
	
	printf("ret_code: %zu\n", ret_code);
	
	if(ret_code == (entry_length - 4))
    {
        // for (itr = 0; itr != (entry_length - 4); itr++)
        //     printf("%02X ", lbx_entry_data[itr]);
        // putchar('\n');
    }
    else // error handling
    {
        if (feof(file_handle))
            printf("Error reading test.bin: unexpected end of file\n");
        else if (ferror(file_handle))
            perror("Error reading test.bin");
    }
	
	fclose(file_handle);
	
	file_handle = NULL;

	bldg_data_table = (struct s_BLDG_DATA *)&lbx_entry_data[0];

	// for(itr = 0; itr < 5; itr++)
	for(itr = 0; itr < record_count; itr++)
	{
		// // printf("%02X\n", lbx_entry_data[(itr * record_size)]);
		// printf("%s\n", bldg_data_table[itr].name);
		// printf("%d\n", bldg_data_table[itr].reqd_bldg_1);
		// printf("%d\n", bldg_data_table[itr].reqd_bldg_2);
		// printf("%d\n", bldg_data_table[itr].replace_bldg);
		// printf("%d\n", bldg_data_table[itr].Grants20XP);
		// printf("%d\n", bldg_data_table[itr].Grants60XP);
		// printf("%d\n", bldg_data_table[itr].Alchemist);
		// printf("%d\n", bldg_data_table[itr].maintenance_cost);
		// printf("%d\n", bldg_data_table[itr].AIVal_Growth);
		// printf("Unknown_24h: %d\n", bldg_data_table[itr].Unknown_24h);
		// printf("Unknown_26h: %d\n", bldg_data_table[itr].Unknown_26h);
		// printf("%d\n", bldg_data_table[itr].AIVal_Religion);
		// printf("%d\n", bldg_data_table[itr].AIVal_Research);
		// printf("%d\n", bldg_data_table[itr].construction_cost);
		// printf("Unknown_2Eh: %d\n", bldg_data_table[itr].Unknown_2Eh);
		// printf("G_Animation: %d\n", bldg_data_table[itr].G_Animation);
		// printf("Category: %d\n", bldg_data_table[itr].Category);
		
		printf("\"%s\", %d\n", bldg_data_table[itr].name, bldg_data_table[itr].G_Animation);
	}

	printf("END\n");
	
	return 0;
}
