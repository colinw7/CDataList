#ifndef CDataList_H
#define CDataList_H

#include <string>
#include <vector>

typedef unsigned char uchar;

class CDataList {
 public:
  enum class Show {
    Double  = (1<<0),
    Float   = (1<<1),
    Integer = (1<<2),
    Short   = (1<<3),
    Char    = (1<<4),
    Chars   = (1<<5),
    Byte    = (1<<6),
    All     = (Double|Float|Integer|Short|Chars)
  };

 public:
  CDataList();
 ~CDataList();

  bool isNumber() const { return number_; }
  void setNumber(bool b) { number_ = b; }

  bool isTitle() const { return title_; }
  void setTitle(bool b) { title_ = b; }

  bool isRepeat() const { return repeat_; }
  void setRepeat(bool b) { repeat_ = b; }

  bool isJoin() const { return join_; }
  void setJoin(bool b) { join_ = b; }

  int length() const { return length_; }
  void setLength(int i) { length_ = i; }

  int offset() const { return offset_; }
  void setOffset(int i) { offset_ = i; }

  int width() const { return width_; }
  void setWidth(int i) { width_ = i; }

  void addShow(uint show) {
    showOr_  |= show;
    showAnd_ &= show;

    showSet_.push_back(show);
  }

  bool isSingleShow(Show show) { return showOr_ == uint(show); }
  bool hasShow(Show show) { return showOr_ & uint(show); }

  bool hasShowDouble () { return showOr_ & uint(Show::Double ); }
  bool hasShowFloat  () { return showOr_ & uint(Show::Float  ); }
  bool hasShowInteger() { return showOr_ & uint(Show::Integer); }
  bool hasShowShort  () { return showOr_ & uint(Show::Short  ); }
  bool hasShowChar   () { return showOr_ & uint(Show::Char   ); }
  bool hasShowChars  () { return showOr_ & uint(Show::Chars  ); }
  bool hasShowByte   () { return showOr_ & uint(Show::Byte   ); }

  bool init(const std::string &filename);

  void showData();

 private:
  void showTitle();

  bool showSet();

  bool showOne(uint show);
  void showAll(uint show);

  bool readData(uint show);

  void encodeData(uint show, const uchar *data);

  void printData(uint show, int length, bool newline);

  void printDouble (uint show);
  void printFloat  (uint show, int i);
  void printInteger(uint show, int i);
  void printShort  (uint show, int i, int n);
  void printBytes  (uint show, int i, int n);
  void printChars  (uint show, int i, int n);
  void printChar   (uint show, int i, int n);

  char encodeChar(char c);

  size_t showSize(uint show) const;

 private:
  FILE              *fp_      { nullptr };
  bool               number_  { false };
  bool               title_   { false };
  bool               repeat_  { false };
  bool               join_    { false };
  int                length_  { -1 };
  int                offset_  { 0 };
  int                width_   { 20 };
  size_t             size_    { 1 };
  uint               showOr_  { 0 };
  uint               showAnd_ { uint(Show::All) };
  std::vector<uint>  showSet_;
  int                n_       { 1 };
  double             dword_[1];
  float              fword_[2];
  int                iword_[2];
  short              hword_[4];
  uchar              cword_[8];
};

#endif
