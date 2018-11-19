#ifndef XPDF_VIDEO_H
#define XPDF_VIDEO_H

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

// resolución
#ifdef WIN32_BIG
# define SCR_WIDTH  640
# define SCR_HEIGHT 480
#else
# define SCR_WIDTH  320
# define SCR_HEIGHT 240
#endif

// convierte rgb an 0xrrggbbaa
#define RGBA(r,g,b) ((r)<<24 | (g)<<16 | (b)<<8 | 0xff)
#define RGBAT(r,g,b,a) ((r)<<24 | (g)<<16 | (b)<<8 | a)

// para la fuente 8x8 de gfx
#define FNT_SIDE 8
#define SCR_COLS (SCR_WIDTH >> 3)  // 40
#define SCR_ROWS (SCR_HEIGHT >> 3) // 30

extern SDL_Surface *screen;

#define KEY_NONE -1

#if defined(GP2X) || defined(WIZ) || defined(WIN32)
// teclas de la gp2x y wiz
enum TGp2xKeys
{
	KEY_UP = 0,     KEY_LEFT = 2,
  KEY_DOWN = 4,   KEY_RIGHT = 6,
	KEY_MENU = 8,	  KEY_SELECT = 9,
#ifdef WIZ
	KEY_FL = 10,	  KEY_FR = 11, // al revés
#else
	KEY_FL = 11,	  KEY_FR = 10,
#endif
  KEY_FA = 12,	  KEY_FB = 13,
#ifdef WIZ
	KEY_FY = 15,	  KEY_FX = 14,
#else
	KEY_FY = 14,	  KEY_FX = 15,
#endif
	KEY_VOLUP = 16, KEY_VOLDN = 17
};
#elif defined(CAANOO)
// teclas cannoo, no hay vol+-
enum TCaanooKeys
{
  KEY_FA = 0,     KEY_FX = 1,
  KEY_FB = 2,     KEY_FY = 3,
  KEY_FL = 4,     KEY_FR = 5,
  KEY_HOME = 6,   // caanoo home
  KEY_HELP1 = 8,  KEY_HELP2 = 9,
  KEY_TACT = 10,  // stick
  KEY_UP = 11,    KEY_DOWN = 12,
  KEY_LEFT = 13,  KEY_RIGHT = 14,
  // simular menu con h1 y sel con home
  KEY_MENU = KEY_HELP2,
  KEY_SELECT = KEY_HOME,
  // simular vol+ y vol- con h1,h2
	KEY_VOLUP = KEY_HELP1,
	KEY_VOLDN = KEY_HELP2
};
#elif defined(GCW0)
enum TGcwzKeys
{
  // teclas virtuales
	KEY_UP = 0,     KEY_LEFT = 2,
  KEY_DOWN = 4,   KEY_RIGHT = 6,
	KEY_MENU = 8,	  KEY_SELECT = 9,
	KEY_FL = 11,	  KEY_FR = 10,
  KEY_FA = 12,	  KEY_FB = 13,
	KEY_FY = 14,	  KEY_FX = 15,
	KEY_LOCK = 17, // powdown
	// simular vol+/- con analógico
	KEY_VOLUP = 18, KEY_VOLDN = 19
};
#endif

// colores estándar
#define COL_BLACK   RGBA(0,0,0)
#define COL_RED     RGBA(128,0,0)
#define COL_GREEN   RGBA(0,128,0)
#define COL_BLUE    RGBA(0,0,128)
#define COL_LRED    RGBA(255,0,0)
#define COL_LGREEN  RGBA(0,255,0)
#define COL_LBLUE   RGBA(0,0,255)
#define COL_BROWN   RGBA(128,128,0)
#define COL_YELLOW  RGBA(255,255,0)
#define COL_PURPLE  RGBA(128,0,128)
#define COL_PINK    RGBA(255,0,255)
#define COL_CYAN    RGBA(0,128,128)
#define COL_LCYAN   RGBA(0,255,255)
#define COL_GRAY    RGBA(128,128,128)
#define COL_LGRAY   RGBA(196,196,196)
#define COL_WHITE   RGBA(255,255,255)

bool VideoInit();
void VideoDone();
int ReadKey();
int WaitKey();
void WaitNoKey();
void ClearScreen();
void TextColor(int rgba);
void WriteString(int f, int c, const char *msg);
void WriteCenter(int f, const char *msg);
void WriteFormat(int f, int c, const char *format, ...);
void GetMemoryStats(long *ktotal, long *kfree);

#endif // XPDF_VIDEO_H
