#include "grower.h"
#include "errorhandler.h"
#include "readfunc.h"
#include "readword.h"
#include "conversion.h"
#include "keeper.h"
#include "areatime.h"
#include "growthcalc.h"
#include "gadget.h"

Grower::Grower(CommentStream& infile, const LengthGroupDivision* const OtherLgrpDiv,
  const LengthGroupDivision* const GivenLgrpDiv, const intvector& Areas,
  const TimeClass* const TimeInfo, Keeper* const keeper, const char* refWeight,
  const AreaClass* const Area, const charptrvector& lenindex)
  : LivesOnAreas(Areas), LgrpDiv(0), CI(0), GrIPar(0), growthcalc(0) {

  ErrorHandler handle;
  char text[MaxStrLength];
  strncpy(text, "", MaxStrLength);
  int i;

  keeper->AddString("grower");
  LgrpDiv = new LengthGroupDivision(*GivenLgrpDiv);
  CI = new ConversionIndex(OtherLgrpDiv, LgrpDiv, 1);

  //Next we read a word to determine which growthfunction to use.
  //1. The Gadget function.
  //2. Read growth from file.
  //3. Von Bertanlanffy growth function.
  //4. Jones.
  //5. Von Bertanlanffy growth function with year, step and area effects.
  //6. Von Bertanlanffy growth function (length), weight read from file
  //7. Growth as a function of l^p,
  //where p = 0 gives linear growth
  //and p < 0 to have length growth decreasing as a function of l.

  int growthfunctionnumber = 0;
  ReadWordAndVariable(infile, "growthfunctionnumber", growthfunctionnumber);
  switch(growthfunctionnumber) {
    case 1:
      growthcalc = new GrowthCalcA(infile, areas, keeper);
      break;
    case 2:
      growthcalc = new GrowthCalcB(infile, areas, TimeInfo, keeper, Area, lenindex);
      break;
    case 3:
      growthcalc = new GrowthCalcC(infile, areas, LgrpDiv, keeper, refWeight);
      break;
    case 4:
      growthcalc = new GrowthCalcD(infile, areas, LgrpDiv, keeper, refWeight);
      break;
    case 5:
      growthcalc = new GrowthCalcE(infile, areas, TimeInfo, LgrpDiv, keeper, refWeight);
      break;
    case 6:
      growthcalc = new GrowthCalcF(infile, areas, TimeInfo, keeper, Area, lenindex);
      break;
    case 7:
      growthcalc = new GrowthCalcG(infile, areas, TimeInfo, keeper, Area, lenindex);
      break;
    default:
      handle.Message("Illegal growthfunction number");
      break;
  }

  growth_type = growthfunctionnumber - 1;
  int rows = 0; //Number of rows in wgrowth and lgrowth
  infile >> ws >>  text;
  if ((strcasecmp(text, "power") == 0)) {
    //New beta binomial growth distribution code is used
    version = 1;

    //Growthimplement parameters put in.
    infile >> power >> ws;
    ReadWordAndVariable(infile, "maxlengthgroupgrowth", maxlengthgroupgrowth);

    infile >> text >> ws;
    if (strcasecmp(text, "beta") == 0)
      infile >> beta;
    else
      handle.Unexpected("beta", text);
    beta.Inform(keeper);

    //Finished reading from input files.
    part1.resize(maxlengthgroupgrowth + 1);
    part2.resize(maxlengthgroupgrowth + 1);
    part4.resize(maxlengthgroupgrowth + 1);
    rows = maxlengthgroupgrowth + 1;

  } else if ((strcasecmp(text, "MeanVarianceParameters") == 0)) {
    //Old growth distribution, with distribution file
    version = 2;

    MeanVarianceParameters.resize(2, keeper);
    infile >> MeanVarianceParameters;
    MeanVarianceParameters.Inform(keeper);

    ReadWordAndVariable(infile, "power", power);
    ReadWordAndValue(infile, "growthdistributionfile", text);

    ifstream subfile;
    CommentStream subcomment(subfile);
    subfile.open(text);
    CheckIfFailure(subfile, text);
    handle.Open(text);
    GrIPar = new GrowthImplementparameters(subcomment);
    handle.Close();
    subfile.close();
    subfile.clear();
    rows = GrIPar->MaxLengthgroupGrowth() - GrIPar->MinLengthgroupGrowth() + 1;

  } else
    handle.Unexpected("growthimplementfile or MeanVarianceParameters", text);

  //Finished reading from input files.
  const int noareas = areas.Size();
  const int len = LgrpDiv->NoLengthGroups();
  const int otherlen = OtherLgrpDiv->NoLengthGroups();
  popinfo nullpop;
  GrEatNumber.AddRows(noareas, len, nullpop);
  keeper->ClearLast();

  //Setting storage spaces for Growth.
  CalcLgrowth.AddRows(noareas, len, 0.0);
  CalcWgrowth.AddRows(noareas, len, 0.0);
  InterpLgrowth.AddRows(noareas, otherlen, 0.0);
  InterpWgrowth.AddRows(noareas, otherlen, 0.0);
  lgrowth.resize(noareas);
  wgrowth.resize(noareas);
  for (i = 0; i < noareas; i++) {
    lgrowth[i] = new doublematrix(rows, otherlen, 0.0);
    wgrowth[i] = new doublematrix(rows, otherlen, 0.0);
  }
  Fphi.resize(len, 0.0);
}

Grower::~Grower() {
  int i;
  for (i = 0; i < lgrowth.Size(); i++) {
    delete lgrowth[i];
    delete wgrowth[i];
  }
  delete CI;
  delete LgrpDiv;
  delete GrIPar;
  delete growthcalc;
}

void Grower::Print(ofstream& outfile) const {
  int i, j, area;

  outfile << "\nGrower\n\tMeanlengths of length groups.\n\t";
  for (i = 0; i < LgrpDiv->NoLengthGroups(); i++)
    outfile << sep << LgrpDiv->Meanlength(i);

  outfile << "\n\tLength increase:\n";
  for (area = 0; area < areas.Size(); area++) {
    outfile << "\tarea " << areas[area];
    for (i = 0; i < CalcLgrowth.Ncol(area); i++) {
      outfile.precision(smallprecision);
      outfile.width(printwidth);
      outfile << sep << CalcLgrowth[area][i];
    }
    outfile << endl;
  }

  outfile << "\tWeight increase:\n";
  for (area = 0; area < areas.Size(); area++) {
    outfile << "\tarea " << areas[area];
    for (i = 0; i < CalcWgrowth.Ncol(area); i++) {
      outfile.precision(smallprecision);
      outfile.width(printwidth);
      outfile << sep << CalcWgrowth[area][i];
    }
    outfile << endl;
  }

  outfile << "\tDistributed length increase:\n";
  for (area = 0; area < areas.Size(); area++) {
    outfile << "\tarea " << areas[area] << endl;
    for (i = 0; i < lgrowth[area]->Nrow(); i++) {
      outfile << TAB;
      for (j = 0; j < lgrowth[area]->Ncol(i); j++)
        outfile << sep << (*lgrowth[area])[i][j];
      outfile << endl;
    }
  }

  outfile << "Distributed weight increase:\n";
  for (area = 0; area < areas.Size(); area++) {
    outfile << "\tarea " << areas[area] << endl;
    for (i = 0; i < wgrowth[area]->Nrow(); i++) {
      outfile << TAB;
      for (j = 0; j < wgrowth[area]->Ncol(i); j++)
        outfile << sep << (*wgrowth[area])[i][j];
      outfile << endl;
    }
  }
  outfile << endl;
}

//The following function is just a copy of Prey::Sum
void Grower::Sum(const popinfovector& NumberInArea, int area) {
  int inarea = AreaNr[area];
  int i;
  for (i = 0; i < GrEatNumber[inarea].Size(); i++)
    GrEatNumber[inarea][i].N = 0.0;
  GrEatNumber[inarea].Sum(&NumberInArea, *CI);
}

void Grower::GrowthCalc (int area,
  const AreaClass* const Area, const TimeClass* const TimeInfo) {

  doublevector FPhi(CalcLgrowth[AreaNr[area]].Size(), 0.0);
  //Let the feeding level and consumption equal 0.
  this->GrowthCalc(area, Area, TimeInfo, FPhi, FPhi);
}

void Grower::GrowthCalc(int area,
  const AreaClass* const Area, const TimeClass* const TimeInfo,
  const doublevector& FPhi, const doublevector& MaxCon) {

  int inarea = AreaNr[area];
  growthcalc->GrowthCalc(area, CalcLgrowth[inarea], CalcWgrowth[inarea],
    GrEatNumber[inarea], Area, TimeInfo, FPhi, MaxCon, LgrpDiv);

  Interp(InterpLgrowth[inarea], CalcLgrowth[inarea], CI);
  Interp(InterpWgrowth[inarea], CalcWgrowth[inarea], CI);
}

const doublematrix& Grower::LengthIncrease(int area) const {
  return *lgrowth[AreaNr[area]];
}

const doublematrix& Grower::WeightIncrease(int area) const {
  return *wgrowth[AreaNr[area]];
}

const doublevector& Grower::getWeight(int area) const {
  return InterpWgrowth[AreaNr[area]];
}

void Grower::Reset() {
  int i, j, area;
  double factorialx, tmppart, tmpmax;

  for (area = 0; area < areas.Size(); area++) {
    for (i = 0; i < LgrpDiv->NoLengthGroups(); i++) {
      CalcLgrowth[area][i] = 0.0;
      CalcWgrowth[area][i] = 0.0;
      GrEatNumber[area][i].N = 0.0;
      GrEatNumber[area][i].W = 0.0;
    }

    for (i = 0; i < wgrowth[area]->Nrow(); i++) {
      for (j = 0; j < wgrowth[area]->Ncol(i); j++) {
        (*wgrowth[area])[i][j] = 0.0;
        (*lgrowth[area])[i][j] = 0.0;
      }
    }

    for (i = 0; i < InterpLgrowth.Ncol(); i++) {
      InterpLgrowth[area][i] = 0.0;
      InterpWgrowth[area][i] = 0.0;
    }
  }

  if (version == 1) {
    //GrowthImplement.resize put 24-3-2000
    tmpmax = double(maxlengthgroupgrowth) * 1.0;
    part1[0] = 1.0;
    part1[1] = tmpmax;
    factorialx = 1.0;
    tmppart = tmpmax;
    if (maxlengthgroupgrowth > 1) {
      for (i = 2; i < maxlengthgroupgrowth + 1; i++) {
        tmppart *= (tmpmax - i + 1);
        factorialx *= double(i);
        part1[i] = tmppart / factorialx;
      }
    }
    part2[maxlengthgroupgrowth] = 1.0;
    part2[maxlengthgroupgrowth - 1] = beta;
    if (maxlengthgroupgrowth > 1)
      for (i = maxlengthgroupgrowth - 2; i >= 0; i--)
        part2[i] = part2[i + 1] * (beta + tmpmax - i - 1);
  }
}
