
#include "LoadGameFiles.hpp"



uint8_t* mgc_buf = NULL;
uint8_t* wzd_buf = NULL;
uint8_t* gam_buf = NULL;

uint8_t* mgc_dseg = NULL;
uint8_t* wzd_dseg = NULL;


struct s_UNITTYPE* unit_type_table;
// uint8_t* unit_type_table;

struct s_RACE* race_table;

struct s_UNIT* _UNITS;



void Load_MAGIC_EXE(void)
{
    mgc_buf = Load_Entire_File("MAGIC.EXE");

    if (NULL == mgc_buf)
    {
        fprintf(stderr, "FAILURE: Could not load MAGIC.EXE.\n");
    }
}


void Load_WIZARDS_EXE(void)
{
    wzd_buf = Load_Entire_File("WIZARDS.EXE");

    if (NULL == wzd_buf)
    {
        fprintf(stderr, "FAILURE: Could not load WIZARDS.EXE.\n");
    }

    wzd_dseg = wzd_buf + WZD_OFS_DSEG;

    unit_type_table = (struct s_UNITTYPE*)(wzd_buf + WZD_OFS_UTT);
    // unit_type_table = (uint8_t*)(wzd_buf + WZD_OFS_UTT);

    // unit_race_table = (struct s_RACE*)(wzd_buf + WZD_OFS_URT);
    race_table = (struct s_RACE*)(wzd_buf + WZD_OFS_RT);


}


void Load_SAVE_GAM(const char* savegam_filename)
{
    // win_MoM.cpp  Load_SAVE_GAM(-1);  "SAVETEST.GAM"

    gam_buf = Load_Entire_File(savegam_filename);

    if (NULL == gam_buf)
    {
        fprintf(stderr, "FAILURE: Could not load %s.\n", savegam_filename);
        // return NULL;
    }

    _UNITS = (struct s_UNIT*)(gam_buf + GAM_OFS_UNITS);

}

void Load_Game_Files(const char* savegam_filename)
{
    Load_MAGIC_EXE();
    Load_WIZARDS_EXE();
    Load_SAVE_GAM(savegam_filename);
}

void Unload_Game_Files(void)
{
    free(gam_buf);
    free(wzd_buf);
    free(mgc_buf);
}
