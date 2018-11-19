/////////////////////////////////////////////////////////
// PdfClass, clase de alto nivel que gestiona el pdf

#ifndef PDFCLASS_H
#define PDFCLASS_H

#include "generals.h"

class GString;
class SplashBitmap;
class SplashOutputDev;
class PDFDoc;

//////////////////////////////////////
class TPdfClass
{
public:
  TPdfClass();
  virtual ~TPdfClass();
  void InitParams();
  void ClearDoc();
  bool IsLoaded() const { return docu!=NULL; }
  PDFDoc *GetDoc() const { return docu; }
  bool LoadFile(const char *path);
  void SetZoom(float val=100.0);
  bool CreateDefOptions(const TString &fpath);
  bool SaveOptions(const TString &fpath);
  bool LoadOptions(const TString &fpath);
  void UpdateOutput();
  bool RenderPage(TSize *imgsize);
  bool ShowPage(SDL_Surface *screen, int ofsx, int ofsy);
  void ShowInfoBar();
  int GetError() const { return docu? docu->getErrorCode(): -1; }
  float GetPercent() const { return 100.0f * curpage / NumPages(); }
  void IncPage(int inc=1) { curpage += inc; }
  const TString &GetAppPath() const { return pathapp; }
  const TString &GetConfPath() const { return pathcfg; }
  int NumPages() const;
  void Release();

private:
  const char *GetFileName(const char *path);
  SDL_Surface *ConvertBitmap(SDL_Surface *screen, const SplashBitmap *bitmap);
  void ShowImage(SDL_Surface *dispage, SDL_Surface *screen, int ofsx=0, int ofsy=0);

public:
  int curpage;    // pag actual
  float zoom;     // % de dpi sobre 72
  int dpiresol;   // dpi
  TString docname; // nombre fich
  short rotation;  // grados
  short initzoom;  // zoom inicial %
  bool showinfo;   // infobar
  bool antialias;  // suavizado
  bool reversevid; // invertir

private:
  PDFDoc *docu; // documento
  SplashOutputDev *splashOut;
  TString pathapp; // en initparams
  TString pathcfg; // en initparams
  bool enableT1lib;
  bool enableFreeType;
  bool firstrender;
  bool msgquiet;
};

// convierte lista de gstring a un array
inline void GStrs2Array(const GString lines[], int num, const char *aux[]) {
  for(int i=0; i<num; i++)
    aux[i] = lines[i].CStr();
}

#endif // PDFCLASS_H
