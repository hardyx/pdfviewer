#ifndef DIALOGS_H
#define DIALOGS_H

#include "generals.h"
#include "xpvideo.h"

// resultado de un dialogo
#define ID_OK      1
#define ID_CANCEL  -1
#define ID_ERROR   0

// colores de la aplicacion
#define COL_TEXTO  COL_BLACK
#define COL_FONDO  RGBA(192,192,255)
#define COL_SELBAR COL_PURPLE
#define COL_TEXTHI COL_WHITE

// datos para una opción
struct TOption {
	TString label, listvals;
	short value, _nvals; // _:autocalc
};

void WaitScreen(const char *title, const char *mens);
void WindowCen(int cols, int filas, int color);
void DrawBar(int f, int c, int width, int color);
int GetMenu(const char *mops[], int num, int colbk, int coltx);
int FileDialog(const char *path, const char *ext, TString *selfile);
void ShowMessage(const char *mens, int colbk, int coltx, bool wait=true);
void ShowError(const char *mens);
void ShowListDlg(const char *lins[], int num, int colbk, int coltx);
bool OptionsDlg(TOption options[], int num, int colbk, int coltx);
bool GetNumberDlg(const char *msg, int min, int max, int *number, int colbk, int coltx);

#endif // DIALOGS_H
