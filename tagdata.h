#ifndef tagdata_h
#define tagdata_h

#include "likelihood.h"
#include "commentstream.h"
#include "recaggregator.h"
#include "doublematrixptrmatrix.h"
#include "tagptrvector.h"

class TagData : public Likelihood {
public:
  TagData(CommentStream& infile, const AreaClass* const Area,
    const TimeClass* const TimeInfo, double w, TagPtrVector Tag, const char* name);
  virtual ~TagData();
  virtual void AddToLikelihood(const TimeClass* const TimeInfo);
  virtual void Reset(const Keeper* const keeper);
  virtual void Print(ofstream& outfile) const;
  virtual void LikelihoodPrint(ofstream& outfile);
  virtual void SetFleetsAndStocks(FleetPtrVector& Fleets, StockPtrVector& Stocks);
private:
  void ReadRecaptureData(CommentStream& infile, const TimeClass* const TimeInfo);
  double LikPoisson(const TimeClass* const TimeInfo);
  RecAggregator** aggregator;
  CharPtrVector tagid;
  CharPtrVector fleetnames;
  CharPtrVector areaindex;
  CharPtrVector lenindex;
  IntMatrix areas;
  DoubleVector lengths;
  IntMatrix Years;
  IntMatrix Steps;
  DoubleMatrixPtrMatrix recaptures;
  DoubleMatrixPtrMatrix modelRecaptures;
  LengthGroupDivision* lgrpdiv;
  TagPtrVector tagvec;
  char* tagname;
  int functionnumber;
  char* functionname;
  int numarea;
  int numlen;
};

#endif
