#include <idc.idc>

static rename_mgc_proc(old_name, new_name)
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


    Message("mgc-rename-from-wzd-summary,attempted=%d,ok=%d,failed=%d\n", attempted, ok, attempted - ok);
    Batch(0);
    if ( ok == attempted )
        Exit(0);
    Exit(1);
}
