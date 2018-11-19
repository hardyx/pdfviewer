//========================================================================
//
// SplashBitmap.h
//
//========================================================================

#ifndef SPLASHBITMAP_H
#define SPLASHBITMAP_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

class SplashBitmap {
public:

  // Create a new bitmap.  It will have <widthA> x <heightA> pixels in
  // color mode <modeA>.  Rows will be padded out to a multiple of
  // <rowPad> bytes.  If <topDown> is false, the bitmap will be stored
  // upside-down, i.e., with the last row first in memory.
  SplashBitmap(int widthA, int heightA, int rowPad,
	       SplashColorMode modeA, GBool alphaA,
	       GBool topDown = gTrue);

  ~SplashBitmap();

  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getRowSize() const { return rowSize; }
  int getAlphaRowSize() const { return width; }
  SplashColorMode getMode() const { return mode; }
  SplashColorPtr getDataPtr() const { return data; }
  Guchar *getAlphaPtr() const { return alpha; }

  SplashError writePNMFile(char *fileName);

  void getPixel(int x, int y, SplashColorPtr pixel);
  Guchar getAlpha(int x, int y);

private:

  int width, height;		// size of bitmap
  int rowSize;			// size of one row of data, in bytes
				//   - negative for bottom-up bitmaps
  SplashColorMode mode;		// color mode
  SplashColorPtr data;		// pointer to row zero of the color data
  Guchar *alpha;		// pointer to row zero of the alpha data
				//   (always top-down)

  friend class Splash;
};

#endif
