#ifndef sionstep_h
#define sionstep_h

#include "areatime.h"
#include "doublematrix.h"
#include "intmatrix.h"
#include "charptrvector.h"
#include "conversionindex.h"
#include "commentstream.h"
#include "actionattimes.h"
#include "keeper.h"
#include "agebandmatrix.h"
#include "gadget.h"

enum FitType { LogLinearFit = 1, FixedSlopeLogLinearFit, FixedLogLinearFit, LinearFit, PowerFit,
    FixedSlopeLinearFit, FixedLinearFit, FixedInterceptLinearFit, FixedInterceptLogLinearFit };

class SIOnStep;

class SIOnStep {
public:
  SIOnStep(CommentStream& infile, const char* datafilename, const char* arealabel,
    const TimeClass* const TimeInfo, int numcols,
    const CharPtrVector& index1, const CharPtrVector& index2);
  SIOnStep(CommentStream& infile, const char* datafilename, const char* arealabel,
    const TimeClass* const TimeInfo, const CharPtrVector& colindex);
  virtual ~SIOnStep();
  virtual void Sum(const TimeClass* const TimeInfo) = 0;
  virtual void SetStocks(const StockPtrVector& Stocks) = 0;
  virtual double Regression();
  virtual void Reset(const Keeper* const keeper);
  virtual void Print(ofstream& outfile) const;
  virtual void LikelihoodPrint(ofstream& outfile);
  virtual void CommandLinePrint(ofstream& outfile, const TimeClass& time, const PrintInfo&) {};
  virtual void PrintLikelihood(ofstream& outfile, const TimeClass& time, const char*) {};
  virtual void PrintLikelihoodHeader(ofstream& outfile, const char* name) {};
protected:
  void SetError() { error = 1; };
  int IsToSum(const TimeClass* const TimeInfo) const;
  void KeepNumbers(const DoubleVector& numbers);
  IntVector Years;
  IntVector Steps;
  IntVector YearsInFile;
  ActionAtTimes AAT;
  DoubleMatrix Indices;
  DoubleMatrix abundance;
  FitType getFitType() { return fittype; };
private:
  void ReadSIData(CommentStream&, const char*, const CharPtrVector&, const CharPtrVector&, const TimeClass*);
  void ReadSIData(CommentStream&, const char*, const CharPtrVector&, const TimeClass*);
  double Fit(const DoubleVector& stocksize, const DoubleVector& indices, int col);
  int NumberOfSums;
  FitType fittype;
  double slope;
  double intercept;
  //Additions to facilitate printing
  DoubleVector slopes;
  DoubleVector intercepts;
  DoubleVector sse;
  int error;
};

#endif
