/*

 SD - a slightly more friendly wrapper for sdfatlib

 This library aims to expose a subset of SD card functionality
 in the form of a higher level "wrapper" object.

 License: GNU General Public License V3
          (Because sdfatlib is licensed with this.)

 (C) Copyright 2010 SparkFun Electronics

 */

#ifndef __SD_H__
#define __SD_H__

#include <Arduino.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>

#define SD_CHIP_SELECT_PIN      0

#define FILE_READ O_RDONLY
#define FILE_WRITE (O_RDWR | O_CREAT)

class File : public Stream {
 private:
  char *_name; // our name
  FILE *_file;
  DIR *_dir;

public:
  File(const char *name, uint8_t mode = O_RDONLY)
  {
    const char *marg = (mode & O_ACCMODE) == O_RDONLY ? "r" : "w";
    struct stat st;
    int err;

    _name = name ? strdup(name) : NULL;
    if (_name == NULL) {
      _dir = NULL;
      _file = NULL;
      return;
    }

    err = stat(_name, &st);
    if (err < 0) {
      free(_name);
      _name = NULL;
      _dir = NULL;
      _file = NULL;
      return;
    }

    if (S_ISDIR(st.st_mode)) {
      _dir = opendir(_name);
      _file = NULL;
    } else {
      _file = fopen(_name, marg);
      _dir  = NULL;
    }
  }

  File(void)
  {
    _name = NULL;
    _file = NULL;
    _dir  = NULL;
  }
  ~File(void)
  {
  }
  virtual size_t write(uint8_t c)
  {
    return fputc(c, _file);
  }
  virtual size_t write(const uint8_t *buf, size_t size)
  {
    return fwrite(buf, sizeof(uint8_t), size, _file);
  }
  virtual int read()
  {
    return fgetc(_file);
  }
  virtual int peek()
  {
    int c = fgetc(_file);
    ungetc(c, _file);
    return c;
  }
  virtual int available()
  {
    return _file ? !feof(_file) : 0;
  }
  virtual void flush()
  {
    fflush(_file);
  }
  int read(void *buf, uint16_t nbyte)
  {
    return fread(buf, sizeof(uint8_t), nbyte, _file);
  }
  boolean seek(uint32_t pos)
  {
    int err;
    err = fseek(_file, pos, SEEK_SET);
    return (err < 0) ? false : true;
  }
  uint32_t position()
  {
    return ftell(_file);
  }
  uint32_t size()
  {
    struct stat st;
    fstat(fileno(_file), &st);
    return st.st_size;
  }
  void close()
  {
    if (_name)
      free(_name);
    if (_file) {
      fclose(_file);
      _file = NULL;
    }
    if (_dir) {
      closedir(_dir);
      _dir = NULL;
    }
  }
  operator bool()
  {
    return (_file == NULL && _dir == NULL) ? false : true;
  }
  char * name()
  {
    char *cp = strrchr(_name, '/');
    return cp ? (cp+1) : _name;
  }

  boolean isDirectory(void)
  {
    return _dir ? true : false;
  }

  File openNextFile(uint8_t mode = O_RDONLY)
  {
    char path[PATH_MAX];
    struct dirent *de;

    if (!_dir)
      return File();

    de = readdir(_dir);
    if (!de)
      return File();

    snprintf(path, sizeof(path), "%s/%s", _name, de->d_name);
    path[sizeof(path)-1] = 0;

    return File(path, mode);
  }

  void rewindDirectory(void)
  {
    rewinddir(_dir);
  }
  
  using Print::write;
};

class SDClass {

public:
  // This needs to be called to set up the connection to the SD card
  // before other methods are used.
  boolean begin(uint8_t csPin = SD_CHIP_SELECT_PIN)
  {
    return true;
  }
  
  // Open the specified file/directory with the supplied mode (e.g. read or
  // write, etc). Returns a File object for interacting with the file.
  // Note that currently only one file can be open at a time.
  File open(const char *filename, uint8_t mode = FILE_READ)
  {
    return File(filename, mode);
  }


  // Methods to determine if the requested file path exists.
  boolean exists(char *filepath)
  {
    struct stat st;

    return (stat(filepath, &st) < 0) ? false : true;
  }

  // Create the requested directory heirarchy--if intermediate directories
  // do not exist they will be created.
  boolean mkdir(char *filepath)
  {
    return (::mkdir(filepath, 0755) < 0) ? false : true;
  }
  
  // Delete the file.
  boolean remove(char *filepath)
  {
    return (::unlink(filepath) < 0) ? false : true;
  }
  
  boolean rmdir(char *filepath)
  {
    return (::rmdir(filepath) < 0) ? false : true;
  }
};

extern SDClass SD;

/* vim: set shiftwidth=2 expandtab: */
#endif
