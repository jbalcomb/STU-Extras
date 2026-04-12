

// MakeCode    (x=0X53F6A4);
// OpEnumEx    (x,  0,  GetEnum("MACRO_AF"),0);
// MakeComm    (0X53F6A8,  "protocol");
// MakeCode    (x=0X53F6AC);
// OpEnumEx    (x,  0,  GetEnum("MACRO_AF"),0);
#include <idc.idc>

static main(void)
{
    auto id;
    // Enums();              // enumerations
    Enums_0(id);

    // auto id;
    // // id = AddEnum(-1,"enum_IDC_TEST",0x2220000);
    // id = GetEnum("enum_IDC_TEST");
    // AddConstEx(id, "AF_UNSPEC",    0,  -1);
    // // AddConstEx(id, "AF_UNIX",    0X1,  -1);
    // // AddConstEx(id, "AF_INET",    0X2,  -1);
    // // AddConstEx(id, "AF_IMPLINK", 0X3,  -1);
    // // AddConstEx(id, "AF_PUP",     0X4,  -1);
    // // AddConstEx(id, "AF_CHAOS",   0X5,  -1);
    // // AddConstEx(id, "AF_NS",      0X6,  -1);
}

static Enums_0(id)
{
    id = AddEnum(-1,"e_BUILDING_TYPE",0x2220000);
    AddConstEx(id, "bt_No_Building",            0, -1);
    AddConstEx(id, "bt_Alien_Control_Center",   1, -1);
    AddConstEx(id, "bt_Armor_Barracks",         2, -1);
    AddConstEx(id, "bt_Artemis_System_Net",     3, -1);
    AddConstEx(id, "bt_Astro_University",       4, -1);
    AddConstEx(id, "bt_Atmosphere_Renewer",     5, -1);
    AddConstEx(id, "bt_Autolab",                6, -1);
    AddConstEx(id, "bt_Automated_Factory",      7, -1);
    AddConstEx(id, "bt_Battlestation",          8, -1);
    AddConstEx(id, "bt_Capitol",                9, -1);
    AddConstEx(id, "bt_Cloning_Center",         10, -1);
    AddConstEx(id, "bt_Colony_Base",            11, -1);
    AddConstEx(id, "bt_Deep_Core_Mine",         12, -1);
    AddConstEx(id, "bt_Core_Waste_Dump",        13, -1);
    AddConstEx(id, "bt_Dimensional_Portal",     14, -1);
    AddConstEx(id, "bt_Biospheres",             15, -1);
    AddConstEx(id, "bt_Food_Replicators",       16, -1);
    AddConstEx(id, "bt_Gaia_Transformation",    17, -1);
    AddConstEx(id, "bt_Currency_Exchange",      18, -1);
    AddConstEx(id, "bt_Galactic_Cybernet",      19, -1);
    AddConstEx(id, "bt_Holo_Simulator",         20, -1);
    AddConstEx(id, "bt_Hydroponic_Farm",        21, -1);
    AddConstEx(id, "bt_Marine_Barracks",        22, -1);
    AddConstEx(id, "bt_Barrier_Shield",         23, -1);
    AddConstEx(id, "bt_Flux_Shield",            24, -1);
    AddConstEx(id, "bt_Gravity_Generator",      25, -1);
    AddConstEx(id, "bt_Missile_Base",           26, -1);
    AddConstEx(id, "bt_Ground_Batteries",       27, -1);
    AddConstEx(id, "bt_Radiation_Shield",       28, -1);
    AddConstEx(id, "bt_Stock_Exchange",         29, -1);
    AddConstEx(id, "bt_Supercomputer",          30, -1);
    AddConstEx(id, "bt_Pleasure_Dome",          31, -1);
    AddConstEx(id, "bt_Pollution_Processor",    32, -1);
    AddConstEx(id, "bt_Recyclotron",            33, -1);
    AddConstEx(id, "bt_Robotic_Factory",        34, -1);
    AddConstEx(id, "bt_Research_Lab",           35, -1);
    AddConstEx(id, "bt_Robo_Miner_Plant",       36, -1);
    AddConstEx(id, "bt_Soil_Enrichment",        37, -1);
    AddConstEx(id, "bt_Space_Academy",          38, -1);
    AddConstEx(id, "bt_Spaceport",              39, -1);
    AddConstEx(id, "bt_Star_Base",              40, -1);
    AddConstEx(id, "bt_Star_Fortress",          41, -1);
    AddConstEx(id, "bt_Stellar_Converter",      42, -1);
    AddConstEx(id, "bt_Subterranean_Farms",     43, -1);
    AddConstEx(id, "bt_Terraforming",           44, -1);
    AddConstEx(id, "bt_Warp_Interdictor",       45, -1);
    AddConstEx(id, "bt_Weather_Controller",     46, -1);
    AddConstEx(id, "bt_Fighter_Garrison",       47, -1);
    AddConstEx(id, "bt_Artificial_Planet",      48, -1);

    AddConstEx(id, "bt_NUM_BUILDING_SLOTS",    100, -1);

    return id;
}

static Enums(void)
{
    auto id;
    // BeginTypeUpdating(UTP_ENUM);
    id = Enums_0(id);
    // EndTypeUpdating(UTP_ENUM);
}
