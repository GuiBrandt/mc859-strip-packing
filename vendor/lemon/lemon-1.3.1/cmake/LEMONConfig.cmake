SET(LEMON_INCLUDE_DIR "/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/include" CACHE PATH "LEMON include directory")
SET(LEMON_INCLUDE_DIRS "${LEMON_INCLUDE_DIR}")

IF(UNIX)
  SET(LEMON_LIB_NAME "libemon.a")
ELSEIF(WIN32)
  SET(LEMON_LIB_NAME "lemon.lib")
ENDIF(UNIX)

SET(LEMON_LIBRARY "/home/guigb/workspace/unicamp/mc859/mc859-strip-packing/lemon/lemon-1.3.1/lib/${LEMON_LIB_NAME}" CACHE FILEPATH "LEMON library")
SET(LEMON_LIBRARIES "${LEMON_LIBRARY}")

MARK_AS_ADVANCED(LEMON_LIBRARY LEMON_INCLUDE_DIR)
