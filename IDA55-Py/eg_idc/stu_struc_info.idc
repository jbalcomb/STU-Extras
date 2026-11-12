GetFirstStrucIdx() 

 GetStrucComment
 GetStrucIdByName
 GetStrucId
 GetStrucIdx
 GetStrucName
 GetStrucNextOff
 GetStrucPrevOff
 GetStrucQty
 GetStrucSize

GetMemberStrId  

***********************************************
** get structure id of a member
        arguments:      id            - structure type ID
                        member_offset - member offset. The offset can be
                                        any offset in the member. For example,
                                        is a member is 4 bytes long and starts
                                        at offset 2, then 2,3,4,5 denote
                                        the same structure member.
        returns:        -1 if bad structure type ID is passed
                           or no such member in the structure
                        otherwise returns structure id of the member.
                        If the current member is not a structure, returns -1.

long GetMemberStrId(long id,long member_offset);


SetMemberName  

***********************************************
** change structure member name
        arguments:      id            - structure type ID
                        member_offset - offset of the member
                        name          - new name of the member
        returns:        !=0 - ok.

long SetMemberName(long id,long member_offset,string name);









AddStrucEx()

GetStrucIdByName()

AddStrucMember()





static Structures_0(id)
{
	
	auto mid;
		
	id = AddStrucEx(-1,"FORT_REC",0);

	id = GetStrucIdByName("FORT_REC");
	
	mid = AddStrucMember(id,"Location",	0X0,	0x60000400,	GetStrucIdByName("LOC_8BIT"),	3);
	mid = AddStrucMember(id,"Active",	0X3,	0x000400,	-1,	1);

}


//------------------------------------------------------------------------
// Information about structure types

static Structures(void)
{
	auto id;
	
	id = Structures_0(id);
	
}

// End of file.
