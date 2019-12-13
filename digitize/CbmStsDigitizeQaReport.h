#ifndef CBMSTSDIGITIZEQAREPORT_H_
#define CBMSTSDIGITIZEQAREPORT_H_

#include "CbmSimulationReport.h"

class CbmStsSetup;
class CbmStsDigitizeParameters;

class CbmStsDigitizeQaReport : public CbmSimulationReport
{
    public:
	CbmStsDigitizeQaReport(CbmStsSetup * setup, CbmStsDigitizeParameters * digipar);
	virtual ~CbmStsDigitizeQaReport();


    private:
	CbmStsSetup * fSetup;
	CbmStsDigitizeParameters * fDigiPar;
	virtual void Create();
	virtual void Draw();
	void DrawNofObjectsHistograms();
	void DrawLogHistograms();
	void DrawHistograms();
	void Draw2dHistograms();
	void ScaleHistograms();

	CbmStsDigitizeQaReport(const CbmStsDigitizeQaReport&);
	CbmStsDigitizeQaReport& operator=(const CbmStsDigitizeQaReport&);

	ClassDef(CbmStsDigitizeQaReport, 1)
};

#endif
