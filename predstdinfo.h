#ifndef predstdinfo_h
#define predstdinfo_h

#include "areatime.h"
#include "abstrpreystdinfo.h"
#include "predstdinfobylength.h"
#include "abstrpredstdinfo.h"
#include "poppredator.h"
#include "stockprey.h"
#include "prey.h"

/**
 * \class PredStdInfo
 * \brief This is the class used to calculate information about the non-dynamic predator (eg fleet) from the predation by the predator of a prey
 */
class PredStdInfo : public AbstrPredStdInfo {
public:
  /**
   * \brief This is the PredStdInfo constructor for a non-dynamic prey (eg otherfood)
   * \param predator is the PopPredator of the predator - prey pair
   * \param prey is the Prey of the predator - prey pair
   * \param areas is the IntVector of areas that the predation can take place on
   */
  PredStdInfo(const PopPredator* predator, const Prey* prey, const IntVector& areas);
  /**
   * \brief This is the PredStdInfo constructor for a dynamic prey (eg stock)
   * \param predator is the PopPredator of the predator - prey pair
   * \param prey is the StockPrey of the predator - prey pair
   * \param areas is the IntVector of areas that the predation can take place on
   */
  PredStdInfo(const PopPredator* predator, const StockPrey* prey, const IntVector& areas);
  /**
   * \brief This is the default PredStdInfo destructor
   */
  virtual ~PredStdInfo();
  /**
   * \brief This will return the number consumed by length group for a specified area
   * \param area is the area that the predation is calculated on
   * \return N, a BandMatrix containing the number consumed
   */
  virtual const BandMatrix& NconsumptionByLength(int area) const;
  /**
   * \brief This will return the biomass consumed by length group for a specified area
   * \param area is the area that the predation is calculated on
   * \return B, a BandMatrix containing the biomass consumed
   */
  virtual const BandMatrix& BconsumptionByLength(int area) const;
  /**
   * \brief This will return the mortality by length group caused by the predation for a specified area
   * \param area is the area that the predation is calculated on
   * \return M, a BandMatrix containing the mortality
   */
  virtual const BandMatrix& MortalityByLength(int area) const;
  /**
   * \brief This is the function that calculates the predation
   * \param TimeInfo is the TimeClass for the current model
   * \param area is the area that the predation is being calculated on
   */
  virtual void Sum(const TimeClass* const TimeInfo, int area);
private:
  /**
   * \brief This is the AbstrPreyStdInfo giving information about the predation of the prey
   */
  AbstrPreyStdInfo* preyinfo;
  /**
   * \brief This is the PredStdInfoByLength giving information about the predation by the predator by length group
   */
  PredStdInfoByLength* predinfo;
  /**
   * \brief This is the PopPredator of the predator - prey pair
   */
  const PopPredator* predator;
  /**
   * \brief This is the Prey of the predator - prey pair
   */
  const Prey* prey;
};

#endif
