# CMakeList file for library libSts.
# Last update: V. Friese, 17.02.2014


# -----   Specify library name   --------------------------
Set(LIBRARY_NAME CbmSts)
# ---------------------------------------------------------


# -----  Specify sources for compilation   ----------------
# --- Sources in setup
set(SRCS_SETUP
setup/CbmStsElement.cxx
setup/CbmStsModule.cxx
setup/CbmStsSensor.cxx
setup/CbmStsSensorConditions.cxx
setup/CbmStsSensorPoint.cxx
setup/CbmStsSetup.cxx
setup/CbmStsStation.cxx
setup/CbmHodoSetup.cxx
)
# --- Sources in mc
set(SRCS_MC
mc/CbmStsAcceptance.cxx
mc/CbmStsMC.cxx
mc/CbmStsSensorFactory.cxx
mc/CbmStsMCQa.cxx
)
# --- Sources in digitize
set (SRCS_DIGITIZE
digitize/CbmStsDigitize.cxx
digitize/CbmStsDigitizeQa.cxx
digitize/CbmStsDigitizeQaReport.cxx
digitize/CbmStsDigitizeParameters.cxx
digitize/CbmStsPhysics.cxx
digitize/CbmStsSensorDssd.cxx
digitize/CbmStsSensorDssdOrtho.cxx
digitize/CbmStsSensorDssdStereo.cxx
digitize/CbmStsSignal.cxx;
)
# --- Sources in reco
set (SRCS_RECO
reco/CbmStsClusterAnalysis.cxx
reco/CbmStsClusterFinderModule.cxx
reco/CbmStsFindClusters.cxx
reco/CbmStsFindHits.cxx
reco/CbmStsFindHitsSingleCluster.cxx
reco/CbmStsDigisToHitsModule.cxx
reco/CbmStsDigisToHits.cxx
reco/CbmStsFindTracksEvents.cxx
reco/CbmStsMatchReco.cxx
reco/CbmStsReco.cxx
reco/CbmStsRecoQa.cxx
reco/CbmStsTestQa.cxx
#reco/CbmStsTimeBasedQa.cxx   has to be modified
#reco/CbmStsTimeBasedQaReport.cxx   has to be modified
)
# --- Sources in ana
set (SRCS_ANA
ana/CbmStsWkn.cxx
)
# --- All sources
set(SRCS
${SRCS_SETUP}
${SRCS_MC}
${SRCS_DIGITIZE}
${SRCS_RECO}
${SRCS_ANA}
CbmStsContFact.cxx
CbmStsFindTracks.cxx
CbmStsFindTracksQa.cxx
CbmStsFitTracks.cxx
CbmStsHitProducerIdeal.cxx
#CbmStsIdealMatchHits.cxx
#CbmStsMatchHits.cxx
CbmStsMatchTracks.cxx
CbmStsRadTool.cxx
CbmStsTrackFinderIdeal.cxx
CbmStsTrackFitterIdeal.cxx
CbmStsSimulationQa.cxx
#CbmStsReconstructionQa.cxx
#CbmStsDigitizeQa.cxx
)
# -----  End of sources   ---------------------------------



# ----  Specify include directories -----------------------
set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/sts
${CBMROOT_SOURCE_DIR}/sts/setup
${CBMROOT_SOURCE_DIR}/sts/mc
${CBMROOT_SOURCE_DIR}/sts/digitize
${CBMROOT_SOURCE_DIR}/sts/reco

# needed by MatchHits???
${CBMROOT_SOURCE_DIR}/passive

${CBMBASE_DIR}
${CBMROOT_SOURCE_DIR}/sim/transport/base

${CBMROOT_SOURCE_DIR}/run

${CBMDATA_DIR}
${CBMDATA_DIR}/sts

# needed by run/CbmMCBuffer.h
${CBMDATA_DIR}/mvd
${CBMDATA_DIR}/rich
${CBMDATA_DIR}/much
${CBMDATA_DIR}/trd
${CBMDATA_DIR}/tof
${CBMDATA_DIR}/ecal
${CBMDATA_DIR}/psd
${CBMDATA_DIR}/global


# for CbmStsTimeBasedQaReport
${CBMBASE_DIR}/report
${CBMBASE_DIR}/draw
${CBMBASE_DIR}/utils
)

set(SYSTEM_INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES}
)
# ----  End of include directories ------------------------


# ----  Specify link directories --------------------------
set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
)
# -----  End of link directories   ------------------------


# -----   Specify library dependences   -------------------
Set(DEPENDENCIES
    CbmPassive CbmBase CbmData Base
)
# ---------------------------------------------------------


# -----  Specify LinkDef file   ---------------------------
set(LINKDEF ${LIBRARY_NAME}LinkDef.h)
# ---------------------------------------------------------


# ---- Enable OpenMP  -------------------------------------
find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
# ---------------------------------------------------------


# -----  Let cmake do the job   ---------------------------
include_directories( ${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})
link_directories( ${LINK_DIRECTORIES})
GENERATE_LIBRARY()
# ---------------------------------------------------------


Install(FILES digitize/CbmStsSignal.h
              digitize/CbmStsDigitizeParameters.h
              digitize/CbmStsPhysics.h
        DESTINATION include/digitize
       )
Install(FILES setup/CbmStsSensor.h
              setup/CbmStsElement.h
              setup/CbmStsModule.h
              setup/CbmStsSensorConditions.h
              setup/CbmStsSensorPoint.h
              setup/CbmStsSetup.h
        DESTINATION include/setup
       )
Install(FILES mc/CbmStsTrackStatus.h 
        DESTINATION include
       )
Install(FILES mc/CbmStsSensorFactory.h
        DESTINATION include/mc
       )

