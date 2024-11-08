
#include <stdint.h>

#include "MoX_BASE.H"
// #include "..\ReMoM\src\MoX\MoX_BASE.H"

// #include "Mouse.H"
#include "..\ReMoM\src\Mouse.H"



// win_MD.C
// int16_t platform_mouse_button_status;
// int16_t lock_mouse_button_status_flag = ST_FALSE;



// Mouse.C
// WZD dseg:78BC
extern int16_t mouse_enabled;
// WZD dseg:78C2
extern int16_t mouse_x;
// WZD dseg:78C4
extern int16_t mouse_y;
// WZD dseg:78C6
extern int16_t current_mouse_list_count;
// WZD dseg:78D4
extern int16_t mouse_interrupt_active;

// WZD s35p02
void Check_Mouse_Shape(int16_t x, int16_t y);
// WZD s35p29
void Check_Mouse_Buffer(int16_t x, int16_t y, int16_t buttons);

// WZD s35p30
void Save_Mouse_On_Page(int16_t x, int16_t y);
// WZD s35p31
void Save_Mouse_Off_Page(int16_t x, int16_t y);

// WZD s35p33
void Copy_Mouse_Off_To_Mouse_Back(void);

// WZD s35p34
void Restore_Mouse_On_Page(void);
// WZD s35p35
void Restore_Mouse_Off_Page(void);

// WZD s35p37
void Draw_Mouse_On_Page(int16_t x, int16_t y);
// WZD s35p38
void Draw_Mouse_Off_Page(int16_t x, int16_t y);




int16_t platform_mouse_button_status;


int16_t lock_mouse_button_status_flag = ST_FALSE;

// WZD s35p05
int16_t Mouse_Button(void)
{

    // int16_t mouse_button_status;
    // mouse_button_status = 0b00000000;
    // if(mouse_driver_installed != ST_FALSE)
    // {
    //     mouse_button_status = mouse_right_button + mouse_left_button;
    // }

    // Pump_Events();

    return platform_mouse_button_status;
}


// WZD s35p10
void Mouse_Movement_Handler(void)
{
    if (mouse_driver_installed != ST_FALSE)
    {
        // set User_Mouse_Handler() with flags for just mouse movement interrupts
        // _AX_ = ST_SUCCESS
        mouse_enabled = ST_TRUE;
        // INT 33,3
        // ~== User_Mouse_Handler() with current mouse buttons, x, y
    }
    lock_mouse_button_status_flag = ST_TRUE;
}

// WZD s35p11
void Mouse_Button_Handler(void)
{
    // YNM  platform_mouse_click_x = 0;
    // YNM  platform_mouse_click_y = 0;
    platform_mouse_button_status = 0;
    lock_mouse_button_status_flag = ST_FALSE;
}



// WZD s35p11
void User_Mouse_Handler(int16_t buttons, int16_t mx, int16_t my)
{
    mouse_x = mx;
    mouse_y = my;
    platform_mouse_button_status = buttons;

    if (mouse_interrupt_active == ST_FALSE)
    {
        mouse_interrupt_active = ST_TRUE;

        Check_Mouse_Buffer(mx, my, buttons);

        if (mouse_enabled == ST_TRUE)
        {
            mouse_enabled = ST_FALSE;
            if (current_mouse_list_count >= 2)
            {
                Check_Mouse_Shape(mouse_x, mouse_y);
            }
            // Restore_Mouse_On_Page();                // mouse_background_buffer -> video_page_buffer[draw_page_num]
            // Save_Mouse_On_Page(mouse_x, mouse_y);   // video_page_buffer[draw_page_num] -> mouse_background_buffer
            // Draw_Mouse_On_Page(mouse_x, mouse_y);   // mouse_palette -> video_page_buffer[draw_page_num]
            Restore_Mouse_Off_Page();
            Save_Mouse_Off_Page(mouse_x, mouse_y);
            Copy_Mouse_Off_To_Mouse_Back();
            Draw_Mouse_Off_Page(mouse_x, mouse_y);

            mouse_enabled = ST_TRUE;
        }
        mouse_interrupt_active = ST_FALSE;
    }

}
