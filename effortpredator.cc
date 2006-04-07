#include "effortpredator.h"
#include "keeper.h"
#include "prey.h"
#include "mathfunc.h"
#include "errorhandler.h"
#include "gadget.h"

extern ErrorHandler handle;

EffortPredator::EffortPredator(CommentStream& infile, const char* givenname,
  const IntVector& Areas, const TimeClass* const TimeInfo, Keeper* const keeper, Formula multscaler)
  : LengthPredator(givenname, Areas, keeper, multscaler) {

  type = EFFORTPREDATOR;
  keeper->addString("predator");
  keeper->addString(givenname);

  //first read in the suitability parameters
  this->readSuitability(infile, TimeInfo, keeper);

  //now we read in the catchability parameters - should be one for each prey
  keeper->addString("catchability");
  int i, count = 0;
  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);
  infile >> text >> ws;
  while (!(strcasecmp(text, "amount") == 0) && (!infile.eof())) {
    for (i = 0; i < preference.Size(); i++) {
      if (strcasecmp(text, this->getPreyName(i)) == 0) {
        infile >> preference[i] >> ws;
        count++;
      }
    }
    infile >> text >> ws;
  }
  if (count != preference.Size())
    handle.logMessage(LOGFAIL, "Error in effortpredator - missing catchability data");
  preference.Inform(keeper);
  keeper->clearLast();

  keeper->clearLast();
  keeper->clearLast();
}

void EffortPredator::Eat(int area, const AreaClass* const Area, const TimeClass* const TimeInfo) {

  int inarea = this->areaNum(area);
  int prey, preyl;
  double tmp;

  int predl = 0;  //JMB there is only ever one length group ...
  totalcons[inarea][predl] = 0.0;
  scaler[inarea] = multi * TimeInfo->getTimeStepSize() / TimeInfo->numSubSteps();

  for (prey = 0; prey < this->numPreys(); prey++) {
    tmp = preference[prey] * scaler[inarea] * prednumber[inarea][predl].N;
    if (tmp > 10.0) //JMB arbitrary value here ...
      handle.logMessage(LOGWARN, "Warning in effortpredator - excessive consumption required");

    if (this->getPrey(prey)->isPreyArea(area)) {
      for (preyl = 0; preyl < (*cons[inarea][prey])[predl].Size(); preyl++) {
        (*cons[inarea][prey])[predl][preyl] = tmp *
          this->getSuitability(prey)[predl][preyl] * this->getPrey(prey)->getBiomass(area, preyl);
        totalcons[inarea][predl] += (*cons[inarea][prey])[predl][preyl];
      }

    } else {
      for (preyl = 0; preyl < (*cons[inarea][prey])[predl].Size(); preyl++)
        (*cons[inarea][prey])[predl][preyl] = 0.0;
    }
  }

  //inform the preys of the consumption
  for (prey = 0; prey < this->numPreys(); prey++)
    if (this->getPrey(prey)->isPreyArea(area))
      this->getPrey(prey)->addBiomassConsumption(area, (*cons[inarea][prey])[predl]);
}

void EffortPredator::adjustConsumption(int area, const TimeClass* const TimeInfo) {
  int prey, preyl;
  int inarea = this->areaNum(area);
  double maxRatio, tmp;

  int predl = 0;  //JMB there is only ever one length group ...
  overcons[inarea][predl] = 0.0;
  maxRatio = MaxRatioConsumed;
  if (TimeInfo->numSubSteps() != 1)
    maxRatio = pow(MaxRatioConsumed, TimeInfo->numSubSteps());

  for (prey = 0; prey < this->numPreys(); prey++) {
    if (this->getPrey(prey)->isOverConsumption(area)) {
      DoubleVector ratio = this->getPrey(prey)->getRatio(inarea);
      for (preyl = 0; preyl < (*cons[inarea][prey])[predl].Size(); preyl++) {
        if (ratio[preyl] > maxRatio) {
          tmp = maxRatio / ratio[preyl];
          overcons[inarea][predl] += (1.0 - tmp) * (*cons[inarea][prey])[predl][preyl];
          (*cons[inarea][prey])[predl][preyl] *= tmp;
        }
      }
    }
  }

  totalcons[inarea][predl] -= overcons[inarea][predl];
  totalconsumption[inarea][predl] += totalcons[inarea][predl];
  overconsumption[inarea][predl] += overcons[inarea][predl];
  for (prey = 0; prey < this->numPreys(); prey++)
    if (this->getPrey(prey)->isPreyArea(area))
      for (preyl = 0; preyl < (*cons[inarea][prey])[predl].Size(); preyl++)
        (*consumption[inarea][prey])[predl][preyl] += (*cons[inarea][prey])[predl][preyl];
}

void EffortPredator::Print(ofstream& outfile) const {
  outfile << "EffortPredator\n";
  PopPredator::Print(outfile);
}