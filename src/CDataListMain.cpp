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
    "[-h] [-s[dfiscCb]] [-n] [-r] [-j] [-w <width>] [-t] [-o <off>] [-l <len>] [--|<filename>]\n" <<
    " -sd : double\n" <<
    " -sf : float\n" <<
    " -si : integer\n" <<
    " -ss : short\n" <<
    " -sc : char\n" <<
    " -sC : chars\n" <<
    " -sb : byte\n" <<
    " -n  : number\n" <<
    " -r  : repeat\n" <<
    " -j  : join\n" <<
    " -S  : stream\n" <<
    " -w  : width\n" <<
    " -t  : title\n";
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
           uint show1 = 0;

           auto len = f.size();

           for (uint j = 0; j < len; ++j) {
             switch (f[j]) {
               case 'd': show1 |= uint(CDataList::Show::Double ); break;
               case 'f': show1 |= uint(CDataList::Show::Float  ); break;
               case 'i': show1 |= uint(CDataList::Show::Integer); break;
               case 's': show1 |= uint(CDataList::Show::Short  ); break;
               case 'c': show1 |= uint(CDataList::Show::Char   ); break;
               case 'C': show1 |= uint(CDataList::Show::Chars  ); break;
               case 'b': show1 |= uint(CDataList::Show::Byte   ); break;
               default :                                          break;
             }
           }

           if (show1 == 0)
             show1 = uint(CDataList::Show::All);

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
      else if (strcmp(argv[i], "-S") == 0) {
        dataList.setStream(true);
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
        std::cerr << "Invalid Option '" << argv[i] << "'\n";
        usage(1);
      }
    }
    else if (filename == "") {
      filename = argv[i];
    }
    else {
      std::cerr << "Invalid Argument '" << argv[i] << "'\n";
      usage(1);
    }
  }

  if (filename == "") {
    std::cerr << "No files specified\n";
    usage(1);
  }

  //---

  if (! dataList.init(filename))
    exit(1);

  //---

  dataList.showData();

  exit(0);
}
