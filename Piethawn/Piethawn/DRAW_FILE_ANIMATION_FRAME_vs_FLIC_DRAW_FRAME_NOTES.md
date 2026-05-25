


**Sources**
- [`in/WIZARDS.lst#L33120`](/mnt/c/STU/devel/STU-Extras/Piethawn/Piethawn/in/WIZARDS.lst#L33120)
- [`in/WIZARDS.lst#L35947`](/mnt/c/STU/devel/STU-Extras/Piethawn/Piethawn/in/WIZARDS.lst#L35947)
- [`in/WIZARDS.asm`](/mnt/c/STU/devel/STU-Extras/Piethawn/Piethawn/in/WIZARDS.asm)

**Compare `Draw_File_Animation_Frame__WIP` and `FLIC_Draw_Frame`**

**Similarities**
- Both decode the same column-oriented FLIC frame packet format.
- Both draw into planar VGA video memory using the VGA sequencer write-map register (`0x3C4/0x3C5`).
- Both compute the same initial screen destination from `x`/`y` and `current_video_page`.
- Both use the same per-plane mask stepping logic based on `VGA_WriteMapMasks*`.
- Both have the same two major decode paths:
  - copy packets when the first control byte is non-negative
  - run-length packets when the first control byte is negative
- Both advance vertically by `+79` after each byte write, which is the same “next scanline in current plane” behavior.
- Both iterate by columns, not by rows.
- Both stop a column when they hit `0xFF` or exhaust the packet count for that column.

**Differences**
- `Draw_File_Animation_Frame__WIP` is EMS-aware; `FLIC_Draw_Frame` is not.
- `Draw_File_Animation_Frame__WIP` pulls its frame stream from EMS-mapped pages using FLIC header EMS metadata.
- `FLIC_Draw_Frame` takes a direct far pointer to frame data via `frame_data_ofst` and `frame_data_sgmt`.
- `Draw_File_Animation_Frame__WIP` reads frame-offset-table data from the animation header to determine the logical EMS page and offset for the selected frame.
- `Draw_File_Animation_Frame__WIP` explicitly maps EMS pages with `int 67h` and calls `VGA_MapNextEMMPage` when the read cursor crosses near the 16 KB page boundary.
- `Draw_File_Animation_Frame__WIP` has a remap path: if the byte at `17 + EMM_Logical_Page_Offset` is nonzero, it delegates to `Remap_Draw_File_Animation_Frame__NOP`.
- `FLIC_Draw_Frame` has no EMS page-boundary or remap helper calls inside the routine.
- `Draw_File_Animation_Frame__WIP` stores scratch state in seg025 globals:
  - `CS_file_animation_width`
  - `CS_file_animation_emm_logical_page`
  - `CS_file_animation_emm_handle`
- `FLIC_Draw_Frame` keeps all state local to the function call.

**Interpretation**
- `FLIC_Draw_Frame` is the generic direct-memory FLIC frame renderer.
- `Draw_File_Animation_Frame__WIP` is the file-animation-specific renderer that adapts EMS-backed frame storage into that same rendering model.

**C89 Translation Notes**
- These are near-literal translations, not cleaned-up ports.
- DOS/VGA helpers like `outp`, `MK_FP`, `pokeb`, `peekb`, and EMS helpers are left in DOS-style form on purpose.
- Struct-offset names below are symbolic stand-ins for the disassembly offsets.

**Near-Literal C89 Translation: `Draw_File_Animation_Frame__WIP`**

```c
static unsigned int CS_file_animation_width;
static unsigned int CS_file_animation_emm_logical_page;
static unsigned int CS_file_animation_emm_handle;

void Draw_File_Animation_Frame__WIP(int x, int y, unsigned int file_animation, int frame_num)
{
    unsigned int emm_handle;
    unsigned int emm_logical_page_number;
    unsigned int emm_logical_page_offset;
    unsigned int frame_width;
    unsigned int column_count;
    unsigned int screen_pos;
    unsigned int current_logical_page;
    unsigned int map_mask;
    unsigned int ds_seg;
    unsigned int es_seg;
    unsigned int bx;
    unsigned int dx;
    unsigned int ax;
    unsigned char remap_flag;
    unsigned char far * emm_ptr;

    ds_seg = file_animation;
    es_seg = file_animation;

    frame_width = farpeekw(ds_seg, s_FLIC_HDR_width);
    CS_file_animation_width = frame_width;

    emm_handle = farpeekb(ds_seg, s_FLIC_HDR_emm_handle_number);
    emm_logical_page_number = farpeekb(ds_seg, s_FLIC_HDR_EMM_Logical_Page_Number);
    emm_logical_page_offset = farpeekw(ds_seg, s_FLIC_HDR_EMM_Logical_Page_Offset);

    EMS_MapPage(emm_handle, emm_logical_page_number, 0);
    EMS_MapPage(emm_handle, emm_logical_page_number + 1, 1);

    ax = (unsigned int)frame_num;
    emm_logical_page_offset += (unsigned int)(18 + (ax * 4));

    emm_ptr = (unsigned char far *)MK_FP(EMS_PFBA, emm_logical_page_offset);
    dx = *(unsigned int far *)emm_ptr;
    ax = *((unsigned int far *)emm_ptr + 1);

    dx++;
    ax += (dx >> 14);
    dx = (unsigned int)((dx << 2) >> 2);

    ax += farpeekb(es_seg, s_FLIC_HDR_EMM_Logical_Page_Number);
    dx += farpeekw(es_seg, s_FLIC_HDR_EMM_Logical_Page_Offset);
    if (dx >= 16384U)
    {
        dx &= 0x3FFFU;
        ax++;
    }

    CS_file_animation_emm_logical_page = ax;

    remap_flag = peekb(es_seg, emm_logical_page_offset - 1);

    column_count = frame_width;
    screen_pos = dx;
    current_logical_page = ax;
    CS_file_animation_emm_handle = farpeekb(es_seg, s_FLIC_HDR_emm_handle_number);

    EMS_MapPage(CS_file_animation_emm_handle, current_logical_page, 0);
    EMS_MapPage(CS_file_animation_emm_handle, current_logical_page + 1, 1);

    if (remap_flag != 0)
    {
        Remap_Draw_File_Animation_Frame__NOP(x, y, file_animation);
        return;
    }

    outp(0x3C4, 2);

    ax = (unsigned int)y;
    bx = ax;
    ax <<= 1;
    ax <<= 1;
    ax += bx;
    es_seg = (unsigned int)(ax + current_video_page);

    ax = (unsigned int)x;
    bx = ax;
    bx >>= 1;
    bx >>= 1;
    screen_pos = bx;

    ax &= 3;
    map_mask = VGA_WriteMapMasks2[ax];

    ds_seg = EMS_PFBA;
    bx = CS_file_animation_width;

    for (;;)
    {
        unsigned int saved_screen_pos;
        unsigned int packet_len;
        unsigned char c1;
        unsigned char c2;

        saved_screen_pos = screen_pos;

        if (screen_pos >= 31744U)
        {
            VGA_MapNextEMMPage();
        }

        outp(0x3C5, map_mask);

        c1 = peekb(ds_seg, screen_pos);
        screen_pos++;

        if (c1 == 0xFF)
        {
            screen_pos = saved_screen_pos;
            bx--;
            if (bx == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9)
            {
                map_mask = 1;
                screen_pos++;
            }
            continue;
        }

        c2 = peekb(ds_seg, screen_pos);
        screen_pos++;
        dx = ((unsigned int)c1 << 8) | c2;

        if ((signed char)c1 >= 0)
        {
            do
            {
                unsigned int skip_count;
                unsigned int copy_count;

                skip_count = peekb(ds_seg, screen_pos + 1);
                screen_pos += (skip_count * 80U);

                copy_count = peekb(ds_seg, screen_pos);
                screen_pos++;
                screen_pos++;
                dx -= 2;
                dx -= copy_count;

                while (copy_count != 0)
                {
                    pokeb(es_seg, saved_screen_pos + (screen_pos - saved_screen_pos),
                          peekb(ds_seg, screen_pos));
                    screen_pos++;
                    saved_screen_pos += 79U;
                    copy_count--;
                }
            } while ((int)dx >= 1);

            screen_pos = saved_screen_pos;
            bx--;
            if (bx == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9)
            {
                map_mask = 1;
                screen_pos++;
            }
        }
        else
        {
            dx &= 0x7FFFU;

            do
            {
                unsigned int skip_count;
                unsigned int packet_count;

                skip_count = peekb(ds_seg, screen_pos + 1);
                screen_pos += (skip_count * 80U);

                packet_count = peekb(ds_seg, screen_pos);
                screen_pos++;
                screen_pos++;
                dx -= 2;
                dx -= packet_count;

                while (packet_count != 0)
                {
                    unsigned char value;

                    value = peekb(ds_seg, screen_pos);
                    screen_pos++;

                    if (value < 224)
                    {
                        pokeb(es_seg, saved_screen_pos + (screen_pos - saved_screen_pos - 1), value);
                        saved_screen_pos += 79U;
                    }
                    else
                    {
                        unsigned int repeat_count;
                        unsigned char repeat_value;

                        repeat_count = (unsigned int)((value & 0x1F) + 1);
                        repeat_value = peekb(ds_seg, screen_pos);
                        screen_pos++;

                        while (repeat_count != 0)
                        {
                            pokeb(es_seg, saved_screen_pos, repeat_value);
                            saved_screen_pos += 79U;
                            repeat_count--;
                        }

                        packet_count--;
                    }

                    packet_count--;
                }
            } while ((int)dx >= 1);

            screen_pos = saved_screen_pos;
            bx--;
            if (bx == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9)
            {
                map_mask = 1;
                screen_pos++;
            }
        }
    }
}
```

**Near-Literal C89 Translation: `FLIC_Draw_Frame`**

```c
void FLIC_Draw_Frame(int x_start,
                     int y_start,
                     int frame_width,
                     unsigned int frame_data_ofst,
                     unsigned int frame_data_sgmt)
{
    unsigned int bx_itr;
    unsigned int screen_pos;
    unsigned int screen_start;
    unsigned int map_mask;
    unsigned char far * frame_data;

    outp(0x3C4, 2);

    bx_itr = (unsigned int)y_start;
    screen_start = (unsigned int)(current_video_page + (y_start * 5));

    bx_itr = (unsigned int)x_start;
    screen_pos = (unsigned int)(bx_itr >> 2);

    bx_itr &= 3U;
    map_mask = VGA_WriteMapMasks4[bx_itr];

    frame_data = (unsigned char far *)MK_FP(frame_data_sgmt, frame_data_ofst);
    bx_itr = (unsigned int)frame_width;

    for (;;)
    {
        unsigned int saved_screen_pos;
        unsigned int dx;
        unsigned char c1;
        unsigned char c2;

        saved_screen_pos = screen_pos;

        outp(0x3C5, map_mask);

        c1 = *frame_data++;
        if (c1 == 0xFF)
        {
            screen_pos = saved_screen_pos;
            bx_itr--;
            if (bx_itr == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9U)
            {
                map_mask = 1;
                screen_pos++;
            }
            continue;
        }

        c2 = *frame_data++;
        dx = ((unsigned int)c1 << 8) | c2;

        if ((signed char)c1 >= 0)
        {
            do
            {
                unsigned int skip_count;
                unsigned int copy_count;

                skip_count = frame_data[1];
                screen_pos += skip_count * 80U;

                copy_count = *frame_data++;
                frame_data++;
                dx -= 2U;
                dx -= copy_count;

                while (copy_count != 0)
                {
                    pokeb(screen_start, screen_pos, *frame_data++);
                    screen_pos += 79U;
                    copy_count--;
                }
            } while ((int)dx >= 1);

            screen_pos = saved_screen_pos;
            bx_itr--;
            if (bx_itr == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9U)
            {
                map_mask = 1;
                screen_pos++;
            }
        }
        else
        {
            dx &= 0x7FFFU;

            do
            {
                unsigned int skip_count;
                unsigned int packet_count;

                skip_count = frame_data[1];
                screen_pos += skip_count * 80U;

                packet_count = *frame_data++;
                frame_data++;
                dx -= 2U;
                dx -= packet_count;

                while (packet_count != 0)
                {
                    unsigned char value;

                    value = *frame_data++;
                    if (value < 224)
                    {
                        pokeb(screen_start, screen_pos, value);
                        screen_pos += 79U;
                    }
                    else
                    {
                        unsigned int repeat_count;
                        unsigned char repeat_value;

                        repeat_count = (unsigned int)((value & 0x1F) + 1);
                        repeat_value = *frame_data++;

                        while (repeat_count != 0)
                        {
                            pokeb(screen_start, screen_pos, repeat_value);
                            screen_pos += 79U;
                            repeat_count--;
                        }

                        packet_count--;
                    }

                    packet_count--;
                }
            } while ((int)dx >= 1);

            screen_pos = saved_screen_pos;
            bx_itr--;
            if (bx_itr == 0)
            {
                break;
            }
            map_mask <<= 1;
            if (map_mask >= 9U)
            {
                map_mask = 1;
                screen_pos++;
            }
        }
    }
}
```

**seg025 Summary**
- `seg025` is the file-animation-frame draw segment.
- Functions present:
  - `Draw_File_Animation_Frame__WIP`
  - `VGA_MapNextEMMPage`
  - `Remap_Draw_File_Animation_Frame__NOP`
- The segment’s purpose is EMS-backed frame rendering for file animations.
- It bridges FLIC frame decoding with EMS page management.
- The key added behavior over the generic renderer is:
  - interpreting FLIC EMS metadata from the animation header
  - mapping the right EMS pages
  - advancing to the next EMS page while drawing when the stream crosses the page boundary
  - handling the remap/non-remap split for file-animation frames

**seg029 Summary**
- `seg029` is the core FLIC frame renderer segment.
- Functions present:
  - `FLIC_Draw_Frame`
  - `FLIC_Remap_Draw_Frame`
  - `FLIC_Draw_Frame_EMM`
  - `VGA_MapNextEMMPages`
  - `FLIC_Draw_Frame_EMM_Remap`
- This segment contains the generic decode/draw engines.
- It covers both direct-memory and EMS-backed frame rendering.
- `FLIC_Draw_Frame` is the simplest direct renderer.
- The other routines in the segment extend that same base logic to remap-aware and EMS-aware cases.
