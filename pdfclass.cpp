#include "aconf.h"
#include <stdio.h>
#include <SDL.h>
#include "gmem.h"
#include "GString.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "SplashBitmap.h"
#include "Splash.h"
#include "SplashOutputDev.h"
#include "config.h"
#include "generals.h"
#include "wizdisk.h"
#include "dialogs.h"
#include "pdfclass.h"

TPdfClass::TPdfClass()
{
  docu = NULL;
  splashOut = NULL;
  curpage = 0;
  zoom = 100.0;
  dpiresol = 72; // dpi
  enableT1lib = false;
  enableFreeType = false; // no ttf
  antialias = true;
  reversevid = false;
  msgquiet = false;
  showinfo = true;
  firstrender = true;
  initzoom = 100;
  rotation = 0; // grad
  docname = "<no file>";
}

TPdfClass::~TPdfClass()
{
  if(docu || splashOut)
    Release();
}

int TPdfClass::NumPages() const
{
  return docu? docu->getNumPages(): 0;
}

void TPdfClass::ClearDoc()
{
  if(docu) {
    // libera también fileName
    delete docu; docu = 0;
  }
  if(splashOut) {
    // esto limpia la página
    splashOut->startDoc(NULL);
    splashOut->startPage(0, NULL);
  }
}

void TPdfClass::Release()
{
	printf("Release\n");
  ClearDoc();
  if(splashOut) {
    delete splashOut;
    splashOut = 0;
  }
  if(globalParams) {
    delete globalParams;
    globalParams = 0;
  }

  // check for memory leaks
  //Object::memCheck(stderr);
  //gMemReport(stderr);
}

// funciona!!
// devuelve el bitmap rgb convertido al formato de pantalla
SDL_Surface *TPdfClass::ConvertBitmap(SDL_Surface *screen, const SplashBitmap *bitmap)
{
  if(!screen || !bitmap)
    return NULL;

  // obtener datos de la imagen original
  int width = bitmap->getWidth();
  int height = bitmap->getHeight();
  int rowsize = bitmap->getRowSize();

  // crear una superficie para los pixels RGB
  SplashColorPtr pdata = bitmap->getDataPtr();
  SDL_Surface *rgbpage = 0;
  rgbpage = SDL_CreateRGBSurfaceFrom(pdata, width, height, 24, rowsize, 0xff, 0xff00, 0xff0000, 0);

  // convertir la superficie al fmt de la pantalla
  SDL_Surface *dispage = 0;
  dispage = SDL_DisplayFormat(rgbpage);
  rgbpage->pixels = 0; // no liberar datos
  SDL_FreeSurface(rgbpage);

  return dispage;
}

// funciona!!
// muestra la página en pantalla con offset x,y
void TPdfClass::ShowImage(SDL_Surface *dispage, SDL_Surface *screen,
  int ofsx/*=0*/, int ofsy/*=0*/)
{
  if(dispage && screen) {
    int ancho = MIN(dispage->w, screen->w);
    int alto = MIN(dispage->h, screen->h);

    // si es menor que pantalla, borrar
    if(ancho<screen->w || alto<screen->h)
      SDL_FillRect(screen, NULL, 0);

    SDL_Rect srcrect = {ofsx, ofsy, ancho, alto};
    SDL_BlitSurface(dispage, &srcrect, screen, NULL);
  }
}

// inicia los parámetros de la librería
void TPdfClass::InitParams()
{
  if (!msgquiet) {
    printf("lib xpdf version %s\n", xpdfVersion);
    printf("%s\n", xpdfCopyright);
  }

  CWizDisk disk;
  char path[MAX_PATH];
  disk.GetCurrPath(path, true);
  pathapp = path; // guardar

#ifdef GCW0
  // guardar config en $home/.appname
  if (disk.GetHomeDir(path, ".pdfviewer")) {
    if(!disk.MakeDirCheck(path, true))
      ShowError("Error creating config dir");
    strcat(path, SDIR_SEP);
    pathcfg = path; // guardar
  }
#else // win32, gph
  pathcfg = pathapp;
#endif

  // read default global params
	printf("Loading globalparams\n");
	if(!globalParams)
    globalParams = new GlobalParams(NULL);

  // ahora usa las fuentes type1
  char dirfonts[MAX_PATH];
  strcpy(dirfonts, pathapp.c_str());
  strcat(dirfonts, "basfonts");
  globalParams->setupBaseFonts(dirfonts);
  if(!disk.DirExists(dirfonts))
    ShowError("basfonts folder not found");

  if (enableT1lib) {
    // needs HAVE_T1LIB_H activated
    if (!globalParams->setEnableT1lib(enableT1lib? "yes": "no"))
      fprintf(stderr, "Bad t1lib value\n");
  }
  if(enableFreeType) {
    if(!globalParams->setEnableFreeType(enableFreeType? "yes": "no"))
      fprintf(stderr, "Bad freetype value\n");
  }
  globalParams->setErrQuiet(msgquiet? gTrue: gFalse);

  // crear el dispositivo para volcar páginas
  SplashColor paperWhite = {0xff, 0xff, 0xff};
  SplashColor paperBlack = {0,0,0};
  if(splashOut)
    delete splashOut;
  splashOut = new SplashOutputDev(splashModeRGB8, 1, reversevid? gTrue: gFalse,
    reversevid? paperBlack: paperWhite, gTrue, antialias? gTrue: gFalse);
  splashOut->startDoc(NULL);
}

// crear fichero de opciones por defecto
bool TPdfClass::CreateDefOptions(const TString &fpath)
{
  // 1 100 0 0 1
  initzoom = 100;
  showinfo = antialias = 1;
  rotation = reversevid = 0;
  return SaveOptions(fpath);
}

// graba las opciones del visor en un fichero
bool TPdfClass::SaveOptions(const TString &path)
{
  FILE *f = fopen(path.c_str(), "wt");
  if(!f)
    return false;

  // grabar en el orden del diálogo
  fprintf(f, "%hd %hd %hd %hd %hd",
    showinfo, initzoom, rotation, reversevid, antialias
  );

  fclose(f);
  return true;
}

// carga las opciones del visor de un fichero
bool TPdfClass::LoadOptions(const TString &path)
{
  FILE *f = fopen(path.c_str(), "rt");
  if(!f)
    return false;

  // cargar en el orden del diálogo
  short shi, rvi, ata;
  fscanf(f, "%hd %hd %hd %hd %hd",
    &shi, &initzoom, &rotation, &rvi, &ata
  );
  showinfo = (shi==1);
  antialias = (ata==1);
  reversevid = (rvi==1);

  fclose(f);
  return true;
}

// actualiza outputdev con los parámetros
void TPdfClass::UpdateOutput()
{
  if(splashOut) {
    SplashColor paperWhite = {0xff, 0xff, 0xff};
    SplashColor paperBlack = {0,0,0};
    delete splashOut;
    splashOut = new SplashOutputDev(splashModeRGB8, 1, reversevid? gTrue: gFalse,
      reversevid? paperBlack: paperWhite, gTrue, antialias? gTrue: gFalse);
    splashOut->startDoc(NULL);
  }
}

// da el nombre de fichero del path dado
const char *TPdfClass::GetFileName(const char *path)
{
  char *p = strrchr(path,'/');
  if(!p) p = strrchr(path,'\\');
  return (p? p+1: "file");
}

// cargar en memoria el fichero pdf dado
bool TPdfClass::LoadFile(const char *path)
{
	printf("LoadFile %s\n", path);
  if(!path || !globalParams)
    return false;
  if(docu) // liberar anterior
    ClearDoc();
  // actualiza output con options
  UpdateOutput();

  // abrir y cargar el documento pdf
  GString *fname = new GString(path); // del doc
  docu = new PDFDoc(fname, NULL, NULL);
  docname = path; // pruebas
  if(!docu->isOk())
    return false;

  // guardar el nombre del fichero
  docname = GetFileName(path);
  firstrender = true;
  SetZoom(initzoom);
  curpage = 1;
  return true;
}

void TPdfClass::SetZoom(float val/*=100*/)
{
  dpiresol = (int)((72 * val) / 100);
  zoom = val;
}

// renderiza la página dada en el dispositivo
bool TPdfClass::RenderPage(TSize *imgsize)
{
  if(!docu || !splashOut)
    return false;
  if(curpage<1 || curpage>docu->getNumPages())
    return false;
  if(!firstrender)
    ShowMessage("Rendering...", COL_FONDO, COL_TEXTO, false);
  else // primera
    firstrender = false;

  // TODO, pintar sólo lo que se ve en pantalla
  splashOut->startDoc(docu->getXRef());
  docu->displayPage(splashOut, curpage, dpiresol, dpiresol, rotation, gFalse, gTrue, gFalse);

  if(imgsize) {
    imgsize->cx = splashOut->getBitmapWidth();
    imgsize->cy = splashOut->getBitmapHeight();
  }
  return true;
}

// mostrar barra de información
void TPdfClass::ShowInfoBar()
{
  if(showinfo) {
    TextColor(COL_TEXTO);
    DrawBar(0, 0, SCR_COLS, COL_FONDO);
    WriteFormat(0, 0, "%.25s", docname.c_str());
    char aux[40];
    sprintf(aux, "%d/%d (%.0f%%)", curpage, NumPages(), zoom);
    int rcol = SCR_COLS - strlen(aux);
    WriteString(0, rcol, aux);
  }
}

// mostrar la página en la pantalla con el offset dado
bool TPdfClass::ShowPage(SDL_Surface *screen, int ofsx, int ofsy)
{
  if(!IsLoaded())
    return false;

  // bitmap es una propiedad interna
  const SplashBitmap *bitmap = splashOut->getBitmap();
  if(bitmap) {
    SDL_Surface *dispage = ConvertBitmap(screen, bitmap);
    // copiar el bitmap (o parte) a la pantalla
    ShowImage(dispage, screen, ofsx, ofsy);
    // liberar la imagen de la página
    SDL_FreeSurface(dispage);
  }

  return true;
}

