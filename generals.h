#ifndef GENERALS_H
#define GENERALS_H

#include <vector>
#include <string>

// macros min y max
#define MIN(a,b) ((a)<(b)? (a): (b))
#define MAX(a,b) ((a)>(b)? (a): (b))

// separa directorios
#ifdef WIN32
# define SDIR_SEP "\\"
# define CDIR_SEP '\\'
#else
# define SDIR_SEP "/"
# define CDIR_SEP '/'
#endif

// cadena de la STL
typedef std::string TString;

// tipo punto simple
struct TPoint {
	int x,y;
	TPoint() { x=y=0; }
	TPoint(int a,int b) { x=a; y=b; }
	void Mult(float val) { x=(int)(x*val); y=(int)(x*val); };
};

// tipo tamaño simple
struct TSize {
	int cx,cy;
	TSize() { cx=cy=0; }
	TSize(int x,int y) { cx=x; cy=y; }
};

#endif // GENERALS_H
