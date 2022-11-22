/**
 * Author: Florine de Geus (fdegeus@cern.ch)
 */

#include "util.hxx"

void setPlotStyle(TGraph *plot) {
  plot->Sort();
  plot->SetFillColor(9);
  plot->GetXaxis()->CenterTitle();
  plot->GetYaxis()->CenterTitle();
}
