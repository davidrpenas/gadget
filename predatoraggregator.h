#ifndef predatoraggregator_h
#define predatoraggregator_h

#include "conversionindexptrvector.h"
#include "agebandm.h"
#include "intmatrix.h"
#include "predatorptrvector.h"
#include "preyptrvector.h"
#include "charptrvector.h"

class PredatorAggregator;

class PredatorAggregator {
public:
  PredatorAggregator(const Predatorptrvector& predators, const Preyptrvector& preys,
    const intmatrix& Areas, const LengthGroupDivision* const predLgrpDiv,
    const LengthGroupDivision* const preyLgrpDiv);
  PredatorAggregator(const charptrvector& pred_names, Preyptrvector& Preys,
    const intmatrix& Areas, const intvector&  ages,
    const LengthGroupDivision* const preyLgrpDiv);
  //The first constructor is for length structured predators,
  //the second constructor is for age structured predators
  void Sum();
  void Sum(int dummy); //to be used with the second constructor
  void NumberSum();
  const bandmatrixvector& ReturnSum() const;
protected:
  Predatorptrvector predators;
  Preyptrvector preys;
  charptrvector prednames;
  intmatrix predConv;  //[predator][predatorLengthGroup]
  intmatrix preyConv;  //[prey][preyLengthGroup]
  intmatrix areas;
  intvector AreaNr;
  intmatrix doeseat;  //[predator][prey] -- does predator eat prey?
  bandmatrixvector total;
};

#endif
