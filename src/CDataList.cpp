#include <CDataList.h>
#include <CStrUtil.h>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <cmath>
#include <vector>

void usage(int rc=1) {
  std::cerr << "Usage: CDataList " <<
               "[-h] [-s[dfiscCb]] [-n] [-t] [-o <off>] [-l <len> ] [--|<filename>]" << std::endl;
  exit(rc);
}

int
main(int argc, char **argv)
{
  if (argc < 2)
    usage(1);

  CDataList dataList;

  std::string filename;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      if      (strcmp(argv[i], "-n") == 0)
        dataList.setNumber(true);
      else if (strncmp(argv[i], "-s", 2) == 0) {
        std::string str(&argv[i][2]);

        std::vector<std::string> fields;

        CStrUtil::addFields(str, fields, ":");

        for (const auto &f : fields) {
           int show1 = 0;

           int len = f.size();

           for (auto j = 0; j < len; ++j) {
             switch (f[j]) {
               case 'd': show1 |= int(CDataList::Show::Double ); break;
               case 'f': show1 |= int(CDataList::Show::Float  ); break;
               case 'i': show1 |= int(CDataList::Show::Integer); break;
               case 's': show1 |= int(CDataList::Show::Short  ); break;
               case 'c': show1 |= int(CDataList::Show::Char   ); break;
               case 'C': show1 |= int(CDataList::Show::Chars  ); break;
               case 'b': show1 |= int(CDataList::Show::Byte   ); break;
               default :                                         break;
             }
           }

           if (show1 == 0)
             show1 = int(CDataList::Show::All);

           dataList.addShow(show1);
         }
      }
      else if (strcmp(argv[i], "-o") == 0) {
        if (i < argc - 1)
          dataList.setOffset(atoi(argv[++i]));
      }
      else if (strcmp(argv[i], "-l") == 0) {
        if (i < argc - 1)
          dataList.setLength(atoi(argv[++i]));
      }
      else if (strcmp(argv[i], "-t") == 0) {
        dataList.setTitle(true);
      }
      else if (strcmp(argv[i], "-r") == 0) {
        dataList.setRepeat(true);
      }
      else if (strcmp(argv[i], "-j") == 0) {
        dataList.setJoin(true);
      }
      else if (strcmp(argv[i], "-w") == 0) {
        if (i < argc - 1)
          dataList.setWidth(atoi(argv[++i]));
      }
      else if (strcmp(argv[i], "-h") == 0) {
        usage(0);
      }
      else if (strncmp(argv[i], "--", 2) == 0) {
        filename = "-";
      }
      else {
        std::cerr << "Invalid Option '" << argv[i] << "'" << std::endl;
        usage(1);
      }
    }
    else if (filename == "") {
      filename = argv[i];
    }
    else {
      std::cerr << "Invalid Argument '" << argv[i] << "'" << std::endl;
      usage(1);
    }
  }

  if (filename == "") {
    std::cerr << "No files specified" << std::endl;
    usage(1);
  }

  //---

  if (! dataList.init(filename))
    exit(1);

  //---

  dataList.showData();

  exit(0);
}

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
      std::cerr << "Can't open file '" << filename << "'" << std::endl;
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

    int show = showSet_[showSet_.size() - 1];

    showAll(show);
  }

  if (isSingleShow(Show::Char)) {
    if (n_ > 0 && ((n_ % width()) != 0))
      printf("\n");
  }
}

void
CDataList::
showTitle()
{
  // display title
  if (isNumber()) printf("        ");

  if (! isSingleShow(Show::Double ) && hasShowDouble ()) printf("     Double      " );
  if (! isSingleShow(Show::Float  ) && hasShowFloat  ()) printf("      Float      " );
  if (! isSingleShow(Show::Integer) && hasShowInteger()) printf("    Integer      " );

  if (! isSingleShow(Show::Short) && hasShowShort()) {
    if (size_ >= 4)
      printf("  Short1   Short2 ");
    else
      printf("  Short  ");
  }

  if (! isSingleShow(Show::Byte) && hasShowByte()) {
    if      (size_ >= 4)
      printf(" B1  B2  B3  B4 ");
    else if (size_ >= 2)
      printf(" B1  B2 ");
    else
      printf(" B1 ");
  }

  if (! isSingleShow(Show::Char) && hasShowChar()) {
    if      (size_ >= 4)
      printf(" C1  C2  C3  C4 ");
    else if (size_ >= 2)
      printf(" C1  C2 ");
    else
      printf(" C1 ");
  }

  if (! isSingleShow(Show::Chars) && hasShowChars()) printf("Char ");

  if (! isSingleShow(Show::Double)  && ! isSingleShow(Show::Float) &&
      ! isSingleShow(Show::Integer) && ! isSingleShow(Show::Short) &&
      ! isSingleShow(Show::Byte)    && ! isSingleShow(Show::Char ) &&
      ! isSingleShow(Show::Chars))
    printf("\n");

  if (isNumber()) printf("        ");

  if (! isSingleShow(Show::Double)  && hasShowDouble ()) printf("---------------- " );
  if (! isSingleShow(Show::Float)   && hasShowFloat  ()) printf("---------------- " );
  if (! isSingleShow(Show::Integer) && hasShowInteger()) printf("---------------- " );

  if (! isSingleShow(Show::Short) && hasShowShort()) {
    if (size_ >= 4)
      printf("  ------   ------ ");
    else
      printf("  ------ ");
  }

  if (! isSingleShow(Show::Byte) && hasShowByte()) {
    if      (size_ >= 4)
      printf(" --  --  --  -- ");
    else if (size_ >= 2)
      printf(" --  -- ");
    else
      printf(" -- ");
  }

  if (! isSingleShow(Show::Char) && hasShowChar()) {
    if      (size_ >= 4)
      printf(" --  --  --  -- ");
    else if (size_ >= 2)
      printf(" --  -- ");
    else
      printf(" -- ");
  }

  if (! isSingleShow(Show::Chars) && hasShowChars()) printf("---- ");

  if (! isSingleShow(Show::Double)  && ! isSingleShow(Show::Float) &&
      ! isSingleShow(Show::Integer) && ! isSingleShow(Show::Short) &&
      ! isSingleShow(Show::Byte)    && ! isSingleShow(Show::Char ) &&
      ! isSingleShow(Show::Chars))
    printf("\n\n");
}

bool
CDataList::
showSet()
{
  int ns = (isRepeat() ? showSet_.size() : showSet_.size() - 1);

  for (int i = 0; i < ns; ++i) {
    uint show = showSet_[i];

    int doffset = showSize(show);

    if (! showOne(show))
      return false;

    offset_ += doffset;

    if (length_ > 0) {
      --length_;

      if (length_ == 0)
        break;
    }
  }

  if (isJoin())
    printf("\n");

  return true;
}

bool
CDataList::
showOne(int show)
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
showAll(int show)
{
  if (offset_ > 0 && fp_ != stdin)
    fseek(fp_, offset_, SEEK_SET);

  while (readData(show)) {
    int l = (length_ > 0 ? length_ - n_ : INT_MAX);

    printData(show, l, true);

    ++n_;

    if (length_ > 0 && n_ > length_)
      break;
  }
}

bool
CDataList::
readData(int show)
{
  size_t size = showSize(show);

  uchar data[8];

  int num = fread(data, size, 1, fp_);

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
    if (isNumber()) printf("%6d: ", n_);

    if (show & uint(Show::Double )) printDouble(show);
    if (show & uint(Show::Float  )) printFloat(show, 0);
    if (show & uint(Show::Integer)) printInteger(show, 0);
    if (show & uint(Show::Short  )) printShort(show, 0, 2);
    if (show & uint(Show::Byte   )) printBytes(show, 0, 4);
    if (show & uint(Show::Char   )) printChar (show, 0, 4);
    if (show & uint(Show::Chars  )) printChars(show, 0, 4);

    if (newline)
      printf("\n");

    //---

    if (show != uint(Show::Double)) {
      if (isNumber()) printf("        ");

      if (show & uint(Show::Double )) printf("................ ");
      if (show & uint(Show::Float  )) printFloat(show, 1);
      if (show & uint(Show::Integer)) printInteger(show, 1);
      if (show & uint(Show::Short  )) printShort(show, 1, 2);
      if (show & uint(Show::Byte   )) printBytes(show, 1, 4);
      if (show & uint(Show::Char   )) printChar (show, 1, 4);
      if (show & uint(Show::Chars  )) printChars(show, 1, 4);

      if (newline)
        printf("\n");
    }
  }
  else if (size >= 4) {
    if (isNumber()) printf("%6d: ", n_);

    if (show & uint(Show::Float  )) printFloat(show, 0);
    if (show & uint(Show::Integer)) printInteger(show, 0);
    if (show & uint(Show::Short  )) printShort(show, 0, 2);
    if (show & uint(Show::Byte   )) printBytes(show, 0, 4);
    if (show & uint(Show::Char   )) printChar (show, 0, 4);
    if (show & uint(Show::Chars  )) printChars(show, 0, 4);

    if (newline)
      printf("\n");
  }
  else if (size >= 2) {
    if (isNumber()) printf("%6d: ", n_);

    if (show & uint(Show::Short)) printShort(show, 0, 1);
    if (show & uint(Show::Byte )) printBytes(show, 0, 2);
    if (show & uint(Show::Char )) printChar (show, 0, 2);
    if (show & uint(Show::Chars)) printChars(show, 0, 2);

    if (newline)
      printf("\n");
  }
  else if (size >= 1) {
    if (show != uint(Show::Char)) {
      if (isNumber()) printf("%6d: ", n_);

      if (show & uint(Show::Byte )) printBytes(show, 0, 1);
      if (show & uint(Show::Char )) printChar (show, 0, 1);
      if (show & uint(Show::Chars)) printChars(show, 0, 1);

      if (newline)
        printf("\n");
    }
    else {
      printf("%c", encodeChar(cword_[0]));

      if (n_ > 0 && ((n_ % width()) == 0))
        printf("\n");
    }
  }
}

void
CDataList::
printDouble(uint show)
{
  if      (isnan(dword_[0])) {
    if (show != uint(Show::Double))
      printf("NaN              ");
    else
      printf("NaN ");
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

      printf("%s ", dstring);
    }
    else {
      if (show != uint(Show::Double))
        printf("%16.5lg ", dword_[0]);
      else
        printf("%.5lg ", dword_[0]);
    }
  }
  else {
    if (show != uint(Show::Double))
      printf("................ ");
    else
      printf(".... ");
  }
}

void
CDataList::
printFloat(uint show, int i)
{
  if      (isnanf(fword_[i])) {
    if (show != uint(Show::Float))
      printf("NaN             ");
    else
      printf("NaN ");
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

    printf("%s ", fstring);
  }
  else {
    if (show != uint(Show::Float))
      printf("................ ");
   else
      printf(".... ");
  }
}

void
CDataList::
printInteger(uint show, int i)
{
  if (show != uint(Show::Integer))
    printf("%16d ", iword_[i]);
  else
    printf("%d ", iword_[i]);
}

void
CDataList::
printShort(uint show, int i, int n)
{
  if (n == 2) {
    if (show != uint(Show::Short))
      printf("%8d %8d ", hword_[2*i + 0], hword_[2*i + 1]);
    else {
      printf("%d ", hword_[2*i + 0]);
      printf("%d ", hword_[2*i + 1]);
    }
  }
  else {
    if (show != uint(Show::Short))
      printf("%8d ", hword_[2*i + 0]);
    else
      printf("%d ", hword_[2*i + 0]);
  }
}

void
CDataList::
printBytes(uint show, int i, int n)
{
  if      (n == 4) {
    if (show != uint(Show::Byte))
      printf("%3d %3d %3d %3d ", cword_[2*i + 0], cword_[2*i + 1],
             cword_[2*i + 2], cword_[2*i + 3]);
    else {
      printf("%d", cword_[2*i + 0]);
      printf("%d", cword_[2*i + 1]);
      printf("%d", cword_[2*i + 2]);
      printf("%d", cword_[2*i + 3]);
    }
  }
  else if (n == 2) {
    if (show != uint(Show::Byte))
      printf("%3d %3d ", cword_[2*i + 0], cword_[2*i + 1]);
    else {
      printf("%d ", cword_[2*i + 0]);
      printf("%d ", cword_[2*i + 1]);
    }
  }
  else if (n == 1) {
    if (show != uint(Show::Byte))
      printf("%3d ", cword_[2*i + 0]);
    else
      printf("%d ", cword_[2*i + 0]);
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
      cstring[j] = cword_[k];
    else
      cstring[j] = '.';
  }

  cstring[j] = '\0';

  printf("%s ", cstring);
}

void
CDataList::
printChar(uint show, int i, int n)
{
  if      (n == 4) {
    if (show != uint(Show::Char))
      printf("%3c %3c %3c %3c ", encodeChar(cword_[2*i + 0]), encodeChar(cword_[2*i + 1]),
             encodeChar(cword_[2*i + 2]), encodeChar(cword_[2*i + 3]));
    else
      printf("%c%c%c%c ", encodeChar(cword_[2*i + 0]), encodeChar(cword_[2*i + 1]),
             encodeChar(cword_[2*i + 2]), encodeChar(cword_[2*i + 3]));
  }
  else if (n == 2) {
    if (show != uint(Show::Char))
      printf("%3c %3c ", encodeChar(cword_[2*i + 0]), encodeChar(cword_[2*i + 1]));
    else
      printf("%c%c ", encodeChar(cword_[2*i + 0]), encodeChar(cword_[2*i + 1]));
  }
  else if (n == 1) {
    if (show != uint(Show::Char))
      printf("%3c ", encodeChar(cword_[2*i + 0]));
    else
      printf("%c ", encodeChar(cword_[2*i + 0]));
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
