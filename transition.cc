#include "transition.h"
#include "errorhandler.h"
#include "keeper.h"
#include "readword.h"
#include "gadget.h"

Transition::Transition(CommentStream& infile, const IntVector& areas, int Age,
  const LengthGroupDivision* const lgrpdiv, Keeper* const keeper)
  : LivesOnAreas(areas), LgrpDiv(new LengthGroupDivision(*lgrpdiv)), age(Age) {

  int i;
  ErrorHandler handle;
  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);
  keeper->AddString("transition");

  infile >> text;
  if (strcasecmp(text, "transitionstocksandratios") == 0) {
    infile >> text;
    i = 0;
    while (strcasecmp(text, "transitionstep") != 0 && infile.good()) {
      TransitionStockNames.resize(1);
      TransitionStockNames[i] = new char[strlen(text) + 1];
      strcpy(TransitionStockNames[i], text);
      Ratio.resize(1);
      infile >> Ratio[i];
      i++;
      infile >> text;
    }
  } else
    handle.Unexpected("transitionstocksandratios", text);

  infile >> TransitionStep >> ws;
  keeper->ClearLast();
}

Transition::~Transition() {
  int i;
  for (i = 0; i < TransitionStockNames.Size(); i++)
    delete[] TransitionStockNames[i];
  for (i = 0; i < CI.Size(); i++)
    delete CI[i];
  delete LgrpDiv;
}

void Transition::SetStock(StockPtrVector& stockvec) {
  int index = 0;
  int i, j, numstocks;
  DoubleVector tmpratio;

  for (i = 0; i < stockvec.Size(); i++)
    for (j = 0; j < TransitionStockNames.Size(); j++)
      if (strcasecmp(stockvec[i]->Name(), TransitionStockNames[j]) == 0) {
        TransitionStocks.resize(1);
        tmpratio.resize(1);
        TransitionStocks[index] = stockvec[i];
        tmpratio[index] = Ratio[j];
        index++;
      }

  if (index != TransitionStockNames.Size()) {
    cerr << "Error: Did not find the stock(s) matching:\n";
    for (i = 0; i < TransitionStockNames.Size(); i++)
      cerr << (const char*)TransitionStockNames[i] << sep;
    cerr << "\nwhen searching for transition stock(s) - found only:\n";
    for (i = 0; i < stockvec.Size(); i++)
      cerr << stockvec[i]->Name() << sep;
    cerr << endl;
    exit(EXIT_FAILURE);
  }

  for (i = Ratio.Size(); i > 0; i--)
    Ratio.Delete(0);
  Ratio.resize(tmpratio.Size());
  for (i = 0; i < tmpratio.Size(); i++)
    Ratio[i] = tmpratio[i];

  numstocks = TransitionStocks.Size();
  CI.resize(numstocks, 0);
  for (i = 0; i < numstocks; i++)
    CI[i] = new ConversionIndex(LgrpDiv, TransitionStocks[i]->ReturnLengthGroupDiv());

  //JMB - check that the tranistion stocks are defined on the areas
  for (i = 0; i < numstocks; i++) {
    index = 0;
    for (j = 0; j < areas.Size(); j++)
      if (!TransitionStocks[i]->IsInArea(areas[j]))
        index++;

    if (index != 0)
      cerr << "Warning: transition requested to stock " << (const char*)TransitionStocks[i]->Name()
        << "\nwhich might not be defined on " << index << " areas\n";
  }

  IntVector minlv(2, 0);
  IntVector sizev(2, LgrpDiv->NoLengthGroups());
  AgeGroup.resize(areas.Size(), age, minlv, sizev);
  TagAgeGroup.resize(areas.Size(), age, minlv, sizev);
}

void Transition::Print(ofstream& outfile) const {
  int i;
  outfile << "\nTransition\n\tRead names of transition stocks:";
  for (i = 0; i < TransitionStockNames.Size(); i++)
    outfile << sep << (const char*)(TransitionStockNames[i]);
  outfile << "\n\tNames of transition stocks (through pointers):";
  for (i = 0; i < TransitionStocks.Size(); i++)
    outfile << sep << (const char*)(TransitionStocks[i]->Name());
  outfile << "\n\tTransition step " << TransitionStep  << endl;
}

void Transition::KeepAgegroup(int area, AgeBandMatrix& Alkeys,
  AgeBandMatrixRatio& TagAlkeys, const TimeClass* const TimeInfo) {

  int inarea = AreaNr[area];
  int numtags = TagAlkeys.NrOfTagExp();
  int i, l, minl, maxl;
  double tagnumber;

  if (TimeInfo->CurrentStep() == TransitionStep) {
    AgeGroup[inarea].SettoZero();
    TagAgeGroup[inarea].SettoZero();
    minl = AgeGroup[inarea].Minlength(age);
    maxl = AgeGroup[inarea].Maxlength(age);
    for (l = minl; l < maxl; l++) {
      AgeGroup[inarea][age][l].N = Alkeys[age][l].N;
      AgeGroup[inarea][age][l].W = Alkeys[age][l].W;
      Alkeys[age][l].N = 0.0;
      Alkeys[age][l].W = 0.0;
      for (i = 0; i < numtags; i++) {
        tagnumber = *(TagAlkeys[age][l][i].N);
        if (tagnumber < verysmall)
          *(TagAgeGroup[inarea][age][l][i].N) = 0.0;
        else
          *(TagAgeGroup[inarea][age][l][i].N) = tagnumber;

        *(TagAlkeys[age][l][i].N) = 0.0;
        TagAlkeys[age][l][i].R = 0.0;
      }
    }
  }
}

//area in the call to this routine is not in the local area numbering of the stock.
void Transition::Move(int area, const TimeClass* const TimeInfo) {
  int s, inarea = AreaNr[area];

  if (TimeInfo->CurrentStep() == TransitionStep) {
    for (s = 0; s < TransitionStocks.Size(); s++) {
      if (!TransitionStocks[s]->IsInArea(area)) {
        cerr << "Error: Transition to stock " << (const char*)(TransitionStocks[s]->Name())
          << " cannot take place on area " << area << " since it doesnt live there!\n";
        exit(EXIT_FAILURE);
      }

      if (TransitionStocks[s]->Birthday(TimeInfo)) {
        AgeGroup[inarea].IncrementAge();
        if (TagAgeGroup.NrOfTagExp() > 0)
          TagAgeGroup[inarea].IncrementAge(AgeGroup[inarea]);
      }

      TransitionStocks[s]->Add(AgeGroup[inarea], CI[s], area, Ratio[s],
        AgeGroup[inarea].Minage(), AgeGroup[inarea].Maxage());

      if (TagAgeGroup.NrOfTagExp() > 0)
        TransitionStocks[s]->Add(TagAgeGroup, inarea, CI[s], area, Ratio[s],
          TagAgeGroup[inarea].Minage(), TagAgeGroup[inarea].Maxage());

    }
  }
}

const StockPtrVector& Transition::GetTransitionStocks() {
  return TransitionStocks;
}

void Transition::AddTag(const char* tagname) {
  TagAgeGroup.addTag(tagname);
}

void Transition::DeleteTag(const char* tagname) {
  int minage, maxage, minlen, maxlen, age, length, i;
  int id = TagAgeGroup.getId(tagname);

  if (id >= 0) {
    minage = TagAgeGroup[0].Minage();
    maxage = TagAgeGroup[0].Maxage();

    // Remove allocated memory
    for (i = 0; i < TagAgeGroup.Size(); i++) {
      for (age = minage; age <= maxage; age++) {
        minlen = TagAgeGroup[i].Minlength(age);
        maxlen = TagAgeGroup[i].Maxlength(age);
        for (length = minlen; length < maxlen; length++) {
          delete[] (TagAgeGroup[i][age][length][id].N);
          (TagAgeGroup[i][age][length][id].N) = NULL;
        }
      }
    }
    TagAgeGroup.deleteTag(tagname);

  } else {
    cerr << "Error in tagging transition - trying to delete tag with name: "
      << tagname << " in transition but there is not any tag with that name\n";
  }
}
