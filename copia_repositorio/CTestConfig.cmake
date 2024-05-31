## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

set(CTEST_PROJECT_NAME lab0)
set(CTEST_NIGHTLY_START_TIME 01:00:00 UTC)

if(CMAKE_VERSION VERSION_GREATER 3.14)
  set(CTEST_SUBMIT_URL http://localhost:8080/submit.php?project=lab0)
else()
  set(CTEST_DROP_METHOD "http")
  set(CTEST_DROP_SITE "localhost:8080")
  set(CTEST_DROP_LOCATION "/submit.php?project=lab0")
endif()

set(CTEST_DROP_SITE_CDASH TRUE)

