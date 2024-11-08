#pragma once

#include "STUlib.hpp"



extern uint8_t* mgc_buf;
extern uint8_t* wzd_buf;
extern uint8_t* gam_buf;

extern uint8_t* mgc_dseg;
extern uint8_t* wzd_dseg;


extern struct s_UNITTYPE* unit_type_table;
// extern uint8_t* unit_type_table;

extern struct s_UNIT* _UNITS;



void Load_MAGIC_EXE(void);
void Load_WIZARDS_EXE(void);
void Load_SAVE_GAM(const char* savegam_filename);

void Load_Game_Files(const char* savegam_filename);
void Unload_Game_Files(void);
