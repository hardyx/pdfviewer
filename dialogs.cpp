//////////////////////////////////////////////////
// Dialogs, modulo de ventanas y diálogos.

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "xpvideo.h"
#include "wizdisk.h"
#include "dialogs.h"

// lista de cadenas
typedef std::vector<TString> TStringList;

// variables privadas
static TPoint windab(0,0); // c,f origen

//////////////////////////////////////////////////

// pantalla de espera con mensaje
void WaitScreen(const char *title, const char *mens)
{
  ClearScreen();
  int fcen = SCR_ROWS / 2;
  TextColor(COL_YELLOW);
	WriteCenter(fcen-2, title);
  TextColor(COL_WHITE);
	WriteCenter(fcen+2, mens);
	SDL_Flip(screen);
}

// pinta una ventana centrada
void WindowCen(int cols, int filas, int color)
{
  int x = (SCR_WIDTH - cols*8)/2;
  x &= ~7; x++;
  int y = (SCR_HEIGHT - filas*8)/2;
  y &= ~7; y++;
  boxColor(screen, x+6, y+6, x+cols*8-1+6, y+filas*8-1+6, COL_GRAY);
  boxColor(screen, x, y, x+cols*8-1, y+filas*8-1, color);
  rectangleColor(screen, x-1, y-1, x+cols*8, y+filas*8, COL_BLACK);
  // guardar fila y col de origen
  windab = TPoint(x/8, y/8);
}

// pinta la fila f de color desde f,c
void DrawBar(int f, int c, int width, int color)
{
  int dec = (f>0)? 2: 0;
  boxColor(screen, c*8,f*8-dec, (c+width)*8, f*8+10, color);
}

// devuelve el ancho de la línea más larga
static int GetAnchoMax(const char *lins[], int num)
{
  int ancho = 0;
  // calcular el ancho maximo
  for(int i=0; i<num; i++) {
    int len = strlen(lins[i]);
    if(len > ancho) ancho=len;
  }
  return ancho;
}

// dibuja el menú dado con la opcion opsel marcada
static void DrawMenu(const char *mops[], int num, int opsel, int colbk, int coltx)
{
  // doble espacio de lineas
  int alto = num*2 + 1;
  int ancho = GetAnchoMax(mops, num);
  ancho += 4; // margen
  WindowCen(ancho, alto, colbk);
  int fila = windab.y + 1;

  for(int i=0; i<num; i++) {
    if(i == opsel)
      DrawBar(fila, windab.x+1, ancho-2, COL_SELBAR);
    TextColor(i==opsel? COL_TEXTHI: coltx);
    WriteCenter(fila, mops[i]);
    fila += 2;
  }
}

// muestra una cadena ajustada en una ventana y espera tecla
void ShowMessage(const char *mens, int colbk, int coltx, bool wait/*=true*/)
{
	WaitNoKey();
  int len = (int)strlen(mens);
  int ancho = MIN(SCR_COLS-4, len);
  int nlins = (len / ancho) + (len%ancho!=0);
  if(nlins > SCR_ROWS-2) // recortar msg
		nlins = SCR_ROWS-2;
  const char *pinic = mens;
  WindowCen(ancho+2, nlins*2+1, colbk);
  int fila = windab.y + 1;

  TextColor(coltx);
  while(pinic) { // trocear mensaje
    WriteFormat(fila, windab.x+1, "%.*s", ancho, pinic);
    if((int)strlen(pinic) > ancho)
      pinic += ancho;
    else pinic=0;
    fila +=2;
  }
  SDL_Flip(screen);
  if(wait) WaitKey();
}

// muestra el mensaje en rojo y espera tecla
void ShowError(const char *mens)
{
  ShowMessage(mens, COL_RED, COL_WHITE);
  SDL_Flip(screen);
	WaitNoKey();
}

// muestra una lista de cadenas en una ventana, sin \n
void ShowListDlg(const char *lins[], int num, int colbk, int coltx)
{
	WaitNoKey();
  // doble espacio de lineas
  int alto = num*2 + 1;
  int ancho = GetAnchoMax(lins, num);
  WindowCen(ancho+2, alto, colbk);
  int fila = windab.y + 1;

  TextColor(coltx);
  for(int i=0; i<num; i++) {
    WriteFormat(fila, windab.x+1, "%.*s", ancho, lins[i]);
    fila += 2;
  }
  SDL_Flip(screen);
  WaitKey();
}

// muestra menú y da opción elegida
int GetMenu(const char *mops[], int num, int colbk, int coltx)
{
	WaitNoKey();
  int opc=0, key=0;
  do {
    if(key != KEY_NONE) {
      DrawMenu(mops, num, opc, colbk, coltx);
      SDL_Flip(screen);
    }
    SDL_Delay(70);
    key = ReadKey();
    if(key == KEY_UP) {
      if(opc>0) opc--; else opc=num-1;
    }
    else if(key == KEY_DOWN) {
      if(opc<num-1) opc++; else opc=0;
    }
    else if(key == KEY_FA) // cancelar
      return ID_CANCEL;
  }
  while(key != KEY_FB);
  return opc;
}

// devuelve el número de palabras de la lista
static int GetNumWords(const TString &lista, char sep=' ')
{
  int num = 0;
  for(int i=0; i<(int)lista.length(); i++) {
    if(lista[i]==sep) num++;
  }
  if(*(lista.end()-1) != sep)
    num++;
  return num;
}

// da la posición donde empieza la palabra ind en lista. -1=error
static int FindWord(int ind, const TString &lista, int *wlen, char sep=' ')
{
  int pos=-1;
  for(int num=0; num < ind; num++) {
    pos = lista.find(sep, pos+1);
    if(pos == (int)lista.npos) // no existe
      return -1;
  }
  if(wlen) {
    // devolver longitud de la palabra
    int spc = lista.find(sep, pos+1);
    if(spc != (int)lista.npos)
      *wlen = spc-pos-1;
    else
      *wlen = lista.length()-pos-1;
  }
  return pos+1;
}

// dibuja las opciones con su etiqueta y valor resaltado
static void DrawOptions(int f, const TOption options[], int num, int lin, int colbk, int coltx)
{
  int fila = windab.y+f;
  int col = windab.x+1;

  for(int i=0; i<num; i++) {
    TextColor(coltx);
    const TOption &opt = options[i];
    int len = opt.label.length()+2;

    // borrar fondo de todos los valores, ok!
    DrawBar(fila, col+len, opt.listvals.length(), colbk);
    // poner fondo resaltado con valor actual
    int wlen = 1;
    int pos = FindWord(opt.value, opt.listvals, &wlen);
    DrawBar(fila, col+len+pos, wlen, COL_SELBAR);

    // escribir la etiqueta
    TString linea = opt.label + ": ";
    linea += opt.listvals;
    WriteString(fila, col, linea.c_str());

    // mostrar el valor selecc en otro color
    TextColor(i==lin? COL_TEXTHI: colbk);
    TString selval = opt.listvals.substr(pos,wlen);
    WriteString(fila, col+len+pos, selval.c_str());
    fila += 2;
  }
}

// ventana de opciones y actualiza variables, true si hay cambios
bool OptionsDlg(TOption options[], int num, int colbk, int coltx)
{
	WaitNoKey();
  // calcular campos _nvals
  for(int i=0; i<num; i++)
    options[i]._nvals = GetNumWords(options[i].listvals);

  // doble espacio de lineas
  int alto = num*2 + 7;
  WindowCen(SCR_COLS-2, alto, colbk);
	WriteCenter(windab.y+1, "== Options ==");
	WriteString(windab.y+alto-2, 2, "(B) Accept, (A) Cancel");
	int key=0, lin=0;

	do {
    TOption &opt = options[lin];
    if(key != KEY_NONE) {
      DrawOptions(4, options, num, lin, colbk, coltx);
      SDL_Flip(screen);
	  }
    SDL_Delay(70);
    key = ReadKey();
    // moverse por las líneas
    if(key == KEY_UP) {
      if(lin>0) lin--;
    }
    else if(key == KEY_DOWN) {
      if(lin<num-1) lin++;
    }
    // moverse por los valores
    if(key == KEY_LEFT) {
      if(opt.value>0) opt.value--;
    }
    else if(key == KEY_RIGHT) {
      if(opt.value<opt._nvals-1) opt.value++;
    }
    // salir sin guardar, cancelar
    else if(key == KEY_FA)
      return false;
	}
  while(key != KEY_FB);
	return true;
}

// dibuja un campo numérico justificado, con el cursor en digito
static void DrawNumField(int f, int c, char *strvalor, int digito, int colbk, int coltx)
{
  int fila = windab.y+f;
  int col = windab.x+c;

  TextColor(coltx);
  // borrar el contenido del campo
  int ancho = strlen(strvalor);
  DrawBar(fila-1, col, ancho, colbk);
  DrawBar(fila, col, ancho, colbk);
  DrawBar(fila+1, col, ancho, colbk);

  // escribir el número
  WriteString(fila, col, strvalor);

  // mostrar el cursor en el digito
  if(digito < ancho) {
    WriteString(fila-1, col+digito, "\x1e");
    WriteString(fila+1, col+digito, "\x1f");
  }
}

// ventana de introducir un número, true si no cancelado
bool GetNumberDlg(const char *msg, int min, int max, int *number, int colbk, int coltx)
{
  const int ancho = 5;
	WaitNoKey();

  int alto = 2 + 7;
  WindowCen(25, alto, colbk);
  TString titulo = "=== ";
  titulo += msg + TString(" ===");
	WriteCenter(windab.y+1, titulo.c_str());
	WriteString(windab.y+4, windab.x+1, "Number: ");

	char strrango[20];
	sprintf(strrango, "(%d-%d)", min, max);
	WriteString(windab.y+4, windab.x+10+ancho, strrango);
	WriteString(windab.y+alto-2, windab.x+1, "(B) Accept, (A) Cancel");

  char strvalor[20]; // valor cadena
  sprintf(strvalor, "%0*d", ancho, *number);
	int key=0, digito=0; //, valor;

	do {
    if(key != KEY_NONE) {
      DrawNumField(4, 9, strvalor, digito, colbk, coltx);
      SDL_Flip(screen);
	  }
    SDL_Delay(70);
    key = ReadKey();
    // subir y bajar el digito actual
    if(key==KEY_UP) {
       if(strvalor[digito]<'9')
        strvalor[digito]++;
    }
    else if(key==KEY_DOWN) {
       if(strvalor[digito]>'0')
        strvalor[digito]--;
    }
    // moverse por los dígitos
    if(key == KEY_LEFT) {
      if(digito>0) digito--;
    }
    else if(key == KEY_RIGHT) {
      if(digito<ancho-1) digito++;
    }
    // cancelar intro de número
    else if(key == KEY_FA)
      return false;
	}
  while(key != KEY_FB);

  int valor = atoi(strvalor);
  if(valor<min || valor>max) {
    ShowError("Page number out of range");
    return false;
  }
  *number = valor;
	return true;
}

////////////////////////////////////////////////////////////

// muestra los ficheros dados desde inic a inic+nlins-1, con isel marcado
static void DrawFileList(const TFileList &lfiles, int isel, int inic, int nlins, int colbk, int coltx)
{
  int ancho = 30;
  int cuantos = MIN((int)lfiles.size()-inic, nlins);
  WindowCen(ancho, nlins*2+1, colbk);

  int fini = windab.y + 1;
  for(int i=0; i<cuantos; i++) {
    const TFileInfo &file = lfiles[i+inic];
    if(i+inic == isel)
      DrawBar(fini+i*2, windab.x+1, ancho-2, COL_SELBAR);
    const char *name = file.name.c_str();
    if(file.attr.type == 'd') { // dir
      TextColor(i+inic==isel? COL_TEXTHI: COL_RED);
      WriteFormat(fini+i*2, windab.x+1, "<%.*s>", ancho-4, name);
    }
    else { // fichero
      TextColor(i+inic==isel? COL_TEXTHI: coltx);
      WriteFormat(fini+i*2, windab.x+1, "%.*s", ancho-2, name);
    }
  }

  if(lfiles.size() <= 1) { //.. o nada
    TextColor(COL_TEXTO);
    WriteCenter(nlins+4, "-No documents found-");
    return;
  }
}

// recorta el último directorio del path
static void TrimLastDir(std::string &path)
{
  int pos = path.rfind(CDIR_SEP);
  if(pos != (int)path.npos) {
    if(!pos) pos++; // dejar raíz
    path.erase(pos, path.npos);
  }
}

// muestra los ficheros de path\*.ext, y da el elegido. 1 si ok.
int FileDialog(const char *path, const char *ext, TString *selfile)
{
  if(!path || !ext || !selfile)
    return ID_ERROR;
	WaitNoKey();
	*selfile = "";

  CWizDisk disk;
  TFileList lfiles; // ext con punto
  if(!disk.DirectoryList(path, ext, &lfiles)) {
    ShowError("Error reading directory");
    return ID_ERROR;
  }

  const int nlins = 10;
  TString curpath(path); // copia
  int key=0, num = lfiles.size();
  int isel = 0, inic = 0;
  bool elegido = false;

  while (!elegido) {
    if(key != KEY_NONE) {
      DrawFileList(lfiles, isel, inic, nlins, COL_FONDO, COL_TEXTO);
      SDL_Flip(screen);
    }
    SDL_Delay(70);
    key = ReadKey();
    if(key == KEY_UP) {
      if(isel>0) isel--;
      if(isel<inic) inic=MAX(isel-nlins+1,0);
    }
    if(key == KEY_DOWN) {
      if(isel<num-1) isel++;
      if(isel>=inic+nlins) inic=isel;
    }
    else if(key == KEY_FA) // cancelar
      return ID_CANCEL;
    else if(key == KEY_FB) {
      // elegir archivo o cambiar dir
      const TFileInfo &info = lfiles[isel];
      if(info.attr.type == 'a')
        elegido = true;
      else if(info.attr.type == 'd') {
        if(info.name == "..")
          TrimLastDir(curpath);
        else {
          // añadir barra si no tiene al final
          if(*(curpath.rbegin()) != CDIR_SEP)
            curpath += CDIR_SEP;
          curpath += info.name;
        }
        lfiles.clear();
        // leer la lista del nuevo directorio
        if(!disk.DirectoryList(curpath.c_str(), ext, &lfiles)) {
          ShowError("Error reading directory");
          return false;
        }
        num = lfiles.size();
        isel = inic = 0;
      }
    }
  }

  if(num > 0) {
    // jmd, arreglado path del fichero
    *selfile = curpath + CDIR_SEP;
    *selfile += lfiles[isel].name;
  }
  return ID_OK;
}

