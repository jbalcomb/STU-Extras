/*

Handle Naive Upscaling

Prepare Video Back Buffer for Platform

*/

#include <malloc.h>



typedef unsigned char uint8_t;
typedef   signed char int8_t;
typedef unsigned short int uint16_t;
typedef   signed short int int16_t;

typedef unsigned char * SAMB_ptr;
typedef unsigned char * byte_ptr;

// #define DEFAULT_FONT_FILE   "FONTS.LBX"
char DEFAULT_FONT_FILE[] = "FONTS.LBX";

extern "C" int16_t draw_page_num;
extern "C" uint8_t * current_video_page;
extern "C" uint8_t * video_page_buffer[];
extern "C" uint8_t * draw_page;
extern "C" int screen_pixel_width;
extern "C" int screen_pixel_height;
extern "C" int screen_pixel_size;

extern "C" byte_ptr p_Palette;
extern "C" uint8_t PFL_Palette[1024];

extern "C" void Load_Font_File(char* font_file);
extern "C" void Load_Palette(int entry, int start_color, int end_color);
extern "C" void Apply_Palette(void);

char main_lbx_file[] = "MAIN";
SAMB_ptr main_background;
extern "C" SAMB_ptr LBX_Load(char* lbx_name, int16_t entry_num);
extern "C" void FLIC_Draw(int16_t x_start, int16_t y_start, SAMB_ptr picture);  // dependencies: Fonts, Graphics, Palette, Video

// Fields.C
// WZD s36p57
extern "C" void Clear_Fields(void);

// Mouse.C
// WZD dseg:78C2
extern "C" int16_t mouse_x;
// WZD dseg:78C4
extern "C" int16_t mouse_y;

extern "C" struct s_mouse_list mouse_list_default[1];
// WZD s35p01
extern "C" void Set_Mouse_List(int16_t count, struct s_mouse_list* list);
// WZD s35p10
extern "C" void Mouse_Movement_Handler(void);
// WZD s35p31
extern "C" void Save_Mouse_Off_Page(int16_t x, int16_t y);
// WZD s35p33
extern "C" void Copy_Mouse_Off_To_Mouse_Back(void);
// WZD s35p37
extern "C" void Draw_Mouse_On_Page(int16_t x, int16_t y);
// WZD s35p38
extern "C" void Draw_Mouse_Off_Page(int16_t x, int16_t y);
// Input.C
// WZD s36p65
extern "C" void Init_Mouse_Keyboard(int16_t input_type);
// MoX_RNG.C
// WZD s22p07
extern "C" void Randomize(void);
// Video.C
// WZD s28p02
extern "C" void Set_Page_Off(void);



int current_scale = 1;
int previous_scale = 1;
int * mox_vbb_scaled = nullptr;




// ~== MoX   Video  Init_Video_Drivers()
// ~== 1oom  hwsdl2_video  hw_video_init()
/*
	set video mode
	set coordinates and dimensions
	allocate video back buffers
	set draw page
*/
void Init_Video(void)
{

	// Video2.C
	screen_pixel_width = 320;
	screen_pixel_height = 200;
	screen_pixel_size = screen_pixel_width * screen_pixel_height;
	
	video_page_buffer[0] = (uint8_t*)malloc(screen_pixel_size);  // 320 * 200 * 1Bpp = 64000 / 16 = 4000 PR
	video_page_buffer[1] = (uint8_t*)malloc(screen_pixel_size);
	video_page_buffer[2] = (uint8_t*)malloc(screen_pixel_size);
	draw_page_num = 0;
	draw_page = video_page_buffer[draw_page_num];
	current_video_page = 0;
	current_video_page = video_page_buffer[1 - draw_page_num];

}

void Init_MoX(void)
{
	// ~== Init_Drivers()
	// s06p05_Empty_pFxn();
	// EMM_Startup();
	// Set_Video_Mode();
	Init_Video();
	Load_Font_File(DEFAULT_FONT_FILE);
	// Audio_Init();
	Init_Mouse_Keyboard(1);
	Mouse_Movement_Handler();
	Randomize();
	Set_Page_Off();

	// ~== Screen_Control()
	Clear_Fields();
	Set_Mouse_List(1, mouse_list_default);
	Save_Mouse_Off_Page(mouse_x, mouse_y);
	Copy_Mouse_Off_To_Mouse_Back();
	
	Load_Palette(0, -1, 0);
	Apply_Palette();

	main_background = LBX_Load(main_lbx_file, 0);
	FLIC_Draw(0, 0, main_background);

	// Draw_Mouse_On_Page(158, 100);
	Draw_Mouse_Off_Page(158, 100);

}

void STU_MoX_Upscale(void)
{
	// copy each n times, where n is naive scale factor
	// 1x
	//		Pi
	// 2x
	//		Pi Pi
	//		Pi Pi
	// 3x
	//		Pi Pi Pi
	//		Pi Pi Pi
	//		Pi Pi Pi
	// 4x
	//		Pi Pi Pi Pi
	//		Pi Pi Pi Pi
	//		Pi Pi Pi Pi
	//		Pi Pi Pi Pi
	// etc.
	unsigned char pixel;
	unsigned char * src;
	unsigned char * dst;
	int itr;

	if(
		(current_scale != previous_scale) ||
		(mox_vbb_scaled == nullptr)
		)
	{
		if (mox_vbb_scaled != nullptr)
		{
			free(mox_vbb_scaled);
		}
		mox_vbb_scaled = (int*)malloc(screen_pixel_width * screen_pixel_height * current_scale * sizeof(*mox_vbb_scaled));
		if (mox_vbb_scaled == nullptr)
		{
			__debugbreak();
		}
	}

	// copy color-map from 6-bpp RGB palette (VGA) to 32-bpp XBGR palette (MS BMP)
	for (itr = 0; itr < 256; itr++)
	{
		if (*(p_Palette + 768 + itr) == 1)
		{
			*(PFL_Palette + (itr * 4) + 3) = 0x00;
			*(PFL_Palette + (itr * 4) + 2) = (*(p_Palette + (itr * 3) + 0) << 2);
			*(PFL_Palette + (itr * 4) + 1) = (*(p_Palette + (itr * 3) + 1) << 2);
			*(PFL_Palette + (itr * 4) + 0) = (*(p_Palette + (itr * 3) + 2) << 2);
		}
	}


	src = video_page_buffer[draw_page_num];


}

void STU_MoX_Prepare_Video_Back_Buffer(void)
{

}