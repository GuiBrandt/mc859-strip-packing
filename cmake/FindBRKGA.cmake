find_path(BRKGA_INCLUDE_DIR
    NAMES brkga_mp_ipr/brkga_mp_ipr.hpp
    HINTS ${PROJECT_SOURCE_DIR}/vendor/brkga_mp_ipr_cpp $ENV{BRKGA_HOME}
    DOC "BRKGA-MP-IPR include path")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BRKGA DEFAULT_MSG BRKGA_INCLUDE_DIR)
