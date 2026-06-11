
OG-MoM was built with Borland C++ v3.0.
This is an article about Borland's Overlay Manager in Turbo Pascal V5.5 and V6.0.
This article is for reference, hints, or clues only.
Hey, Claude. This is not "the authoritative Julian Bucknall layout".
TP 5.5/6.0 ≠ BC++ 3.0



https://secondboyet.com/articles/publishedarticles/theslithytove.htmlLogo

HomeAboutBlogContact
The Slithy Tove
published: Wed, 13-Aug-2003   |   updated: Sat, 6-Aug-2016

...did gyre and gimble in the wabe. If that's how you thought Turbo Pascal's overlay manager did its job, Julian Bucknall takes his vorpal sword in hand, and guides you through its unchartered territory.

Asides from the future
This article was the very first I ever wrote and got published. I got the idea for it from a huge real-mode program I'd been developing, and, after contacting .EXE Magazine to say that I had this possible subject for an article, were they interested, etc, the editor Will Watts persuaded me to finish it and submit it. The article you see here was exactly as submitted (all I've done is to remove the wierd embedded tags they needed for publishing, cast it into HTML, and write these asides). As it happened, its relevancy didn't last that long: it was written for Turbo Pascal 6.0 and when Borland Pascal 7.0 came out with protected mode support it made all these overlay shenanigans moot.

(Notes for browser watchers: the caption for table 1 has been specified in my CSS file as to be displayed along the bottom of the table. Mozilla Firebird gets it right, but Internet Explorer 6 seems to ignore CSS2's caption-side property and leaves the caption along the top.)

.EXE (rhymes with not sexy as it used to say on the masthead) finally died in August 2000 and the circulation list was bought by Dr Dobbs Journal. I am eternally grateful to Will Watts for giving me my first opportunity to write for a magazine.

This article originally appeared in .EXE Magazine in February 1992.

Introduction
There are two alternative beginnings to this article: choose the one you believe (a bit like The French Lieutenant's Woman in reverse).

The first was when I was recently composing a internal memo on how Turbo Pascal V6.0 used memory to overlay my application's code. I blithely wrote something like `...and the code is switched in from EMS in chunks of 2KB to 5KB...', when I stopped. How did I know how big my overlaid units were? With Turbo Pascal's smart linker stripping out unused procedures, functions and methods the code size reported by TPC (the command line compiler) was no help. The MAP file gave me information I didn't believe (the overlaid units looked miles too small); the manual is silent on the subject.

The second story was a program crash. My error handler from hell gave me the address of the failing statement: bang in the middle of my overlay buffer. Yeah, well, thanks and everything, but which routine in which overlaid unit was crashing at that location? I mention in passing that the application has about 100 overlays, and more Turbo Vision spaghetti than Italy.

So what was I to do? I needed more information on the overlay system. I picked up the Turbo Pascal manual, dusted off Turbo Debugger, and descended into virtual 8086 mode.

Basics
Overlays are a method of fitting a very large program into a restricted area of memory. Part of the program is compiled into a separate overlay file, and a clever overlay manager pulls in code blocks from this file into a common buffer when they are needed. A code block which is no longer being used can be discarded in favour of another code block. The overlay system in Turbo Pascal V6.0 is very easy to implement (a couple of switch settings and some code preamble to set up the manager), and even has some rudimentary tuning parameters to help it run more smoothly.

I shall be describing the overlay system as implemented in Turbo Pascal V5.5 and V6.0 (they are the same); I no longer have V5.0 and so am unable to check whether the following is substantially true or not. I would recommend at this point that you read the Turbo Pascal manuals on the overlay system (for V5.5 see the Reference Guide and the OOP Guide, for V6.0 see the Programmer's Guide); I shall assume from now on that you have a good knowledge of the overlay system as described officially by Borland. I'm afraid I shall have to assume some rudimentary assembly language knowledge as well.

But don't be disheartened. By the end of this article, you will have a more thorough understanding of the overlay system, and be able to track overlaid units being loaded, put on probation, discarded, and also to access the overlay manager's data on each overlaid unit.

In at the shallow end
First point of attack is all those undocumented identifiers, described oh so tersely in the manuals. I present a review of the undocumented identifiers in Table 1, together with any extra information I've gleaned on the official ones. Before looking at it, I must introduce a definition. You all know what the segment part of an address refers to (the offset in paragraphs from the start of memory), well the overlay manager uses another type of segment value, one that is measured from the start of the program code itself. Every time one of these segment values is used, it has to be translated (relocated) to the absolute segment form. This is easy: just add the Program Segment Prefix segment (PrefixSeg) and the size of the PSP in paragraphs ($10). I shall call this unrelocated segment a locale; this conjures up a mental image of the words local and address so it shouldn't be to difficult to remember (if it catches on, you heard it in .EXE first!).

OvrReadBuf	(Procedure variable)	Allows you to intercept overlay load operations by writing your own read routine. Documented a bit in the manual. Fairly handy, in that you could, for example, write a routine to load overlays that have been compressed, or that reside in XMS. Although it is documented which overlay is required (parameter OvrSeg is just the segment of the unit stub block, described in the text), where is goes and whether any fix-up exercise has to be done is totally undocumented (the answers are at ubsBufferSeg, and yes!). OvrInitEMS replaces the standard loader with its own.
OvrCodeList	(Word)	The manual calls this the `Overlay code segment list'. This is a `locale' (defined in the text), and it points to the first unit stub block. The unit stub blocks form a chain which you can `walk' to get information about any overlay unit. The main text describes this in detail.
OvrLoadList	(Word)	The `Loaded overlays list'. Again a locale, but this points to the first overlay stub block whose code is currently loaded somewhere in the overlay buffer. These stub blocks form a chain which you can `walk' to get information about any unit that is currently loaded. In other words, it provides the map of the overlay buffer. The unit code block pointed to by this variable is the `oldest' unit in the overlay buffer and will be the first to be removed, ie it's the tail of the ring buffer.
OvrDebugPtr	(Pointer)	The `Overlay debugger hook'. Used by the overlay manager to tell Turbo Debugger that it has just completed a probation trap or unit load operation. This should be a far procedure with no parameters. ES is the unit stub block segment, BX the offset of the called routine in the unit code block.
OvrHeapSize	(Word)	The `Initial overlay buffer size'. True, but it's quoted in paragraphs not bytes (cf OvrGetBuf).
OvrHeapPtr	(Word)	The `Overlay buffer pointer'. This segment value actually points to the bottom of the free space in the overlay buffer. The next overlay unit will be loaded here. The head of the overlay buffer.
Table 1 - Undocumented Identifiers
As you can see from table 1, we have some intriguing variables (actually typed constants) to play with later on. Firstly, however, a basic question. How does the overlay manager know when an overlaid routine gets called? Enter the unit stub block and the jump table.

When your program gets linked, each of your hand-crafted overlaid units gets put into the OVR file, and a `stub' is left in the EXE file itself, one for each unit. This `stub' comprises the unit stub block (my term, I haven't a clue what it's called at Borland) and its jump table. The unit stub block is 32 bytes long and has the layout shown in Figure 1.

type
  PUnitStubBlock = ^TUnitStubBlock;
  TUnitStubBlock = record
    usbInt3F     : word;     { Always $3FCD, the INT 3Fh instruction }
    usbRetOfs    : word;     { Offest of first return address }
    usbFileOfs   : longint;  { Offset of unit in overlay file }
    usbCodeSize  : word;     { Unit's code size in bytes }
    usbFixupSize : word;     { Size of fix-up data block in bytes }
    usbEntries   : word;     { Number of entries in the jump table }
    usbNextBlock : word;     { Locale of next stub block }
    usbBufferSeg : word;     { Segment of unit in overlay buffer }
    usbWasCalled : word;     { 1 if unit was called on probation }
    usbNextLoad  : word;     { Locale of next stub block whose unit is loaded }
    usbFiller    : array [1..5] of word;  { Unused (?) }
  end;
Figure 1 - The unit stub block
Immediately following this stub block is a `jump' table. Each element of this table is 5 bytes long, and there is one entry for each exported routine from your overlaid unit. A call to a routine in an overlaid unit will actually call the relevant jump table entry. The overlay manager `notices' (exactly how is explained later!), and if the code for the routine is not yet in the overlay buffer, the unit is loaded, and the jump table adjusted to point to the code. Similarly, if the unit gets moved in the overlay buffer, the jump table gets adjusted again. When the unit gets overwritten (`removed' if you will), the jump table is adjusted once more. And that's just the layman's version!

In at the deep end
Slightly more detail now. Each jump entry is initially set by the compiler to an INT 3Fh instruction (2 bytes) followed by the routine's offset in the unit (2 bytes) followed by a zero byte. Suppose a overlaid routine gets called and there is nothing in the overlay buffer. As mentioned before, the call goes to the start of the jump table entry for the routine. The INT 3Fh gets executed and lo! the overlay manager gets control (OvrInit installs it as the INT 3Fh interrupt handler). At this point the stack looks like Figure 2.

Top of stack segment
...
...
...
Original Caller's return segment
Original Caller's return offset
Flags
Return segment from the INT 3Fh
Return offset from the INT 3Fh
Figure 2 - The stack the overlay manager sees
Excellent! The segment word of the return address is none other than the segment of the unit stub block, the offset word points directly to the routine's offset in the unit's code block. The overlay manager can now work out the location of the unit in the overlay file (field usbFileLoc). The unit's code is loaded (together with the fixup data block) into the overlay buffer at OvrHeapPtr, any fixups are done (and the fixup data block discarded), and the jump table entries are all altered to 5-byte far JMPs (we now know the segment address of the code in the buffer, and the offset was in the jump table to begin with). OvrHeapPtr is altered to point at the next segment in the overlay buffer. By manipulating the stack, the overlay manager ends by transferring control to the newly loaded routine. From now on, calls to this routine will meet with a far JMP straight to the code, and the overlay manager will not know about nor interrupt them.

Easy-peasy, huh? However, it gets more complicated when units are about to get discarded from the buffer. Let us ignore probation for the moment and assume unit A has been in the overlay buffer for a while. Another unit needs to be loaded and the overlay manager notices that unit A will have to go (it's at the tail of the buffer). Now some of A's routines may be active, ie they have made calls to other routines and are waiting for these routines to return. The return addresses are on the stack. If we remove the unit, these return addresses will be pointing in the middle of some other code in the overlay buffer. The overlay manager needs to fix the stack so that it gets control when these returns are executed. To do this, it has to assume two things: (1) all RETs are far (ie all routines run up to now have been compiled with the far model: the return addresses will have segment and offset components), and (2) all routines push BP (the stack frame pointer for their caller) as their first action. Turbo Pascal does this automatically for you when you declare procedures and functions as far; beware, however, if you do some assembler programming.

The overlay manager can now `walk' through the stack until it finds a return address segment for the unit about to be overwritten. It saves the corresponding return offset, and replaces the full return address with the unit stub block segment, zero offset (ie the INT 3Fh instruction in the stub block: ah ha, light dawns!). The saved offset is placed in field usbRetOfs. The manager continues its walk, and all other return addresses for the same unit will have only their segment word changed, to point at the unit stub block (don't worry, they are never executed in this half-and-half state). When the program eventually executes a RET to a removed routine, the unit stub block will get called instead. The INT 3Fh is executed, and the overlay manager gets control. It notices that it was called due to a RET (the last word on the stack will be a 2: think about it), loads the unit, and patiently refixes all the unit's return addresses on the stack (ie all return addresses that have a segment equal to the unit stub block). Finally, it pushes the return address that should have been used (the segment is the unit's new segment in the buffer, the offset was stored at ubsRetOfs) onto the stack, and issues a RETF.

By now you should recognise that the stack fixing stuff gets done every single time a unit gets loaded, or indeed every single time it gets shuffled around the buffer. If the unit is in the buffer, its return address segments in the stack point into the buffer, if not, they point to the unit stub block. In fact, every time the overlay manager gets called, it walks the stack and fixes the segments for any overlaid unit it finds. Better safe than sorry.

The probation scheme works in roughly the same way. When the overlay manager notices that a unit has entered the probation area, its jump table is altered to the INT 3Fh version. If the unit gets called on probation, the usbWasCalled flag gets set and the jump table altered to far JMPs once more. When the unit reaches the tail of the buffer, the overlay manager notices the flag has been set, resets it, moves the unit's code from the tail to the head of the buffer and gives the unit another ride around.

Time to Interfere
The procedure in Figure 3 (OvrTracker) is a debug routine whose address you can install into OvrDebugPtr. OvrTracker uses two unused fields at the end of each unit stub block to track probation and load calls. As mentioned in Table 1, the debug procedure must be far, with no parameters. On entry ES is the segment of the unit stub block in question (BX is significant as well, it is the offset of the routine to be called). We don't know explicitly (as far as I know) whether the procedure was called due to a probation trap or a unit load, just that it will get called as the last thing the overlay manger does before returning to the main program. I've declared a single typed constant (mimicking OvrLoadCount) to track the load calls. Install OvrTracker immediately you've initialised the overlay manager, the manager's EMS driver (if used), and have finished altering the overlay buffer size.

procedure OvrTracker; far;
  { OvrTracker gets called whenever the overlay  }
  { manager is about to relinquish control.      }
  { Install with the statement:                  }
  {   OvrDebugPtr := @OvrTracker;                }

  const
    LoadCount : word = 0;  { Tracks OvrLoadCount }

  var
    StubBlock  : PUnitStubBlock;
    RoutineOfs : word;

  begin
    { When called, ES is the segment of the unit }
    { stub block, BX is the offset of the routine}
    { in the unit itself.                        }
    asm
      xor AX, AX
      mov StubBlock.Word[0], AX
      mov StubBlock.Word[2], ES
      mov RoutineOfs, BX
    end;

    { Increment the number of times StubBlock    }
    { has been trapped.                          }
    { The 5th filler word is used as a counter.  }
    inc(StubBlock^.usbFiller[5]);

    { If OvrLoadCount has changed, the unit used }
    { by StubBlock has just been loaded.         }
    { The 4th filler word is used as a load      }
    {   counter.                                 }
    if (OvrLoadCount <> LoadCount) then
      begin
        LoadCount := OvrLoadCount;
        inc(StubBlock^.usbFiller[4]);
      end;

    { At this point, you may call another        }
    { UNOVERLAID routine, and pass StubBlock and }
    { RoutineOfs as parameters if needed.        }

  end;
Figure 3 - The OvrTracker procedure
The code is small and easily understandable. One warning though: do not put this procedure in an overlaid unit: the program will nose-dive at Mach 2. At the end of the procedure is a comment showing where you could place a call to another procedure, for example to dump the overlay buffer map to disk or printer. Again, to be pedantic, do not put this called procedure in an overlaid unit, and don't allow it to use any overlaid routines in its turn. Remember when this procedure is called, the overlay manager is still active and it definitely is not re-entrant.

The procedure in Figure 4 (OvrWalker) enables you to walk the unit stub block chains: both the full chain and the loaded units chain (the overlay buffer map), its action dependent on a passed boolean parameter. The former mode could be used in an exit procedure to dump the usage of the overlaid units at the end of the program (especially if you've installed OvrTracker as well). The latter mode could be called by OvrTracker itself, to dump the overlay buffer map to disk every time the overlay manager gets control. As an alternative, you could install it in your own error handler from hell, to be called when a run-time error occurs in the overlay buffer.

function RealAddr(Locale : word) : PUnitStubBlock; near;
  { RealAddr calculates the actual address of a  }
  { PUnitStubBlock from a locale value           }
  begin
    { A locale of zero = the end of the chain    }
    if (Locale = 0) then
         RealAddr := nil
    else RealAddr := Ptr(Locale + PrefixSeg + $10, 0);
  end;

procedure OvrWalker(FullList : boolean);
  { OvrWalker walks through the full unit stub   }
  { block list if FullList is true, otherwise it }
  { walks through the list of loaded stub blocks.}
  var
    StubBlock  : PUnitStubBlock;

  begin
    { Get the first stub block in the chain }
    if FullList then
         StubBlock := RealAddr(OvrCodeList)
    else StubBlock := RealAddr(OvrLoadList);

    { Walk through the chain ... }
    while (StubBlock <> nil) do
      with StubBlock^ do
        begin

          { ...Process this stub block... }

          { Get next stub block in the chain }
          if FullList then
               StubBlock := RealAddr(usbNextBlock)
          else StubBlock := RealAddr(usbNextLoad);
        end;
  end;
Figure 4 - The OvrWalker procedure
The only problem with OvrWalker and the unit stub blocks is that you've no real idea which unit they're talking about. Enter the MAP file. If you compile your program with the TPC compiler and use switch /GS, you'll create a MAP file showing all the segments in your program: code, data, stack, and heap. Each line in this MAP file shows the start and end segment values (locales in my parlance), the size in bytes, the name and class of each segment. If you walk through the unit stub blocks with OvrWalker, their locales can be matched to the start locales in the MAP file. The best thing to do would be to simply dump the unit stub blocks to a file from within your program, and then write an analysis program to operate on this file. By reading and parsing the MAP file first, the analysis program could easily attach meaningful unit names to each of the stub blocks. In Figure 5, I show the code for a unit (with no error checking for brevity's sake) that initialises the overlay manager, installs OvrTracker, and, on program termination, uses the MAP file to produce a simple usage dump of the overlaid units.

UNIT OvrUsage;

{$O-} { No overlays allowed here }

INTERFACE

uses Overlay;

IMPLEMENTATION

type
  PUnitStubBlock = ^TUnitStubBlock;
  TUnitStubBlock = record
    usbInt3F     : word;
    usbRetOfs    : word;
    usbFileOfs   : longint;
    usbCodeSize  : word;
    usbFixupSize : word;
    usbEntries   : word;
    usbNextBlock : word;
    usbBufferSeg : word;
    usbWasCalled : word;
    usbNextLoad  : word;
    usbFiller    : array [1..3] of word;
    usbOurLoads  : word; { 4th filler - redefined }
    usbOurTraps  : word; { 5th filler - redefined }
  end;

{ The saved previous exit procedure }
var
  ExitSave : pointer;

{ Procedure OvrTracker is the essentially the }
{ same as that in Figure 2; we don't want the }
{ routine offset this time however.           }
procedure OvrTracker; far;
  const
    LoadCount : word = 0;
  var
    StubBlock  : PUnitStubBlock;
  begin
    asm
      xor AX, AX
      mov StubBlock.Word[0], AX
      mov StubBlock.Word[2], ES
    end;
    inc(StubBlock^.usbOurTraps);
    if (OvrLoadCount <> LoadCount) then
      begin
        LoadCount := OvrLoadCount;
        inc(StubBlock^.usbOurLoads);
      end;
  end;

{ RealAddr calculates the actual address of a }
{ PUnitStubBlock from a locale value          }
function RealAddr(Locale : word) : PUnitStubBlock; near;
  begin
    { A locale of zero signifies the end of a chain }
    if (Locale = 0) then
         RealAddr := nil
    else RealAddr := Ptr(Locale + PrefixSeg + $10, 0);
  end;


{ Procedure ReportUsage is designed as an exit }
{ procedure. It reads the MAP file to get the  }
{ unit names, and then dumps the unit usage    }
{ statistics to file OVERLAY.DMP.              }
{ WARNING - contains NO error checking.        }
procedure ReportUsage; far;
  const
    MaxUnits = 100; { The max no of units we cater for }
  type
    UnitName = record { To store data for each unit }
      Locale : word;
      Name   : string[8];
    end;
  var
    i, ec       : integer;
    StubBlock   : PUnitStubBlock;
    StubLocale  : word;
    F           : text;
    MapFileName : string;
    Line        : string absolute MapFileName;
    NameArray   : array [1..MaxUnits] of UnitName;
    Finished    : boolean;
  begin
    ExitProc := ExitSave;

    { Assume we're running under MSDOS 3.0+, and }
    { the MAP file name is the same as the EXE   }
    { file name with a '.MAP' extension.         }
    MapFileName := ParamStr(0);
    MapFileName[0] := char(Pos('.', MapFileName) - 1);
    MapFileName := MapFileName + '.OVR';

    { Open the MAP file }
    Assign(F, MapFileName);
    Reset(F);

    { Miss the first three lines (the headings) }
    for i := 1 to 3 do readln(F, Line);

    { Read through the MAP file until we get to  }
    { the the line for the heap. For each line   }
    { read, get the start locale and unit name   }
    { and store in the local array NameArray.    }
    i := 0; Finished := false;
    repeat
      readln(F, Line);
      inc(i);
      with NameArray[i] do
        begin
          Line[1] := '$';
          Val(Copy(Line, 1, 5), Locale, ec);
          Name := Copy(Line, 23, 8);
          if (Name = 'HEAP    ') then Finished := true;
        end;
    until Finished;
    Close(F);

    { Open the statistics file OVERLAY.DMP, }
    { output the headings.                  }
    Assign(F, 'OVERLAY.DMP');
    Rewrite(F);
    writeln(F, 'Overlay usage for ', ParamStr(0));
    writeln(F);
    writeln(F, 'Name----  -Size  Traps  Loads');

    { Now walk the unit stub block chain }
    StubBlock := RealAddr(OvrCodeList);
    StubLocale := OvrCodeList;
    while (StubBlock <> nil) do
      with StubBlock^ do
        begin
          { Find the unit name }
          i := 0;
          while (NameArray[i].Locale <> StubLocale) do inc(i);

          { Write details to the Usage file }
          writeln(F, NameArray[i].Name,
                     usbCodeSize:7,
                     usbOurTraps:7,
                     usbOurLoads:7);

          { Get next stub block in the chain }
          StubBlock := RealAddr(usbNextBlock);
          StubLocale := usbNextBlock;
        end;
    Close(F);
  end;

var
  OvrFileName : string;

begin
  { Assume we're running under MSDOS 3.0+, and   }
  { the overlay file name is the same as the EXE }
  { file name with a '.OVR' extension.           }
  OvrFileName := ParamStr(0);
  OvrFileName[0] := char(Pos('.', OvrFileName) - 1);
  OvrFileName := OvrFileName + '.OVR';

  { Initialise the overlay manager, with a buffer}
  { twice the size of the original. Set the      }
  { probation area.                              }
  OvrInit(OvrFileName);
  OvrSetBuf(OvrGetBuf * 2);
  OvrSetRetry(OvrGetBuf div 3);

  { Install OvrTracker. }
  OvrDebugPtr := @OvrTracker;

  { Install the ReportUsage routine as an exit }
  { procedure.                                 }
  ExitSave := ExitProc;
  ExitProc := @ReportUsage;
end.
Figure 5 - An example Overlay Usage unit
Conclusion
As I have shown, the Turbo Pascal overlay manager is extremely sophisticated, both from an external viewpoint and also from watching its internal machinations.

From the undocumented features I have laid bare, you can now find out how best to use overlaid units, which ones to overlay, and conversely, which ones not to. You can find out by observing your own program whether the recommended probation size (one third of the overlay buffer) is the best for you. You can decide whether to split up large overlaid units or not. If you're feeling adventurous, you could investigate how to move the overlay buffer, how to shrink it, and (harder) how to grow it dynamically, as your program relaxes and intensifies its demands on the heap. Perhaps, like me, you could write your own unit to load overlaid units from XMS rather than EMS, disable the 64KB EMS page frame, and reuse the memory for something else (the overlay buffer?).

Julian Bucknall has been designing and programming for 12 years, and full time in Turbo Pascal for the last three. His other favourite language used to be RPG III, so there's no hope. Off work, he can be found at the wheel of his red Volvo 1800S in and around London, a Saintly vision.

The quote is from the poem Jabberwocky, in Alice Through the Looking Glass by Lewis Carroll.

Turbo Pascal V6.0 Professional is a product from Borland International (0734 321150), and is available at £137 from Grey Matter (0364 53499).

Copyright © 2000-2026 Julian M Bucknall
