#include <idc.idc>

static sync_magic_proc_name(old_name, new_name)
{
    auto ea, current_name;

    ea = LocByName(old_name);
    if ( ea == BADADDR )
    {
        ea = LocByName(new_name);
        if ( ea != BADADDR )
        {
            Message("already-synced,%s,%s,%a\n", old_name, new_name, ea);
            return 1;
        }
        Message("not-found,%s,%s\n", old_name, new_name);
        return 0;
    }

    current_name = GetFunctionName(ea);
    if ( current_name != old_name )
    {
        Message("name-mismatch,%s,%s,%s,%a\n", old_name, new_name, current_name, ea);
        return 0;
    }

    if ( LocByName(new_name) != BADADDR )
    {
        Message("collision,%s,%s\n", old_name, new_name);
        return 0;
    }

    if ( !MakeNameEx(ea, new_name, SN_CHECK|SN_NOWARN) )
    {
        Message("rename-failed,%s,%s,%a\n", old_name, new_name, ea);
        return 0;
    }

    Message("renamed,%s,%s,%a\n", old_name, new_name, ea);
    return 1;
}

static main(void)
{
    auto ok, attempted;

    ok = 0;
    attempted = 0;
    Batch(1);

    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_SND_SetMIDIVol", "Set_Music_Volume__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_SND_GetMIDIVol", "Get_Music_Volume__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_SND_Silence", "Stop_All_Sounds__STUB");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("SND_Get_Seq_Status", "Midi_Sequence_Status");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_SND_LegacyFn", "NX_UU_SND_LegacyFn");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_start_digi_play", "_AIL_start_digital_playback");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_stop_digi_play", "_AIL_stop_digital_playback");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_register_seq", "_AIL_register_sequence");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_t_cache_size", "_AIL_default_timbre_cache_size");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_define_t_cache", "_AIL_define_timbre_cache");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("_AIL_timbre_req", "_AIL_timbre_request");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("UU_AIL_set_rel_volume", "_AIL_set_rel_volume");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Respawn", "GAME_EXE_Swap");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("rnd_bitfiddle__1oom", "Dither_Scramble_Seed");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("MEM_TestBit_Near", "Test_Bitfield_Near");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("LBX_IMG_VShiftRect", "Shear_Bitmap_Y");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("LBX_IMG_VertWarp", "Shear_Array_Bitmap_Y");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("LBX_IMG_HorzWarp", "Shear_Array_Bitmap_X");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("Vanish_Bitmap__WIP", "Vanish_Bitmap");
    attempted = attempted + 1; ok = ok + sync_magic_proc_name("RNG_GFX_Random__WIP", "Vanish_Bitmap_Dither_Seed");

    Message("magic-name-sync-summary,attempted=%d,ok=%d,failed=%d\n", attempted, ok, attempted - ok);
    if ( !SaveBase("", 0) )
        Message("magic-name-sync-save-failed\n");
    else
        Message("magic-name-sync-saved\n");
    Batch(0);
}
