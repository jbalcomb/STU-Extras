// stu_dseg_info.idc
// C:\STU_WD_IDA55\idc\stu_dseg_info.idc

#include <idc.idc>

// #include <stu_mom_def.idc>

/*
    MAGIC.EXE (MGC) Data Segment
*/
#define MGC_DSEG_BEGIN  0x00031D10
#define MGC_DSEG_END    0x0003C75A
// MGC Initialized Data
#define MGC_DSEG_DATA_BEGIN  0x00031DBA
#define MGC_DSEG_DATA_END    0x00036C8A
// MGC Uninitialized Data
#define MGC_DSEG_BSS_BEGIN   0x00036FC2
#define MGC_DSEG_BSS_END     0x0003C620

/*
    WIZARDS.EXE (WZD) Data Segment
*/
#define WZD_DSEG_BEGIN  0x00036AA0
#define WZD_DSEG_END    0x000454F4
// WZD Initialized Data (DATA)
#define WZD_DSEG_DATA_BEGIN  0x00036AA0
#define WZD_DSEG_DATA_END    0x0003EF48
// WZD Uninitialized Data (BSS)
#define WZD_DSEG_BSS_BEGIN   0x0003F330
#define WZD_DSEG_BSS_END     0x000454F4
// WZD DSEG DSEG        [0x00036AA0, 0x000454F4)
// WZD DSEG DATA        [0x00036AA0, 0x0003EF48)
// WZD DSEG BSS         [0x0003F330 ,0x000454F4)



static main(void)
{
    // stu_dseg_info();
	stu_dseg_range();
}


static stu_dseg_range(void)
{
    auto file_name;
    auto file_handle;
    auto file_cursor_position;
    Message("\n\n------ Creating the output file.... --------\n");
    file_name = GetIdbPath();
    Message("file_name: %s\n", file_name);
    file_name = substr(file_name, 0, strstr(file_name, ".")) + "_dseg_addr" + ".csv";
    Message("file_name: %s\n", file_name);
    file_handle = fopen(file_name, "w");
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    Message("dseg_addr, head_name_visible, head_name_true, addr_has_xref, addr_has_xref_code, addr_has_xref_code_nontrival, addr_has_xref_code_from, addr_has_xref_code_from_nontrival, addr_has_xref_code_to, addr_has_xref_code_to_nontrival, addr_has_xef_data, addr_has_xef_data_from, addr_has_xef_data_to\n");
    writestr(file_handle, "dseg_addr, head_name_visible, head_name_true, addr_has_xref, addr_has_xref_code, addr_has_xref_code_nontrival, addr_has_xref_code_from, addr_has_xref_code_from_nontrival, addr_has_xref_code_to, addr_has_xref_code_to_nontrival, addr_has_xef_data, addr_has_xef_data_from, addr_has_xef_data_to\n");
	
	// WZD DSEG DATA        [0x00036AA0, 0x0003EF48)
	stu_dseg_info_range(file_handle, WZD_DSEG_DATA_BEGIN, WZD_DSEG_DATA_END);
	// WZD DSEG BSS         [0x0003F330 ,0x000454F4)
	stu_dseg_info_range(file_handle, WZD_DSEG_BSS_BEGIN, WZD_DSEG_BSS_END);
	
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);
    fclose(file_handle);
}



static stu_dseg_info_range(file_handle, begin, end)
{
    auto itr_ea_addr_next;
	auto head_name_visible;
    auto head_name_true;
	auto addr_has_xref;
	auto addr_has_xref_code;
	auto addr_has_xref_code_nontrival;
	auto addr_has_xref_code_from;
	auto addr_has_xref_code_from_nontrival;
	auto addr_has_xref_code_to;
	auto addr_has_xref_code_to_nontrival;
	auto addr_has_xef_data;
	auto addr_has_xef_data_from;
	auto addr_has_xef_data_to;
    for(itr_ea_addr_next = begin; itr_ea_addr_next < end; itr_ea_addr_next = NextAddr(itr_ea_addr_next))
    {
		head_name_visible = NameEx(0, itr_ea_addr_next);
        head_name_true = GetTrueNameEx(0, itr_ea_addr_next);
		addr_has_xref = has_xref(itr_ea_addr_next);
		addr_has_xref_code = has_xref_code(itr_ea_addr_next);
		addr_has_xref_code_nontrival = has_xref_code_nontrival(itr_ea_addr_next);
		addr_has_xref_code_from = has_xref_code_from(itr_ea_addr_next);
		addr_has_xref_code_from_nontrival = has_xref_code_from_nontrival(itr_ea_addr_next);
		addr_has_xref_code_to = has_xref_code_to(itr_ea_addr_next);
		addr_has_xref_code_to_nontrival = has_xref_code_to_nontrival(itr_ea_addr_next);
		addr_has_xef_data = has_xef_data(itr_ea_addr_next);
		addr_has_xef_data_from = has_xef_data_from(itr_ea_addr_next);
		addr_has_xef_data_to = has_xef_data_to(itr_ea_addr_next);
        Message(
			"0x%08X, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            itr_ea_addr_next,
			head_name_visible,
			head_name_true,
            addr_has_xref,
			addr_has_xref_code,
			addr_has_xref_code_nontrival,
			addr_has_xref_code_from,
			addr_has_xref_code_from_nontrival,
			addr_has_xref_code_to,
			addr_has_xref_code_to_nontrival,
			addr_has_xef_data,
			addr_has_xef_data_from,
			addr_has_xef_data_to
		);
        fprintf(
            file_handle,
            "0x%08X, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            itr_ea_addr_next,
			head_name_visible,
			head_name_true,
            addr_has_xref,
			addr_has_xref_code,
			addr_has_xref_code_nontrival,
			addr_has_xref_code_from,
			addr_has_xref_code_from_nontrival,
			addr_has_xref_code_to,
			addr_has_xref_code_to_nontrival,
			addr_has_xef_data,
			addr_has_xef_data_from,
			addr_has_xef_data_to
        );
    }
}


static stu_dseg_info(void)
{
    auto itr_ea_addr_next;

    auto file_name;
    auto file_handle;
    auto file_cursor_position;

	auto head_name_visible;
    auto head_name_true;

	auto addr_has_xref;
	auto addr_has_xref_code;
	auto addr_has_xref_code_nontrival;
	auto addr_has_xref_code_from;
	auto addr_has_xref_code_from_nontrival;
	auto addr_has_xref_code_to;
	auto addr_has_xref_code_to_nontrival;
	auto addr_has_xef_data;
	auto addr_has_xef_data_from;
	auto addr_has_xef_data_to;


    Message("\n\n------ Creating the output file.... --------\n");

    file_name = GetIdbPath();
    Message("file_name: %s\n", file_name);
    file_name = substr(file_name, 0, strstr(file_name, ".")) + "_dseg_addr" + ".csv";
    Message("file_name: %s\n", file_name);
    file_handle = fopen(file_name, "w");
    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);


    Message("dseg_addr, head_name_visible, head_name_true, addr_has_xref, addr_has_xref_code, addr_has_xref_code_nontrival, addr_has_xref_code_from, addr_has_xref_code_from_nontrival, addr_has_xref_code_to, addr_has_xref_code_to_nontrival, addr_has_xef_data, addr_has_xef_data_from, addr_has_xef_data_to\n");

    writestr(file_handle, "dseg_addr, head_name_visible, head_name_true, addr_has_xref, addr_has_xref_code, addr_has_xref_code_nontrival, addr_has_xref_code_from, addr_has_xref_code_from_nontrival, addr_has_xref_code_to, addr_has_xref_code_to_nontrival, addr_has_xef_data, addr_has_xef_data_from, addr_has_xef_data_to\n");


    for(itr_ea_addr_next = WZD_DSEG_BEGIN; itr_ea_addr_next < WZD_DSEG_END; itr_ea_addr_next = NextAddr(itr_ea_addr_next))
    {
		head_name_visible = NameEx(0, itr_ea_addr_next);
        head_name_true = GetTrueNameEx(0, itr_ea_addr_next);

		addr_has_xref = has_xref(itr_ea_addr_next);
		addr_has_xref_code = has_xref_code(itr_ea_addr_next);
		addr_has_xref_code_nontrival = has_xref_code_nontrival(itr_ea_addr_next);
		addr_has_xref_code_from = has_xref_code_from(itr_ea_addr_next);
		addr_has_xref_code_from_nontrival = has_xref_code_from_nontrival(itr_ea_addr_next);
		addr_has_xref_code_to = has_xref_code_to(itr_ea_addr_next);
		addr_has_xref_code_to_nontrival = has_xref_code_to_nontrival(itr_ea_addr_next);
		addr_has_xef_data = has_xef_data(itr_ea_addr_next);
		addr_has_xef_data_from = has_xef_data_from(itr_ea_addr_next);
		addr_has_xef_data_to = has_xef_data_to(itr_ea_addr_next);


        Message(
			"0x%08X, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            itr_ea_addr_next,
			head_name_visible,
			head_name_true,
            addr_has_xref,
			addr_has_xref_code,
			addr_has_xref_code_nontrival,
			addr_has_xref_code_from,
			addr_has_xref_code_from_nontrival,
			addr_has_xref_code_to,
			addr_has_xref_code_to_nontrival,
			addr_has_xef_data,
			addr_has_xef_data_from,
			addr_has_xef_data_to
		);

        fprintf(
            file_handle,
            "0x%08X, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            itr_ea_addr_next,
			head_name_visible,
			head_name_true,
            addr_has_xref,
			addr_has_xref_code,
			addr_has_xref_code_nontrival,
			addr_has_xref_code_from,
			addr_has_xref_code_from_nontrival,
			addr_has_xref_code_to,
			addr_has_xref_code_to_nontrival,
			addr_has_xef_data,
			addr_has_xef_data_from,
			addr_has_xef_data_to
        );

    }

    file_cursor_position = ftell(file_handle);
    Message("file_cursor_position: %d\n", file_cursor_position);

    fclose(file_handle);

}

static has_xref(ea)
{
	return Rfirst(ea) != BADADDR || RfirstB(ea) != BADADDR || Dfirst(ea) != BADADDR || DfirstB(ea) != BADADDR;
}

static has_xref_code(ea)
{
	return Rfirst(ea) != BADADDR || RfirstB(ea) != BADADDR;
}

static has_xref_code_nontrival(ea)
{
	return Rfirst0(ea) != BADADDR || RfirstB0(ea) != BADADDR;
}

static has_xref_code_from(ea)
{
	return Rfirst(ea) != BADADDR;
}

static has_xref_code_from_nontrival(ea)
{
	return Rfirst0(ea) != BADADDR;
}

static has_xref_code_to(ea)
{
	return RfirstB(ea) != BADADDR;
}

static has_xref_code_to_nontrival(ea)
{
	return RfirstB0(ea) != BADADDR;
}

static has_xef_data(ea)
{
	return Dfirst(ea) != BADADDR || DfirstB(ea) != BADADDR;
}

static has_xef_data_from(ea)
{
	return Dfirst(ea) != BADADDR;
}

static has_xef_data_to(ea)
{
	return DfirstB(ea) != BADADDR;
}
