#ifndef preyoverprinter_h
#define preyoverprinter_h

#include "printer.h"
#include "preyoveraggregator.h"

//class PreyOverPrinter;
//class PreyOverAggregator;

/**
 * \class PreyOverPrinter
 * \brief This is the class used to print the overconsumption information for one or more preys
 */
class PreyOverPrinter : public Printer {
public:
  /**
   * \brief This is the default PreyOverPrinter constructor
   * \param infile is the CommentStream to read the printer parameters from
   * \param Area is the AreaClass for the current model
   * \param TimeInfo is the TimeClass for the current model
   */
  PreyOverPrinter(CommentStream& infile, const AreaClass* const Area, const TimeClass* const TimeInfo);
  /**
   * \brief This is the default PreyOverPrinter destructor
   */
  virtual ~PreyOverPrinter();
  /**
   * \brief This will select the preys required for the printer class to print the requested information
   * \param preyvec is the PreyPtrVector of all the available preys
   */
  void setPrey(PreyPtrVector& preyvec);
  /**
   * \brief This will print the requested information for the printer class to the ofstream specified
   * \param TimeInfo is the TimeClass for the current model
   */
  virtual void Print(const TimeClass* const TimeInfo);
protected:
  /**
   * \brief This is the IntMatrix used to store aggregated age information
   */
  IntMatrix areas;
  /**
   * \brief This is the CharPtrVector of the names of the aggregated areas to print
   */
  CharPtrVector areaindex;
  /**
   * \brief This is the CharPtrVector of the names of the lengths to print
   */
  CharPtrVector lenindex;
  /**
   * \brief This is the CharPtrVector of the names of the prey names to print
   */
  CharPtrVector preynames;
  /**
   * \brief This is the LengthGroupDivision of the prey
   */
  LengthGroupDivision* preyLgrpDiv;
  /**
   * \brief This is the PreyOverAggregator used to collect information about the overconsumption
   */
  PreyOverAggregator* aggregator;
  /**
   * \brief This ofstream is the file that all the model information gets sent to
   */
  ofstream outfile;
};

#endif
