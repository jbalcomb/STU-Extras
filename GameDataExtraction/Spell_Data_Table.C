
/*
SPELLDAT.LBX

Entries:   1
Type:      5

Record Count:  215
Record Size:    36


*/

#include <stdint.h>
#include <stdio.h>      /* FILE; fclose(), fopen(), fread(), frite(), fseek(); */

/* MOX_BASE.H */
#define LEN_SPELLDAT_NAME 19

/* MOM_Data.H */
enum e_SPELL_BOOK_CATEGORY
{
    sbc_Special      = 0,
    sbc_Summoning    = 1,
    sbc_Enchantment  = 2,
    sbc_City         = 3,
    sbc_Unit         = 4,
    sbc_Combat       = 5,
};

enum e_SPELL_CASTING_CATEGORY
{
    sdt_Summoning           =  0,
    Unit_Enchantment        =  1,
    City_Enchantment        =  2,
    City_Curse              =  3,
    Fixed_Dmg_Spell         =  4,
    Special_Spell           =  5,
    Target_Wiz_Spell        =  6,
    sdt_Global_Enchantment  =  9,
    Battlefield_Spell       = 10,
    sdt_Crafting_Spell      = 11,
    Destruction_Spell       = 12,
    Resistable_Spell        = 13,
    No_Resist_Spell         = 14,
    Mundane_Enchantment     = 15,
    Mundane_Curse           = 16,
    sdt_Infusable_Spell     = 17,
    Dispel_Spell            = 18,
    Disenchant_Spell        = 19,
    Disjunction_Spell       = 20,
    Counter_Spell           = 21,
    Var_Dmg_Spell           = 22,
    Banish_Spell            = 23
};

/* spell_data_table */
struct s_SPELL_DATA
{
    /* 00 */ char name[LEN_SPELLDAT_NAME];  // ¿ (LEN_SPELL_NAME - 1) ?
    /* 13 */  int8_t AI_Group;              //  ; enum AI_Spell_Groups
    /* 14 */  int8_t AI_Value;
    /* 15 */  int8_t type;                  //  enum e_SPELL_CASTING_CATEGORY (SCC)
    /* 16 */  int8_t spell_book_category;  // 1-byte, signed;  "spell category" "categories: summoning spells, special spells, city spells, enchantments, unit spells, combat spells and research spells"
    /* 17 */  int8_t magic_realm;  // magic realm  ¿ sbr_ ?
    /* 18 */  int8_t Eligibility;   /* ¿ overland, combat, ... ? */
    /* 19 */  /* 2-byte alignment padding */
    /* 1A */  int16_t Casting_Cost;
    /* 1C */  int16_t research_cost;
    /* 1E */  int8_t Sound;
    /* 1F */  /* 2-byte alignment padding */
    // /* 20 */  int8_t Param0;                // 2-byte, signed;  ; unit type, base damage, UE flag, or CE index;  for sdt_Global_Enchantment, used to index _players[].Globals[]
    // /* 21 */  int8_t Param1;
    union
    {
        /* 20 */  int8_t Param0;
        /* 21 */  int8_t Param1;
        int16_t unit_type;                  // 0: sdt_Summoning IDK_SummonAnim_Draw()
        int16_t ge_idx;                     // 9: sdt_Global_Enchantment
        uint8_t cmbt_0;                     // Attack_Strength  CMB_ConvSpellAttack()
        uint8_t cmbt_1;                     // Immunity_Flags   CMB_ConvSpellAttack()
    };
    /* 22 */  int16_t Params2_3;            //  ; ATK_Flags
    /* 24 */
};

int main(int argc, char * argv[])
{
	FILE * file_handle = NULL;
	size_t ret_code = 0;
	uint8_t lbx_entry_data[(7744 - 4)] = { 0 };
	int itr = 0;
	struct s_SPELL_DATA * spell_data = NULL;
	
	printf("BEGIN\n");
	
	printf("sizeof(struct s_SPELL_DATA): %zu\n", sizeof(struct s_SPELL_DATA));
	
	file_handle = fopen("SPELLDAT.LBX", "rb");
	if(file_handle == NULL)
	{
		printf("ERROR: fopen()\n");
	}

	fseek(file_handle, (544 + 4), 0);
	
	ret_code = fread(lbx_entry_data, 1, (7744 - 4), file_handle);
	
	printf("ret_code: %zu\n", ret_code);
	
	if(ret_code == (7744 - 4))
    {
        // for (itr = 0; itr != (7744 - 4); itr++)
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

	spell_data = (struct s_SPELL_DATA *)&lbx_entry_data[0];

	// for(itr = 0; itr < 5; itr++)
	for(itr = 0; itr < 215; itr++)
	{
		// printf("%02X\n", lbx_entry_data[(itr * 36)]);
		// printf("%s\n", spell_data[itr].name);
		// printf("%d\n", spell_data[itr].type);
		printf("\"%s\", %d\n", spell_data[itr].name, spell_data[itr].type);
	}

	printf("END\n");
	
	return 0;
}