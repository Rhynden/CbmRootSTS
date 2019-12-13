/** @file CbmStsTimeBasedQaReport.h
 ** @author Grigory Kozlov <g.kozlov@gsi.de>
 ** @date 19.04.2016
 **/

#ifndef CBMSTSTIMEBASEDQAREPORT_H_
#define CBMSTSTIMEBASEDQAREPORT_H_

#include "CbmSimulationReport.h"
#include <string>

class CbmStsTimeBasedQaReport : public CbmSimulationReport
{
public:
  CbmStsTimeBasedQaReport();

  CbmStsTimeBasedQaReport(Bool_t useDaq);

  virtual ~CbmStsTimeBasedQaReport();
private:
  virtual void Create();

  virtual void Draw();

  void Draw2dHistograms(const std::string& type);

  void DrawNofObjectsHistograms(const std::string& type);

  void DrawResidualAndPullHistograms(const std::string& type);

  void DrawPointsInHitHistograms(const std::string& type);

  void DrawDigiPerObjectHistograms(const std::string& type);

  void DrawHistograms(const std::string& type);

  void ScaleHistograms(const std::string& type);

  Bool_t fDaq;

  ClassDef(CbmStsTimeBasedQaReport, 1)
};

#endif
