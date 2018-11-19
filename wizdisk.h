//////////////////////////////////////////////////////////////////////////
// CWizDisk: Disk and File management library

#ifndef _WIZDISK_H
#define _WIZDISK_H

#include "generals.h"

//////////////////////////////////////////////////////////////

#ifndef byte
#define byte unsigned char
#endif

#if defined(__arm__) || defined(__mips__)
#define MAX_PATH 512
#endif

//////////////////////////////////////////////////////////////

struct TFileAttr
{
  TFileAttr() { type='.'; size=mtime=0; }
  char type; // archive,dir,system,?
  long size;
  time_t mtime;
};

struct TDirEntry
{
  TDirEntry() { name=""; }
	TString name; // jmd
};

struct TFileInfo
{
  TFileInfo();
  TString DisplayName() const;
  TString name; // file o dir
  TFileAttr attr;
};

// lista de ficheros con atributos
typedef std::vector<TFileInfo> TFileList;

/////////////////////////////////////////////////
class CWizDisk
{
public:
	CWizDisk();
	void GetCurrPath(char *path, bool sep);
  bool FileGetSize(const char *filename, long *size);
	bool FileAttr(const char *name, TFileAttr *attr);

	bool DirExists(const char *dirname);
  bool IsRootDir(const char *dirname);
  bool MakeDirCheck(const char *dirname, bool check);
	bool GetHomeDir(char *path, const char *appname);
	long DirEnumSize(const char *dirname);
	long DirEnumList(const char *dirname, long istart, long icount, TDirEntry *list);
	bool DirectoryList(const char *dirname, const char *ext, TFileList *lfiles);
};

#endif // _WIZDISK_H
