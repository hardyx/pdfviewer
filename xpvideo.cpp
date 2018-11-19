///////////////////////////////////////////////////////////////////////////////
// XPVideo, funciones de vídeo y gráficos
// la sdl de ikari soporta 24bpp, la oficial revienta.
// la fuente interna de sdl_gfx es de 8x8 (40x30 caracteres)
//

#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include "xpvideo.h"

SDL_Surface *screen = NULL;
static SDL_Joystick *m_joy = NULL;
static int m_txcolor = 0xffffffff; // en RGBA

///////////////////////////////////////////////////////////////

bool VideoInit()
{
  // Initialize sdl subsystems
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1) {
    fprintf(stderr, "Error inicializando SDL\n");
    return false;
  }
  atexit(VideoDone);

#ifndef WIN32
  SDL_ShowCursor(SDL_DISABLE);
#endif

  int isok = SDL_VideoModeOK(SCR_WIDTH, SCR_HEIGHT, 16, SDL_SWSURFACE);
  fprintf(stderr, "Modo de vídeo asignado: %d bpp\n", isok);

  // create the screen surface, 24 bits rgb (ikari)
  screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 16, SDL_SWSURFACE);
  if (!screen) {
    fprintf(stderr, "Error creando modo de video\n");
    return false;
  }

#ifndef GPHMODE // win32, gcw0
  SDL_EnableKeyRepeat(500, 30);
#endif

	// check and open joystick device
	if (SDL_NumJoysticks() > 0) {
		m_joy = SDL_JoystickOpen(0);
		if(!m_joy) {
			fprintf(stderr, "No puedo abrir el joystick: %s\n", SDL_GetError());
      return false;
		}
	}
  return true;
}

void VideoDone()
{
  printf("VideoDone\n");
  // cerrar el joystick
  if(SDL_JoystickOpened(0))
     SDL_JoystickClose(m_joy);
  SDL_Quit();

#ifdef GPHMODE
  chdir("/usr/gp2x");
  execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#endif
}

// lee tecla con repetición sin bloqueo
int ReadKey()
{
	int ret = KEY_NONE;
#ifdef GPHMODE
	if(m_joy) {
		SDL_JoystickUpdate();

    // solucionado analógico caanoo
#ifdef CAANOO
    if(SDL_JoystickGetAxis(m_joy,1) < -16384)
      ret = KEY_UP;
    else if(SDL_JoystickGetAxis(m_joy,1) > 16384)
      ret = KEY_DOWN;
    else if(SDL_JoystickGetAxis(m_joy,0) < -16384)
      ret = KEY_LEFT;
    else if(SDL_JoystickGetAxis(m_joy,0) > 16384)
      ret = KEY_RIGHT;
#else // gp2x/wiz
		if(SDL_JoystickGetButton(m_joy,KEY_LEFT))
			ret = KEY_LEFT;
		else if(SDL_JoystickGetButton(m_joy,KEY_RIGHT))
			ret = KEY_RIGHT;
		else if(SDL_JoystickGetButton(m_joy,KEY_UP))
			ret = KEY_UP;
		else if(SDL_JoystickGetButton(m_joy,KEY_DOWN))
			ret = KEY_DOWN;
#endif

		else if(SDL_JoystickGetButton(m_joy,KEY_FL))
			ret = KEY_FL;
		else if(SDL_JoystickGetButton(m_joy,KEY_FR))
			ret = KEY_FR;
		else if(SDL_JoystickGetButton(m_joy,KEY_FX))
			ret = KEY_FX;
		else if(SDL_JoystickGetButton(m_joy,KEY_FY))
			ret = KEY_FY;
		else if(SDL_JoystickGetButton(m_joy,KEY_FA))
			ret = KEY_FA;
		else if(SDL_JoystickGetButton(m_joy,KEY_FB))
			ret = KEY_FB;
		else if(SDL_JoystickGetButton(m_joy,KEY_VOLUP))
			ret = KEY_VOLUP;
		else if(SDL_JoystickGetButton(m_joy,KEY_VOLDN))
			ret = KEY_VOLDN;
		else if(SDL_JoystickGetButton(m_joy,KEY_MENU))
			ret = KEY_MENU;
		else if(SDL_JoystickGetButton(m_joy,KEY_SELECT))
			ret = KEY_SELECT;
	}
#else
# ifdef GCW0
	if(m_joy) {
	  // hacer vol+/- con el analógico
		SDL_JoystickUpdate();
    if(SDL_JoystickGetAxis(m_joy,1) < -16384)
      ret = KEY_VOLUP;
    else if(SDL_JoystickGetAxis(m_joy,1) > 16384)
      ret = KEY_VOLDN;
  }
# endif

  // events en win32 o gcw
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT:
        exit(0); break;
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym) {
#ifdef WIN32
          case SDLK_ESCAPE: exit(0); break;
          case SDLK_UP:    ret = KEY_UP; break;
          case SDLK_DOWN:  ret = KEY_DOWN; break;
          case SDLK_LEFT:  ret = KEY_LEFT; break;
          case SDLK_RIGHT: ret = KEY_RIGHT; break;
          case SDLK_a: ret = KEY_FA; break;
          case SDLK_b: ret = KEY_FB; break;
          case SDLK_y: ret = KEY_FY; break;
          case SDLK_x: ret = KEY_FX; break;
          case SDLK_l: ret = KEY_FL; break;
          case SDLK_r: ret = KEY_FR; break;
          case SDLK_KP_MINUS: ret = KEY_VOLDN; break;
          case SDLK_KP_PLUS: ret = KEY_VOLUP; break;
          case SDLK_e: ret = KEY_MENU; break;
          case SDLK_s: ret = KEY_SELECT; break;
#else // GCW0
          case SDLK_UP:    ret = KEY_UP; break;
          case SDLK_DOWN:  ret = KEY_DOWN; break;
          case SDLK_LEFT:  ret = KEY_LEFT; break;
          case SDLK_RIGHT: ret = KEY_RIGHT; break;
          // A y B están cambiadas fisicamente
          case SDLK_LCTRL: ret = KEY_FB; break;
          case SDLK_LALT: ret = KEY_FA; break;
          case SDLK_LSHIFT: ret = KEY_FX; break;
          case SDLK_SPACE: ret = KEY_FY; break;
          case SDLK_TAB: ret = KEY_FL; break;
          case SDLK_BACKSPACE: ret = KEY_FR; break;
          case SDLK_ESCAPE: ret = KEY_SELECT; break;
          case SDLK_RETURN: ret = KEY_MENU; break;
          case SDLK_PAUSE: ret = KEY_LOCK; break;
#endif
          default: break;
        }
      break;
    case SDL_KEYUP:
      ret = KEY_NONE; break;
    default: break;
    }
  }
#endif

	return ret;
}

// espera a una tecla pulsada
int WaitKey()
{
  while(true) {
    int key = ReadKey();
    if(key != KEY_NONE)
      return key;
    SDL_Delay(1);
  }
}

// espera ninguna tecla pulsada
void WaitNoKey()
{
  while(ReadKey() != KEY_NONE)
    SDL_Delay(1);
}

void ClearScreen()
{
  if(!screen) return;
  memset(screen->pixels,0,(screen->pitch)*SCR_HEIGHT);
}

////////////////////////////////////////////////

// asigna el color del texto, def=blanco
void TextColor(int rgba)
{
  if(!screen) return;
  m_txcolor = rgba;
}

// escribe texto en la fila y columna dada
void WriteString(int f, int c, const char *msg)
{
  if(!screen) return;
  stringColor(screen, c*8+1, f*8+1, msg, m_txcolor);
}

// escribe texto centrado en la fila f
void WriteCenter(int f, const char *msg)
{
  int col = (SCR_COLS - strlen(msg)) / 2;
  //if(!(strlen(msg) % 2)) col--;
  WriteString(f, col, msg);
}

// escribe una cadena con formato en f,c
void WriteFormat(int f, int c, const char *format, ...)
{
  char buffer[4096];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  WriteString(f, c, buffer);
}

