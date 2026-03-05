#include <CDataList.h>
#include <CStrUtil.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <cmath>

CDataList::
CDataList()
{
}

CDataList::
~CDataList()
{
  if (fp_ && fp_ != stdin)
    fclose(fp_);
}

bool
CDataList::
init(const std::string &filename)
{
  if (filename != "-") {
    fp_ = fopen(filename.c_str(), "r");

    if (! fp_) {
      errorMsg("Can't open file '" + filename + "'");
      return false;
    }
  }
  else
    fp_ = stdin;

  //---

  if (showSet_.empty()) {
    showSet_.push_back(int(Show::All));

    showAnd_ = uint(Show::All);
    showOr_  = uint(Show::All);
  }

  return true;
}

void
CDataList::
showData()
{
  line_ = "";

  size_ = showSize(showOr_);

  //---

  n_ = 1;

  if (title_)
    showTitle();

  if (isRepeat()) {
    while (true) {
      if (! showSet())
        break;
    }
  }
  else {
    showSet();

    uint show = showSet_[showSet_.size() - 1];

    showAll(show);
  }

  if (isSingleShow(Show::Char)) {
    if (n_ > 0 && ((n_ % width()) != 0))
      flushLine();
  }

  flushLine();
}

void
CDataList::
showTitle()
{
  line_ = "";

  // display title
  if (isNumber())
    line_ += "        ";

  if (! isSingleShow(Show::Double ) && hasShowDouble())
    line_ += "     Double      ";
  if (! isSingleShow(Show::Float  ) && hasShowFloat())
    line_ += "      Float      ";
  if (! isSingleShow(Show::Integer) && hasShowInteger())
    line_ += "    Integer      ";

  if (! isSingleShow(Show::Short) && hasShowShort()) {
    if (size_ >= 4)
      line_ += "  Short1   Short2 ";
    else
      line_ += "  Short  ";
  }

  if (! isSingleShow(Show::Byte) && hasShowByte()) {
    if      (size_ >= 4)
      line_ += " B1  B2  B3  B4 ";
    else if (size_ >= 2)
      line_ += " B1  B2 ";
    else
      line_ += " B1 ";
  }

  if (! isSingleShow(Show::Char) && hasShowChar()) {
    if      (size_ >= 4)
      line_ += " C1  C2  C3  C4 ";
    else if (size_ >= 2)
      line_ += " C1  C2 ";
    else
      line_ += " C1 ";
  }

  if (! isSingleShow(Show::Chars) && hasShowChars())
    line_ += "Char ";

  if (line_ != "") {
    outputLine(line_);

    line_ = "";
  }

  if (isNumber())
    line_ += "        ";

  if (! isSingleShow(Show::Double)  && hasShowDouble ())
    line_ += "---------------- ";
  if (! isSingleShow(Show::Float)   && hasShowFloat  ())
    line_ += "---------------- ";
  if (! isSingleShow(Show::Integer) && hasShowInteger())
    line_ += "---------------- ";

  if (! isSingleShow(Show::Short) && hasShowShort()) {
    if (size_ >= 4)
      line_ += "  ------   ------ ";
    else
      line_ += "  ------ ";
  }

  if (! isSingleShow(Show::Byte) && hasShowByte()) {
    if      (size_ >= 4)
      line_ += " --  --  --  -- ";
    else if (size_ >= 2)
      line_ += " --  -- ";
    else
      line_ += " -- ";
  }

  if (! isSingleShow(Show::Char) && hasShowChar()) {
    if      (size_ >= 4)
      line_ += " --  --  --  -- ";
    else if (size_ >= 2)
      line_ += " --  -- ";
    else
      line_ += " -- ";
  }

  if (! isSingleShow(Show::Chars) && hasShowChars())
    line_ += "---- ";

  if (line_ != "") {
    outputLine(line_);

    line_ = "";

    outputLine(line_);
  }
}

bool
CDataList::
showSet()
{
  auto ns = uint(isRepeat() ? showSet_.size() : showSet_.size() - 1);

  for (uint i = 0; i < ns; ++i) {
    uint show = showSet_[i];

    auto doffset = showSize(show);

    if (! showOne(show))
      return false;

    offset_ += int(doffset);

    if (length_ > 0) {
      --length_;

      if (length_ == 0)
        break;
    }
  }

  if (isJoin())
    flushLine();

  return true;
}

bool
CDataList::
showOne(uint show)
{
  if (offset_ > 0 && fp_ != stdin)
    fseek(fp_, offset_, SEEK_SET);

  if (! readData(show))
    return false;

  printData(show, 1, ! isJoin());

  ++n_;

  return true;
}

void
CDataList::
showAll(uint show)
{
  if (offset_ > 0 && fp_ != stdin)
    fseek(fp_, offset_, SEEK_SET);

  while (readData(show)) {
    int l = (length_ > 0 ? length_ - n_ : INT_MAX);

    printData(show, l, ! isStream());

    ++n_;

    if (length_ > 0 && n_ > length_)
      break;
  }
}

bool
CDataList::
readData(uint show)
{
  size_t size = showSize(show);

  uchar data[8];

  auto num = fread(data, size, 1, fp_);

  if (num != 1)
    return false;

  encodeData(show, &data[0]);

  return true;
}

void
CDataList::
encodeData(uint show, const uchar *data)
{
  size_t size = showSize(show);

  if      (size >= 8) {
    memcpy(&dword_[0], data,   sizeof(double));
    memcpy(&fword_[0], data, 2*sizeof(float ));
    memcpy(&iword_[0], data, 2*sizeof(int   ));
    memcpy(&hword_[0], data, 4*sizeof(short ));
    memcpy(&cword_[0], data, 8*sizeof(char  ));
  }
  else if (size >= 4) {
    memcpy(&fword_[0], data,   sizeof(float ));
    memcpy(&iword_[0], data,   sizeof(int   ));
    memcpy(&hword_[0], data, 2*sizeof(short ));
    memcpy(&cword_[0], data, 4*sizeof(char  ));
  }
  else if (size >= 2) {
    memcpy(&hword_[0], data,   sizeof(short ));
    memcpy(&cword_[0], data, 2*sizeof(char  ));
  }
  else {
    memcpy(&cword_[0], data,   sizeof(char  ));
  }
}

void
CDataList::
printData(uint show, int /*length*/, bool newline)
{
  size_t size = showSize(show);

  if      (size >= 8) {
    if (isNumber())
      line_ += CStrUtil::strprintf("%6d: ", n_);

    if (show & uint(Show::Double )) printDouble(show);
    if (show & uint(Show::Float  )) printFloat(show, 0);
    if (show & uint(Show::Integer)) printInteger(show, 0);
    if (show & uint(Show::Short  )) printShort(show, 0, 2);
    if (show & uint(Show::Byte   )) printBytes(show, 0, 4);
    if (show & uint(Show::Char   )) printChar (show, 0, 4);
    if (show & uint(Show::Chars  )) printChars(show, 0, 4);

    if (newline)
      flushLine();

    //---

    if (show != uint(Show::Double)) {
      if (isNumber())
        line_ += "        ";

      if (show & uint(Show::Double )) line_ += "................ ";
      if (show & uint(Show::Float  )) printFloat(show, 1);
      if (show & uint(Show::Integer)) printInteger(show, 1);
      if (show & uint(Show::Short  )) printShort(show, 1, 2);
      if (show & uint(Show::Byte   )) printBytes(show, 1, 4);
      if (show & uint(Show::Char   )) printChar (show, 1, 4);
      if (show & uint(Show::Chars  )) printChars(show, 1, 4);

      if (newline)
        flushLine();
    }
  }
  else if (size >= 4) {
    if (isNumber())
      line_ += CStrUtil::strprintf("%6d: ", n_);

    if (show & uint(Show::Float  )) printFloat(show, 0);
    if (show & uint(Show::Integer)) printInteger(show, 0);
    if (show & uint(Show::Short  )) printShort(show, 0, 2);
    if (show & uint(Show::Byte   )) printBytes(show, 0, 4);
    if (show & uint(Show::Char   )) printChar (show, 0, 4);
    if (show & uint(Show::Chars  )) printChars(show, 0, 4);

    if (newline)
      flushLine();
  }
  else if (size >= 2) {
    if (isNumber())
      line_ += CStrUtil::strprintf("%6d: ", n_);

    if (show & uint(Show::Short)) printShort(show, 0, 1);
    if (show & uint(Show::Byte )) printBytes(show, 0, 2);
    if (show & uint(Show::Char )) printChar (show, 0, 2);
    if (show & uint(Show::Chars)) printChars(show, 0, 2);

    if (newline)
      flushLine();
  }
  else if (size >= 1) {
    if (show != uint(Show::Char)) {
      if (isNumber())
        line_ += CStrUtil::strprintf("%6d: ", n_);

      if (show & uint(Show::Byte )) printBytes(show, 0, 1);
      if (show & uint(Show::Char )) printChar (show, 0, 1);
      if (show & uint(Show::Chars)) printChars(show, 0, 1);

      if (newline)
        flushLine();
    }
    else {
      line_ += CStrUtil::strprintf("%c", encodeChar(char(cword_[0])));

      if (n_ > 0 && ((n_ % width()) == 0))
        flushLine();
    }
  }
}

void
CDataList::
printDouble(uint show)
{
  if      (std::isnan(dword_[0])) {
    if (show != uint(Show::Double))
      line_ += "NaN              ";
    else
      line_ += "NaN ";
  }
  else if (dword_[0] > -DBL_MAX && dword_[0] < DBL_MAX) {
    if (dword_[0] > -FLT_MAX && dword_[0] < FLT_MAX) {
      char dstring[128];

      if (show != uint(Show::Double)) {
        sprintf(dstring, "%16.5lf", dword_[0]);

        if (strlen(dstring) > 16)
          sprintf(dstring, "%16.5lg", dword_[0]);
      }
      else {
        sprintf(dstring, "%.5lf", dword_[0]);

        if (strlen(dstring) > 16)
          sprintf(dstring, "%.5lg", dword_[0]);
      }

      line_ += CStrUtil::strprintf("%s ", dstring);
    }
    else {
      if (show != uint(Show::Double))
        line_ += CStrUtil::strprintf("%16.5lg ", dword_[0]);
      else
        line_ += CStrUtil::strprintf("%.5lg ", dword_[0]);
    }
  }
  else {
    if (show != uint(Show::Double))
      line_ += "................ ";
    else
      line_ += ".... ";
  }
}

void
CDataList::
printFloat(uint show, int i)
{
  if      (isnanf(fword_[i])) {
    if (show != uint(Show::Float))
      line_ += "NaN             ";
    else
      line_ += "NaN ";
  }
  else if (fword_[i] > -FLT_MAX && fword_[i] < FLT_MAX) {
    char fstring[64];

    if (show != uint(Show::Float)) {
      sprintf(fstring, "%16.5f", fword_[i]);

      if (strlen(fstring) > 16)
        sprintf(fstring, "%16.5g", fword_[i]);
    }
    else {
      sprintf(fstring, "%.5f", fword_[i]);

      if (strlen(fstring) > 16)
        sprintf(fstring, "%.5g", fword_[i]);
    }

    line_ += CStrUtil::strprintf("%s ", fstring);
  }
  else {
    if (show != uint(Show::Float))
      line_ += "................ ";
   else
      line_ += ".... ";
  }
}

void
CDataList::
printInteger(uint show, int i)
{
  if (show != uint(Show::Integer))
    line_ += CStrUtil::strprintf("%16d ", iword_[i]);
  else
    line_ += CStrUtil::strprintf("%d ", iword_[i]);
}

void
CDataList::
printShort(uint show, int i, int n)
{
  if (n == 2) {
    if (show != uint(Show::Short))
      line_ += CStrUtil::strprintf("%8d %8d ", hword_[2*i + 0], hword_[2*i + 1]);
    else {
      line_ += CStrUtil::strprintf("%d ", hword_[2*i + 0]);
      line_ += CStrUtil::strprintf("%d ", hword_[2*i + 1]);
    }
  }
  else {
    if (show != uint(Show::Short))
      line_ += CStrUtil::strprintf("%8d ", hword_[2*i + 0]);
    else
      line_ += CStrUtil::strprintf("%d ", hword_[2*i + 0]);
  }
}

void
CDataList::
printBytes(uint show, int i, int n)
{
  if      (n == 4) {
    if (show != uint(Show::Byte))
      line_ += CStrUtil::strprintf("%3d %3d %3d %3d ",
                                   cword_[2*i + 0], cword_[2*i + 1],
                                   cword_[2*i + 2], cword_[2*i + 3]);
    else {
      line_ += CStrUtil::strprintf("%d", cword_[2*i + 0]);
      line_ += CStrUtil::strprintf("%d", cword_[2*i + 1]);
      line_ += CStrUtil::strprintf("%d", cword_[2*i + 2]);
      line_ += CStrUtil::strprintf("%d", cword_[2*i + 3]);
    }
  }
  else if (n == 2) {
    if (show != uint(Show::Byte))
      line_ += CStrUtil::strprintf("%3d %3d ", cword_[2*i + 0], cword_[2*i + 1]);
    else {
      line_ += CStrUtil::strprintf("%d ", cword_[2*i + 0]);
      line_ += CStrUtil::strprintf("%d ", cword_[2*i + 1]);
    }
  }
  else if (n == 1) {
    if (show != uint(Show::Byte))
      line_ += CStrUtil::strprintf("%3d ", cword_[2*i + 0]);
    else
      line_ += CStrUtil::strprintf("%d ", cword_[2*i + 0]);
  }
}

void
CDataList::
printChars(uint /*show*/, int i, int n)
{
  char cstring[32];

  int j = 0;

  for ( ; j < n; j++) {
    int k = i*n + j;

    if (isprint(cword_[k]) && ! iscntrl(cword_[k]))
      cstring[j] = char(cword_[k]);
    else
      cstring[j] = '.';
  }

  cstring[j] = '\0';

  line_ += CStrUtil::strprintf("%s ", cstring);
}

void
CDataList::
printChar(uint show, int i, int n)
{
  if      (n == 4) {
    if (show != uint(Show::Char))
      line_ += CStrUtil::strprintf("%3c %3c %3c %3c ",
                                   encodeChar(char(cword_[2*i + 0])),
                                   encodeChar(char(cword_[2*i + 1])),
                                   encodeChar(char(cword_[2*i + 2])),
                                   encodeChar(char(cword_[2*i + 3])));
    else
      line_ += CStrUtil::strprintf("%c%c%c%c ",
                                   encodeChar(char(cword_[2*i + 0])),
                                   encodeChar(char(cword_[2*i + 1])),
                                   encodeChar(char(cword_[2*i + 2])),
                                   encodeChar(char(cword_[2*i + 3])));
  }
  else if (n == 2) {
    if (show != uint(Show::Char))
      line_ += CStrUtil::strprintf("%3c %3c ",
                                   encodeChar(char(cword_[2*i + 0])),
                                   encodeChar(char(cword_[2*i + 1])));
    else
      line_ += CStrUtil::strprintf("%c%c ",
                                   encodeChar(char(cword_[2*i + 0])),
                                   encodeChar(char(cword_[2*i + 1])));
  }
  else if (n == 1) {
    if (show != uint(Show::Char))
      line_ += CStrUtil::strprintf("%3c ", encodeChar(char(cword_[2*i + 0])));
    else
      line_ += CStrUtil::strprintf("%c ", encodeChar(char(cword_[2*i + 0])));
  }
}

char
CDataList::
encodeChar(char c)
{
  if (isprint(c) && ! iscntrl(c))
    return c;

  return '.';
}

size_t
CDataList::
showSize(uint show) const
{
  if (show & uint(Show::Double))
    return 8;

  if ((show & uint(Show::Chars)) || (show & uint(Show::Integer)) || (show & uint(Show::Float)))
    return 4;

  if (show & uint(Show::Short))
    return 2;

  return 1;
}

void
CDataList::
flushLine()
{
  if (line_ != "") {
    std::cout << line_ <<  "\n";

    line_ = "";
  }
}

void
CDataList::
outputLine(const std::string &line) const
{
  std::cout << line << "\n";
}

void
CDataList::
errorMsg(const std::string &msg) const
{
  std::cerr << msg << "\n";
}
