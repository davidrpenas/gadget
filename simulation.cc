#include "ecosystem.h"
#include "agebandmatrix.h"
#include "errorhandler.h"
#include "runid.h"
#include "gadget.h"

#ifdef INTERRUPT_HANDLER
#include "interruptinterface.h"
#endif

extern RunID RUNID;
extern ErrorHandler handle;

void Ecosystem::SimulateOneAreaOneTimeSubstep(int area) {

  //Calculate the number of preys and predators in area.
  //Base includes the classes OtherFood, Fleet and Stock.
  int i;
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->CalcNumbers(area, Area, TimeInfo);

  //Calculate consumption and Growth;
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->calcEat(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->checkEat(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->adjustEat(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->ReducePop(area, Area, TimeInfo);
}

void Ecosystem::GrowthAndSpecialTransactions(int area) {
  int i;

  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->Grow(area, Area, TimeInfo);

  //Under SpecialTransactions are movements to mature stock, spawning and renewal.
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->FirstSpecialTransactions(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->SecondSpecialTransactions(area, Area, TimeInfo);
}

void Ecosystem::updateOneTimestepOneArea(int area) {
  //Age related update and movements between stocks.
  int i;
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->FirstUpdate(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->SecondUpdate(area, Area, TimeInfo);
  for (i = 0; i < basevec.Size(); i++)
    if (basevec[i]->IsInArea(area))
      basevec[i]->ThirdUpdate(area, Area, TimeInfo);
}

void Ecosystem::SimulateOneTimestep() {
  int i;
  while (TimeInfo->CurrentSubstep() <=  TimeInfo->NrOfSubsteps()) {
    for (i = 0; i < basevec.Size(); i++)
      basevec[i]->Migrate(TimeInfo);
    for (i = 0; i < Area->NoAreas(); i++)
      SimulateOneAreaOneTimeSubstep(i);
    TimeInfo->IncrementSubstep();
  }

  for (i = 0; i < Area->NoAreas(); i++)
    GrowthAndSpecialTransactions(i);
}

//Returns 1 when success, 0 when failure.
int Ecosystem::Simulate(int Optimize, int print) {
  likelihood = 0.0;
  int i, j;

  for (i = 0; i < Likely.Size(); i++)
    Likely[i]->Reset(keeper);

  if (Optimize)
    //put here since boundlikelihood is allowed to change values
    for (i = 0; i < Likely.Size(); i++)
      Likely[i]->addLikelihoodKeeper(TimeInfo, keeper);

  TimeInfo->ResetToBeginning();
  for (i = 0; i < basevec.Size(); i++)
    basevec[i]->Reset(TimeInfo);
  for (i = 0; i < tagvec.Size(); i++)
    tagvec[i]->Reset(TimeInfo);

  for (i = 0; i < TimeInfo->TotalNoSteps(); i++) {
    if (i > 0) //do not need to call reset again on the first step.
      for (j = 0; j < basevec.Size(); j++)
        basevec[j]->Reset(TimeInfo);

    //Add in any new tagging experiments
    tagvec.updateTags(TimeInfo);

    if (TimeInfo->CurrentStep() == 1) //Migrations recalculated once per year.
      for (j = 0; j < basevec.Size(); j++)
        basevec[j]->RecalcMigration(TimeInfo);

    SimulateOneTimestep();
    if (Optimize)
      for (j = 0; j < Likely.Size(); j++)
        Likely[j]->addLikelihood(TimeInfo);

    if (print) {
      for (j = 0; j < printvec.Size(); j++)
        printvec[j]->Print(TimeInfo);
      for (j = 0; j < likprintvec.Size(); j++)
        likprintvec[j]->Print(TimeInfo);
    }

    for (j = 0; j < Area->NoAreas(); j++)
      updateOneTimestepOneArea(j);

    #ifdef INTERRUPT_HANDLER
      if (interrupted) {
        InterruptInterface ui(*this);
        if (!ui.menu()) {
          handle.logMessage("** Gadget interrupted - quitting current simulation **");
          //JMB - dump *current* switch values to a file - these wont usually be the *best* values
          char interruptfile[15];
          strncpy(interruptfile, "", 15);
          strcpy(interruptfile, "interrupt.out");
          this->writeParamsInColumns(interruptfile, 0);
          exit(EXIT_SUCCESS);
        }
        interrupted = 0;
      }
    #endif

    //Remove any expired tagging experiments
    tagvec.DeleteTags(TimeInfo);

    if (i != TimeInfo->TotalNoSteps() - 1)
      TimeInfo->IncrementTime();
  }

  for (i = 0; i < Likely.Size(); i++)
    likelihood += Likely[i]->returnLikelihood();

  return 1;
}
