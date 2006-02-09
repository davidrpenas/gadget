#include "stockaggregator.h"
#include "conversionindex.h"
#include "stock.h"
#include "mathfunc.h"

StockAggregator::StockAggregator(const StockPtrVector& Stocks,
  const LengthGroupDivision* const LgrpDiv, const IntMatrix& Areas, const IntMatrix& Ages)
  : stocks(Stocks), areas(Areas), ages(Ages), alptr(0) {

  int i;
  for (i = 0; i < stocks.Size(); i++)
    CI.resize(new ConversionIndex(stocks[i]->getLengthGroupDiv(), LgrpDiv));

  //Resize total using dummy variables tmppop and popmatrix.
  PopInfo tmppop;
  tmppop.N = 1.0;
  PopInfoMatrix popmatrix(ages.Nrow(), LgrpDiv->numLengthGroups(), tmppop);
  total.resize(areas.Nrow(), 0, 0, popmatrix);
  this->Reset();
}

StockAggregator::~StockAggregator() {
  int i;
  for (i = 0; i < CI.Size(); i++)
    delete CI[i];
}

void StockAggregator::Print(ofstream& outfile) const {
  int i;
  for (i = 0; i < total.Size(); i++) {
    outfile << "\tInternal areas " << areas[i][0] << endl;
    total[i].printNumbers(outfile);
  }
  outfile.flush();
}

void StockAggregator::Reset() {
  int i;
  for (i = 0; i < total.Size(); i++)
    total[i].setToZero();
}

void StockAggregator::Sum() {
  int area, age, i, j, k;

  this->Reset();
  //Sum over the appropriate stocks, areas, ages and length groups.
  for (i = 0; i < stocks.Size(); i++) {
    for (area = 0; area < areas.Nrow(); area++) {
      for (j = 0; j < areas.Ncol(area); j++) {
        if (stocks[i]->isInArea(areas[area][j])) {
          alptr = &stocks[i]->getAgeLengthKeys(areas[area][j]);
          for (age = 0; age < ages.Nrow(); age++) {
            for (k = 0; k < ages.Ncol(age); k++) {
              if ((alptr->minAge() <= ages[age][k]) && (ages[age][k] <= alptr->maxAge()))
                total[area][age].Add((*alptr)[ages[age][k]], *CI[i]);
            }
          }
        }
      }
    }
  }
}
