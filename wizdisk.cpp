//////////////////////////////////////////////////////////////////////////
// CWizDisk: Disk and File management library
// creada en clase y optimizada para windows y wiz
//////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include "wizdisk.h"

//////////////////////////////////////////////////////////
// TFileInfo, nombre de fichero y su información

TFileInfo::TFileInfo(): attr()
{
   name="";
}

TString TFileInfo::DisplayName() const
{
  if(attr.type == 'd')
    return TString("<") + name + ">";
  else if(attr.type == 's')
    return name + "$";
  else // 'a' y '?'
    return name;
}

//////////////////////////////////////////////////////////
// funciones de disco

CWizDisk::CWizDisk()
{
}

// path del directorio actual
void CWizDisk::GetCurrPath(char *path, bool sep)
{
  getcwd(path, MAX_PATH);
  if(sep) strcat(path, SDIR_SEP);
}

// devuelve true si el directorio existe
bool CWizDisk::DirExists(const char *dirname)
{
	if(!dirname || !*dirname)
		return false;

	if(access(dirname,0) != 0)
		return false;
	return true;
}

// indica si es un directorio raíz
bool CWizDisk::IsRootDir(const char *dirname)
{
#ifndef WIN32
	bool inroot = !strcmp(dirname,"/mnt") || !strcmp(dirname,"/tmp/mnt");
#else
	bool inroot = !strcmp(dirname,"c:\\mios");
#endif
  return inroot;
}

// crear el directorio dado si no existe
bool CWizDisk::MakeDirCheck(const char *dirname, bool check)
{
  if(check && DirExists(dirname))
    return true;
#ifdef WIN32
  int ret = mkdir(dirname);
#else
  // lectura y escritura para el usuario
  int ret = mkdir(dirname, S_IRUSR | S_IWUSR);
#endif
  return (ret==0);
}

// path de configuración de la app, para escribir
bool CWizDisk::GetHomeDir(char *path, const char *appname)
{
  if(!path || !appname)
    return false;
#ifdef WIN32
  sprintf(path, "%s\\%s", getenv("HOMEPATH"), appname);
#else
  sprintf(path, "%s/%s", getenv("HOME"), appname);
#endif
  return true;
}

// da el número de elementos de un diretorio. -1=error
long CWizDisk::DirEnumSize(const char *dirname)
{
	if(!dirname)
		return -1;

	long num = 0;
  DIR *dir = opendir(dirname);
  if(dir) {
    while(readdir(dir)) num++;
    closedir(dir);
  }
  // ignorar el "."
	return num-1;
}

// da una lista con los elementos de un directorio
// desde la posición de archivo idstart da idcount elementos
long CWizDisk::DirEnumList(const char *dirname, long istart, long icount, TDirEntry *list)
{
	if(!dirname || !icount || !list)
		return -1;

  long iend = istart + icount - 1;
  long index = 0, leidos = 0;
  DIR *dir = opendir(dirname);

  if(dir) {
    struct dirent *de = readdir(dir);
    while(de) {
      if(!strcmp(de->d_name,".")) { // ignorar '.'
        de = readdir(dir); // leer sig
        continue;
      }
      if(index>=istart && index<iend) {
        list->name = de->d_name;
        leidos++;
        list++; // siguiente elem
      }
      index++;
      de = readdir(dir);
    }
    closedir(dir);
  }
	return leidos;
}

// da los atributos de un fichero
bool CWizDisk::FileAttr(const char *name, TFileAttr *attr)
{
	if(!name || !attr)
		return false;

  // implementación mejor
	memset(attr, 0, sizeof(TFileAttr));
  struct stat st;
  if(stat(name,&st) == -1)
    return false;

	if(S_ISREG(st.st_mode))
     attr->type = 'a'; // archivo
  else if(S_ISDIR(st.st_mode))
     attr->type = 'd'; // directorio
  else
     attr->type = 's'; // sys+hidden

  // tamaño del archivo
  attr->size = (long)st.st_size;

	// fecha de modificación
	attr->mtime = st.st_mtime;
  return true;
}

// lista de ficheros del directorio con atributos, .ext opcional
bool CWizDisk::DirectoryList(const char *dirname, const char *ext, TFileList *lfiles)
{
  if(!dirname || !lfiles)
    return false;
  TFileList ldirs; // directorios
  ldirs.clear();

#ifdef WIN32
  // evita que falle en el raiz "u:"
  TString fixdir = TString(dirname) + SDIR_SEP;
  dirname = fixdir.c_str();
#endif

  // rellenar los nombres
  lfiles->clear();
  DIR *dir = opendir(dirname);

  if(dir) {
    struct dirent *de = readdir(dir);
    while(de) {
      if(!strcmp(de->d_name,".")) { // ignorar '.'
        de = readdir(dir); // leer sig
        continue;
      }
      TFileInfo info;
      info.name = de->d_name;

      TString path(dirname);
      path += TString(SDIR_SEP) + info.name;
      if(!FileAttr(path.c_str(), &info.attr))
        info.attr.type = '?';
      else if(info.attr.type == 'd') // dir
        ldirs.push_back(info);
      else {
        int len = path.length();
        int len2 = ext? strlen(ext): 0;
        // añadir a la lista si terminan en ext, opcional
        if(len2 && len2<=len && !strncmp(&path[len-len2], ext, len2))
          lfiles->push_back(info);
      }
      de = readdir(dir);
    }
    closedir(dir);
  }

  // añadir los directorios al principio
  lfiles->insert(lfiles->begin(), ldirs.begin(), ldirs.end());
  ldirs.clear();
  return true;
}

// da el tamaño de un fichero
bool CWizDisk::FileGetSize(const char *filename, long *size)
{
	if(!filename || !size)
		return false;

  // unix no tiene _filelength ni _tell
  FILE *file = fopen(filename,"rb");
  if(file == NULL)
		return false;

  long oldpos = ftell(file);
  fseek(file,0,SEEK_END);
  long lsize = ftell(file);
  fseek(file,oldpos,SEEK_SET);
  fclose(file);

	if(lsize == -1)
		return false;
	*size = lsize;
	return true;
}
