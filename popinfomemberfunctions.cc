#include "popinfovector.h"
#include "popinfoindexvector.h"
#include "doubleindexvector.h"
#include "conversionindex.h"
#include "mathfunc.h"
#include "popinfo.h"
#include "gadget.h"

//Pre: CI maps from Number to 'this' and Number has finer or even resolution than 'this'.
void PopInfoVector::Sum(const PopInfoVector* const Number, const ConversionIndex& CI) {
  int i;
  for (i = 0; i < size; i++) {
    v[i].N = 0.0;
    v[i].W = 0.0;
  }
  for (i = CI.Minlength(); i < CI.Maxlength(); i++)
    this->operator[](CI.Pos(i)) += (*Number)[i];
}

void PopinfoAdd(PopInfoIndexVector& target, const PopInfoIndexVector& Addition,
  const ConversionIndex& CI, double ratio) {

  PopInfo pop;
  int l, minl, maxl, offset;

  if (CI.SameDl()) {   //Same dl on length distributions
    offset = CI.Offset();
    minl = max(target.Mincol(), Addition.Mincol() + offset);
    maxl = min(target.Maxcol(), Addition.Maxcol() + offset);
    for (l = minl; l < maxl; l++) {
      pop = Addition[l - offset];
      pop *= ratio;
      target[l] += pop;
    }
  } else {             //Not same dl on length distributions
    if (CI.TargetIsFiner()) {
      //Stock that is added to has finer division than the stock that is added to it.
      minl = max(target.Mincol(), CI.Minpos(Addition.Mincol()));
      maxl = min(target.Maxcol(), CI.Maxpos(Addition.Maxcol() - 1) + 1);
      for (l = minl; l < maxl; l++) {
        pop = Addition[CI.Pos(l)];
        pop *= ratio;
        target[l] += pop;
        if (isZero(CI.Nrof(l)))
          cerr << "Error - divide by zero in popinfoadd\n";
        else
          target[l].N /= CI.Nrof(l);
      }
    } else {
      //Stock that is added to has coarser division than the stock that is added to it.
      minl = max(CI.Minpos(target.Mincol()), Addition.Mincol());
      maxl = min(CI.Maxpos(target.Maxcol() - 1) + 1, Addition.Maxcol());
      for (l = minl; l < maxl; l++) {
        pop = Addition[l];
        pop *= ratio;
        target[CI.Pos(l)] += pop;
      }
    }
  }
}

void PopinfoAdd(PopInfoIndexVector& target, const PopInfoIndexVector& Addition,
  const ConversionIndex& CI, double ratio, const DoubleIndexVector& Ratio) {

  PopInfo pop;
  int l, minl, maxl, offset;
  if (CI.SameDl()) {   //Same dl on length distributions
    offset = CI.Offset();
    minl = max(target.Mincol(), Addition.Mincol() + offset, Ratio.Mincol());
    maxl = min(target.Maxcol(), Addition.Maxcol() + offset, Ratio.Maxcol());
    for (l = minl; l < maxl; l++) {
      pop = Addition[l - offset];
      pop *= ratio * Ratio[l];
      target[l] += pop;
    }
  } else {             //Not same dl on length distributions
    if (CI.TargetIsFiner()) {
      //Stock that is added to has finer division than the stock that is added to it.
      minl = max(target.Mincol(), CI.Minpos(Addition.Mincol()), Ratio.Mincol());
      maxl = min(target.Maxcol(), CI.Maxpos(Addition.Maxcol() - 1) + 1, Ratio.Mincol());
      for (l = minl; l < maxl; l++) {
        pop = Addition[CI.Pos(l)];
        pop *= ratio * Ratio[l];
        target[l] += pop;
        if (isZero(CI.Nrof(l)))
          cerr << "Error - divide by zero in popinfoadd\n";
        else
          target[l].N /= CI.Nrof(l);
      }
    } else {
      //Stock that is added to has coarser division than the stock that is added to it.
      minl = max(CI.Minpos(target.Mincol()), Addition.Mincol(), Ratio.Mincol());
      maxl = min(CI.Maxpos(target.Maxcol() - 1) + 1, Addition.Maxcol(), Ratio.Maxcol());
      for (l = minl; l < maxl; l++) {
        pop = Addition[l];
        pop *= ratio * Ratio[l];
        target[CI.Pos(l)] += pop;
      }
    }
  }
}