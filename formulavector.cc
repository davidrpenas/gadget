#include "formulavector.h"

#ifndef INLINE_VECTORS
#include "formulavector.icc"
#endif

Formulavector::Formulavector(const Formulavector& initial) {
  int i;
  size = initial.size;
  if (size > 0) {
    v = new Formula[size];
    for (i = 0; i < size; i++)
      v[i] = initial.v[i];
  } else
    v = 0;
}

void Formulavector::resize(int addsize, Keeper* keeper) {
  int i;
  if (v == 0) {
    size = addsize;
    v = new Formula[size];
  } else if (addsize > 0) {
    Formula* vnew = new Formula[addsize + size];
    for (i = 0; i < size; i++)
      v[i].Interchange(vnew[i], keeper);
    delete[] v;
    v = vnew;
    size += addsize;
  }
}

void Formulavector::Inform(Keeper* keeper) {
  int i;
  for (i = 0; i < size; i++) {
    ostrstream ostr;
    //AJ 26.10.00 Changing because output not the same when use
    //ReadDoubleWithSwitches and infile << formula and then formula.Inform(keeper)
    //unless use i instead of i + 1. Do not know which is correct...
    //ostr << i + 1 << ends;
    ostr << i << ends;
    keeper->AddString(ostr.str());
    v[i].Inform(keeper);
    keeper->ClearLast();
  }
}

CommentStream& operator >> (CommentStream& infile, Formulavector& Fvec) {
  if (infile.fail()) {
    infile.makebad();
    return infile;
  }
  int i;
  for (i = 0; i < Fvec.size; i++) {
    if (!(infile >> Fvec[i])) {
      infile.makebad();
      return infile;
    }
  }
  return infile;
}
