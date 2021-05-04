/******************************************************************************
 *
 * 
 *
 * Copyright (C) 1997-2015 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include "dia.h"
#include "portable.h"
#include "config.h"
#include "message.h"
#include "util.h"

#include <qdir.h>


#include <stdexcept>

filepath::filepath(const char* c): path(c)
{
    if(!valid_path(c))
        throw std::invalid_argument("Invalid filepath");
}

filepath::filepath(const QCString s): path(s) {}

bool filepath::valid_path(const char* c)
{
    return true;
}

QCString operator+ (const QCString & s, const filepath & f)
{
    return QCString(s + static_cast<const QCString>(f));
}

QCString operator+ (const filepath & f, const char* s)
{
    return QCString(static_cast<const QCString>(f) + s);
}

FTextStream& operator<<(FTextStream& o, const filepath& f)
{
    return o<<static_cast<const char*>(f);
}

int filepath::findRev(char c, int index , bool cs ) const
{
  return path.findRev(c,index,cs);
}

uint filepath::length() const
{
  return path.length();
}

filepath filepath::right(uint len) const
{
  return filepath(static_cast<const QString>(path).right(len).local8Bit());
}

filepath filepath::left(uint len) const
{
  return filepath(static_cast<const QString>(path).left(len).local8Bit());
}

int	filepath::find( char c, int index, bool cs) const
{
  return path.find(c,index,cs);
}

filepath& filepath::prepend(const char *s)
{
  path.prepend(s);
  return *this;
}

static const int maxCmdLine = 40960;

void writeDiaGraphFromFile(filepath inFile,filepath outDir,
                           filepath outFile,DiaOutputFormat format)
{
  QCString absOutFile = static_cast<const QCString>(outDir);
  absOutFile+=Portable::pathSeparator();
  absOutFile+=static_cast<const QCString>(outFile);

  // chdir to the output dir, so dot can find the font file.
  QCString oldDir = QDir::currentDirPath().utf8();
  // go to the html output directory (i.e. path)
  QDir::setCurrent((const QString) outDir);
  //printf("Going to dir %s\n",QDir::currentDirPath().data());
  QCString diaExe = Config_getString(DIA_PATH)+"dia"+Portable::commandExtension();
  QCString diaArgs;
  QCString extension;
  diaArgs+="-n ";
  if (format==DIA_BITMAP)
  {
    diaArgs+="-t png-libart";
    extension=".png";
  }
  else if (format==DIA_EPS)
  {
    diaArgs+="-t eps";
    extension=".eps";
  }

  diaArgs+=" -e \"";
  diaArgs+=static_cast<const QCString>(outFile);
  diaArgs+=extension+"\"";

  diaArgs+=" \"";
  diaArgs+=static_cast<const QCString>(inFile);
  diaArgs+="\"";

  int exitCode;
  //printf("*** running: %s %s outDir:%s %s\n",diaExe.data(),diaArgs.data(),outDir,outFile);
  Portable::sysTimerStart();
  if ((exitCode=Portable::system(diaExe,diaArgs,FALSE))!=0)
  {
    err("Problems running %s. Check your installation or look typos in you dia file %s\n",
        diaExe.data(),static_cast<const char*>(inFile));
    Portable::sysTimerStop();
    goto error;
  }
  Portable::sysTimerStop();
  if ( (format==DIA_EPS) && (Config_getBool(USE_PDFLATEX)) )
  {
    QCString epstopdfArgs(maxCmdLine);
    epstopdfArgs.sprintf("\"%s.eps\" --outfile=\"%s.pdf\"",
                         static_cast<const char*>(outFile),static_cast<const char*>(outFile));
    Portable::sysTimerStart();
    if (Portable::system("epstopdf",epstopdfArgs)!=0)
    {
      err("Problems running epstopdf. Check your TeX installation!\n");
    }
    Portable::sysTimerStop();
  }

error:
  QDir::setCurrent(oldDir);
}

