#include "netdata.h"

#ifdef GADGET_NETWORK
#include "gadget.h"
#else
#include "paramin.h"
#endif

NetDataVariables::NetDataVariables(int numVar) {
  tag = -1;
  x_id = -1;
  if (numVar < 1) {
    cerr << "Error in netdata - number of variables must be positive\n";
    exit(EXIT_FAILURE);
  }
  x = new double[numVar];
}

NetDataVariables::~NetDataVariables() {
  delete[] x;
}

NetDataResult::NetDataResult() {
}

NetDataResult::~NetDataResult() {
}
