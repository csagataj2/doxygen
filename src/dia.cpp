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

filepathtype::filepathtype(char* c): path(c)
{
    if(!valid_path(c))
        throw std::invalid_argument("Invalid filepath");
}

filepathtype::filepathtype(QCString s): path(s) {}

bool filepathtype::valid_path(char* c)
{
    return true;
}

QCString operator+ (const QCString & s, const filepathtype & f)
{
    return QCString(s + (QCString)f);
}

QCString operator+ (const filepathtype & f, const char* s)
{
    return QCString((QCString)f + s);
}

FTextStream& operator<<(FTextStream& o, const filepathtype& f)
{
    return o<<(const char*)f;
}


static const int maxCmdLine = 40960;

void writeDiaGraphFromFile(filepathtype inFile,filepathtype outDir,
                           filepathtype outFile,DiaOutputFormat format)
{
  QCString absOutFile = (QCString) outDir;
  absOutFile+=Portable::pathSeparator();
  absOutFile+=(QCString) outFile;

  // chdir to the output dir, so dot can find the font file.
  QCString oldDir = QDir::currentDirPath().utf8();
  // go to the html output directory (i.e. path)
  QDir::setCurrent((QString) outDir);
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
  diaArgs+=(QCString) outFile;
  diaArgs+=extension+"\"";

  diaArgs+=" \"";
  diaArgs+=(QCString) inFile;
  diaArgs+="\"";

  int exitCode;
  //printf("*** running: %s %s outDir:%s %s\n",diaExe.data(),diaArgs.data(),outDir,outFile);
  Portable::sysTimerStart();
  if ((exitCode=Portable::system(diaExe,diaArgs,FALSE))!=0)
  {
    err("Problems running %s. Check your installation or look typos in you dia file %s\n",
        diaExe.data(),(const char*)inFile);
    Portable::sysTimerStop();
    goto error;
  }
  Portable::sysTimerStop();
  if ( (format==DIA_EPS) && (Config_getBool(USE_PDFLATEX)) )
  {
    QCString epstopdfArgs(maxCmdLine);
    epstopdfArgs.sprintf("\"%s.eps\" --outfile=\"%s.pdf\"",
                         (const char*)outFile,(const char*)outFile);
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

