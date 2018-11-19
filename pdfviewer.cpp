///////////////////////////////////////////////////////////////////////////////
// PdfViewer, visor de pdf usando la librería xpdf y splashbitmap.
///////////////////////////////////////////////////////////////////////////////
// * carga del fichero y obtención de la imagen en 16 bpp.
// * manejo de las teclas del cursor y de L, R.
// * funciona ok con graficos, fuentes, tablas, etc.
// * arreglado fallo al borrar doc, fileName owned.
// * lista de ficheros y dirs sencilla mostrar solo pdf.
// * implementar el zoom (resoluc) con +/-, redibujar pag.
// * gestionar directorios para moverse en vez de cargar.
// * encuentra las fuentes type1 en Wiz y Pc y salen bien.
// * cambiadas fuentes ttf por type1 y añadidas de ghostcript.
// * waitnokey al inic dlgs para evitar pulsaciones anteriores.
// * detectar a=cancelar en filedlg para cerrar sin cargar doc.
// * mensaje al renderizar cada pagina, excepto en la primera.
// * añadir retardo o waitnokey en las teclas en Wiz.
// * ventana de ayuda con las teclas usadas en el programa.
// * ventana de opciones con listas valores parámetros.
// * no funcionaba cancelar con A en ventana de opciones.
// * pasar los mensajes a inglés, dialogs y pdfviewer.
// * version gp2x en estático y caanoo, manejo de teclas.
// * en caanoo hlp1/hlp2 para zoom, home para menu.
// * al cambiar orientación mostraba lo anterior en los bordes.
// * dialogo de saltar a una pagina cogiendo el numero, ok.
// * corregido error con el path actual al cargar documentos.
// * las opciones se guardan y se cargar de un fichero.
// * si dir docums no existe, mostrar el dir de la app.
// * gestionadas las teclas y joystick de gcw para zoom.
// * gestionar fichero config en dir de usuario para gcw.
// * inverse vid y antialias no se activan al cargar un pdf.
// * fallaba al subir al dir raiz de linux, arreglado.
// - falla al cargar los metadatos del opk en gcw zero.
// - pdfs asciihex y plaintext (codec streams) no los carga.
// - acelerar el renderizado pdf usando tiles, mirar renderpage.
// - colocar el origen de angulos verticales en su posición.
// - poner las teclas de zoom en incrementos de 10%.
// - mostrar posibles errores de carga del pdf en ventana.
//   salida de errores en xpdf/error.cpp, se puede guardar último.
//

#include "aconf.h"
#include <stdio.h>
#include <string.h>
#include "PDFDoc.h"
#include "xpvideo.h"
#include "dialogs.h"
#include "wizdisk.h"
#include "pdfclass.h"

// defines usados
#define VERSION "0.92 beta"
#define DIR_DOCS "docums"
#define OPT_FILENAME "pdfvw.opt"
#undef TESTFILE

// funciones locales
bool LoadDocument(const char *path);
void CargarFich();
void Opciones();
void Ayuda();
void AcercaDe();

// variables locales
TPdfClass pdf;
TPoint ofs(0,0); // offsets img
TSize limits;    // tamaño de página

///////////////////////////////////////////////////////////////////////

// maneja menu principal, 0=salir
int MainMenu()
{
  const int nops = 5;
  const char *menupr[nops] = {
    "Load document","Options","Quick help","About","Exit viewer"
  };
  int opc = GetMenu(menupr, nops, COL_FONDO, COL_TEXTO);

  switch(opc) {
    case 0:
      CargarFich(); break;
    case 1:
      Opciones(); break;
    case 2:
      Ayuda(); break;
    case 3:
      AcercaDe(); break;
    case 4:
      return 0;
  }
  return 1;
}

void CargarFich()
{
  CWizDisk disk;
  char path[MAX_PATH];
#ifndef GCW0
  disk.GetCurrPath(path, true);
  strcat(path, DIR_DOCS);
  // si no hay dir docums, abrir path app
  if(!disk.DirExists(path))
    strcpy(path, pdf.GetAppPath().c_str());
#else // gcw0
  strcpy(path, "/media/");
#endif

  TString selfile;
  int res = FileDialog(path, ".pdf", &selfile);
  if(!res)
    printf("Error in FileDialog\n");
  else if(res != ID_CANCEL)
    LoadDocument(selfile.c_str());
}

void Opciones()
{
  const int nops = 5;
  TOption opcs[nops] = {
    {"Show infobar ", "no yes", pdf.showinfo, 0},
    {"Initial zoom ", "50 75 100 125 150", pdf.initzoom/25-2, 0},
    {"Page rotation", "-90 0 90", pdf.rotation/90+1, 0},
    {"Inverse video", "no yes", pdf.reversevid, 0},
    {"Antialias    ", "no yes", pdf.antialias, 0}
  };

	if(OptionsDlg(opcs, nops, COL_FONDO, COL_TEXTO)) {
	  // guardar los valores elegidos
	  short showinfo = opcs[0].value;
	  short initzoom = (opcs[1].value+2)*25;
	  short rotation = (opcs[2].value-1)*90;
	  short videoinv = opcs[3].value;
	  short antialias = opcs[4].value;

    // al cambiar rotación, poner origen cero
    if(pdf.rotation != rotation)
      ofs.x = ofs.y = 0;

	  // pasar las opciones a params del pdf
    pdf.showinfo = (showinfo==1);
    pdf.initzoom = initzoom;
    pdf.rotation = rotation;
    pdf.antialias = (antialias==1);
    pdf.reversevid = (videoinv==1);

    // grabar las opciones
    TString pathopt(pdf.GetConfPath());
    pathopt += OPT_FILENAME;
    if(!pdf.SaveOptions(pathopt))
      ShowError("Error saving options file");

    if(pdf.IsLoaded()) {
      pdf.UpdateOutput();
      pdf.RenderPage(&limits);
    }
	}
}

void GotoPage(TPoint &pos)
{
  if(pdf.IsLoaded()) {
    int max = pdf.NumPages();
    int npage = pdf.curpage;

    // mover a la pagina dada, ok
    if(GetNumberDlg("Go to page", 1, max, &npage, COL_FONDO, COL_TEXTO)) {
      pdf.curpage = npage;
      pos = TPoint();
      pdf.RenderPage(&limits);
    }
	}
}

void Ayuda()
{
  const int nlins = 10;
  const char *lines[nlins] = {
    "PDF viewer help", "",
#ifdef CAANOO
    "Home: Shows the menu",
#else
    "Select: Shows the menu",
#endif
    "Pad: Moving in page", "L/R: Previous and next page",
#ifdef CAANOO
    "Help I/II: Zoom in and out",
#elif defined(GCW0)
    "Joy up/dn: Zoom in and out",
#else
    "Vol +/-: Zoom in and out",
#endif
#ifdef GCW0
    "A: Choose options, accept", "B: Hide menu, cancel",
#else
    "B: Choose options, accept", "A: Hide menu, cancel",
#endif
    "X: Go to page number", "Y: Show or hide info bar"
  };
  ShowListDlg(lines, nlins, COL_FONDO, COL_TEXTO);
}

void AcercaDe()
{
  const int nlins = 7;
  const char *lines[nlins] = {
    "PdfViewer v" VERSION, "Document viewer for handhelds.",
    "Created by Hardyx, 2010-2014.", "",
    "Based in xpdf 3.02:", "www.foolabs.com/xpdf",
    "Ghostscript type1 fonts"
  };
  ShowListDlg(lines, nlins, COL_FONDO, COL_TEXTO);
}

// mover el pos.xy según el botón pulsado, da 0 si salir
int ManejarBotones(int key, TPdfClass &pdf, TPoint &pos, TSize &limits)
{
  const int inc = 10;
	bool scroll = false;
  if (key==KEY_LEFT && pos.x>0) { // v
    pos.x -= inc; scroll = true;
	}
  else if (key==KEY_UP && pos.y>0) {// ^
    pos.y -= inc; scroll = true;
	}
  else if (key==KEY_RIGHT && pos.x+inc<limits.cx-SCR_WIDTH) { // >
    pos.x += inc; scroll = true;
	}
  else if (key==KEY_DOWN && pos.y+inc<limits.cy-SCR_HEIGHT) { // <
    pos.y += inc; scroll = true;
	}
  if (key==KEY_FX) { // x
    GotoPage(pos);
	}
  if (key==KEY_FY) { // y
    pdf.showinfo = !pdf.showinfo;
	}
  if (key==KEY_SELECT) { // select
    if(!MainMenu()) return 0;
  }
  else if (key==KEY_FL && pdf.curpage>1) { // l
    pdf.curpage--; pos = TPoint();
    pdf.RenderPage(&limits);
  }
  else if (key==KEY_FR && pdf.curpage<pdf.NumPages()) { // r
    pdf.curpage++; pos = TPoint();
    pdf.RenderPage(&limits);
  }
  else if (key==KEY_VOLDN && pdf.zoom>25) { // -
    pdf.SetZoom(pdf.zoom-25); pos.Mult(0.25);
    pdf.RenderPage(&limits);
  }
  else if (key==KEY_VOLUP && pdf.zoom<175) { // +
    pdf.SetZoom(pdf.zoom+25); pos.Mult(0.75);
    pdf.RenderPage(&limits);
  }

	// si no desplazamto, limpiar tecla
	if(!scroll) WaitNoKey();
  return 1;
}

// carga el documento Pdf y da false si error
bool LoadDocument(const char *path)
{
  ShowMessage("Loading file...", COL_FONDO, COL_TEXTO, false);

  if(!pdf.LoadFile(path)) {
    // aqui mostrar el error del doc
    GString caderr;
    caderr.appendf("Error {0:d} loading document", pdf.GetError());
    ShowError(caderr.CStr());
    pdf.ClearDoc(); // doc vacio
    return false;
  }
  printf("Loadfile, %d pags\n", pdf.NumPages());

  ofs = TPoint(0,0);
  if(!pdf.RenderPage(&limits)) {
    printf("Error rendering page %d\n", pdf.curpage);
    pdf.ClearDoc();
    return false;
  }
  return true;
}

// liberar los recursos
void MainDone()
{
  printf("MainDone\n");
  pdf.Release();
}

int main(int argc, char *args[])
{
  if(!VideoInit())
    return 1;
  atexit(MainDone);
  pdf.InitParams();

  // cargar las opciones
  TString pathopt(pdf.GetConfPath());
  pathopt += OPT_FILENAME;
  printf("Config file: %s\n", pathopt.c_str());
  if(!pdf.LoadOptions(pathopt))
    pdf.CreateDefOptions(pathopt);

#ifdef TESTFILE
  if(!LoadDocument(DIR_DOCS "/Patrones 1.pdf")) {
    ShowError("Error loading document");
    return 1;
  }
#endif

  int key = 0;
  while(true)	{
    if(key != -1) {
      if(!pdf.ShowPage(screen, ofs.x, ofs.y))
        WaitScreen("PdfViewer v" VERSION, "Press Select to show Menu");
      pdf.ShowInfoBar();
      SDL_Flip(screen);
    }
    SDL_Delay(50);
    key = ReadKey();
    if(!ManejarBotones(key, pdf, ofs, limits))
      break; // exit
  }
  return 0;
}

