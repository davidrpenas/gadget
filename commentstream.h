#ifndef commentstream_h
#define commentstream_h

#include "gadget.h"

const char COMMENTCHAR = ';';
extern void KillComments(istream* const istrptr);

class CommentStream;
typedef CommentStream& (*__commentmanip)(CommentStream&);

class Whitespace {
public:
  Whitespace() { v = 0; };
  friend istream& operator >> (istream& istr, Whitespace& ws);
private:
  int v;
};

class CommentStream {
public:
  friend CommentStream& ws(CommentStream& ins);
  CommentStream();
  void SetStream(istream& istr)  { istrptr = &istr; };
  CommentStream(istream& istr) { istrptr = &istr; };
  CommentStream& operator >> (int& a) {
    KillComments(istrptr);
    istrptr->operator >> (a);
    return *this;
  };
  CommentStream& operator >> (char* a) {
    KillComments(istrptr);
    istrptr->operator >> (a);
    return *this;
  };
  CommentStream& operator >> (char& a) {
    KillComments(istrptr);
    istrptr->operator >> (a);
    return *this;
  };
  CommentStream& operator >> (double& a) {
    KillComments(istrptr);
    istrptr->operator >> (a);
    return *this;
  };
  CommentStream& operator >> (__commentmanip func) {
    (*func)(*this);
    return *this;
  };
  int peek() { return(istrptr->peek() == COMMENTCHAR ? '\n':istrptr->peek()); };
  int eof() { return istrptr->eof(); };
  int fail() { return istrptr->fail(); };
  int bad() { return istrptr->bad(); };
  int good() { return istrptr->good(); };
  //void clear(int state = 0) { istrptr->clear(state); };
  void get(char* text, int length, char sep) { istrptr->get(text, length, sep); };
  CommentStream& seekg(streampos Pos) {
    istrptr->seekg(Pos);
    return *this;
  };
  streampos tellg() { return istrptr->tellg(); };
  int operator !() { return istrptr->fail(); };
  void makebad() { istrptr->clear(ios::badbit|istrptr->rdstate()); };
  CommentStream& get(char& c);
  CommentStream& getline(char* ptr, int len, char delim = '\n');
protected:
  istream* istrptr;
};

#endif
