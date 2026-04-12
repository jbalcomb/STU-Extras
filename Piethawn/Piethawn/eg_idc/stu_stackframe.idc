
// IDA IDC Stack Frame
// https://zairon.wordpress.com/2008/02/15/idc-script-and-stack-frame-variables-length/
/*
The stack frame created by Ida is divided into some parts, it looks like a sequence of fields:
 – local variables
 – saved registers
 – return address
 – function parameters
*/
/*
GetFrame
 - returns the id of the function frame structure
 - need the id when you deal with the internal fields of the structure
 - once you have the id you can start scanning the entire structure from the first till the last item
GetFirstMember and GetLastMember
 - return the first and the last offset
 - now you can retrieve all the information you need
GetMemberName
GetMemberSize
GetMemberFlag
    idc.idc
    #define FF_DATA 0x00000400L // Data ?
    #define FF_0OFF 0x00500000L // Offset?
    #define FF_1OFF 0x05000000L // Offset?
    #define FF_DWRD 0x20000000L // dword
*/
/*
Ida repeats the field information on every byte of the field itself. To display only one item per field you can update the ‘i’ variable inside the for statement, add the next line after the Message instruction:
i = i + GetMemberSize(id, i) - 1;
*/
/*
It works almost fine, but it goes into an infinite loop with certain functions.
From what I’ve seen the problem occours when Ida is not able to understand which kind of variable has been declared.
Look at this simple example:
    ...
    INIT:BF9B0786 var_1A4 = dword ptr -1A4h
    INIT:BF9B0786 var_18E = byte ptr -18Eh
    INIT:BF9B0786 var_4 = dword ptr -4
    ...
var_18E is marked as byte but there’s a big gap between var_18E and var_4. Stack frame windows reveals an interesting thing:
    -0000018E var_18E db ?
    -0000018D db ? ; undefined
    -0000018C db ? ; undefined
    -0000018B db ? ; undefined
    -0000018A db ? ; undefined
    -00000189 db ? ; undefined
    -00000188 db ? ; undefined
    -00000187 db ? ; undefined
    -00000186 db ? ; undefined
    -00000185 db ? ; undefined
    -00000184 db ? ; undefined
    -00000183 db ? ; undefined
var_18E length is 394 byte and as you can see Ida doesn’t collapse the definition into a single line, but it “explodes” the variable through the 394 bytes.
How can you solve this problem?
 You can use my initial script adding some more checks.
 Nothing hard of course, you have all the necessary functions, just use your brain defining a good algo.
 Hint: take a look at the value returned by GetMemberSize.
*/

#include <idc.idc>

static main()
{
    auto function_address;
    
    function_address = 0x00013C92; // Address of the function to check

    get_stack_frame(function_address);

}


static get_stack_frame(long function_address)
{
    auto id, i, firstM, lastM, address;
    auto mName, mSize, mFlag;
    
    id = GetFrame(address);
    firstM = GetFirstMember(id);
    lastM = GetLastMember(id);

    for(i=firstM;i<=lastM;i++)
    {
        mName = GetMemberName(id,i); // Get the name
        mSize = GetMemberSize(id, i); // Get the size (in byte)
        mFlag = GetMemberFlag(id, i); // Get the flag
        Message(“\n%s %d %x”, mName, mSize, mFlag);
    }

}


// #include <idc.idc>

// static main()
// {
    // auto id, i, firstM, lastM, address;
    // auto mName, mSize, mFlag;

    // address = 0x00013C92;               // Address of the function to check
    // id = GetFrame(address);
    // firstM = GetFirstMember(id);
    // lastM = GetLastMember(id);

    // for ( i = firstM; i <= lastM; i++ )
    // {
        // mName = GetMemberName(id, i);   // Get the name
        // mSize = GetMemberSize(id, i);   // Get the size (in byte)
        // mFlag = GetMemberFlag(id, i);   // Get the flag
        // Message(“\n%s %d %x”, mName, mSize, mFlag);
    // }
// }
