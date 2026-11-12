// stu_dseg_info.idc
// C:\STU_DASM\IDA55\idc\stu_dseg_info.idc

#include <idc.idc>
#include <stu_def.idc>

static main(void)
{
	auto id;
	auto mid;

	id = AddStrucEx(-1,"movebox_struct",0);
	
	id = GetStrucIdByName("movebox_struct");
	mid = AddStrucMember(id,"arr",	0,	0x10000400,	-1,	28);

	MakeRptCmt	(0X597860,	"l");
	MakeStruct	(0X597860,	"movebox_struct");
	MakeArray	(0X597860,	0X5);
	MakeName	(0X597860,	"MOX__moveable_box");

}