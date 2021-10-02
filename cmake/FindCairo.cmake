FIND_PATH(CAIRO_INCLUDE_DIRS
    NAMES cairo.h
    PATH_SUFFIXES include/cairo include
	PATHS
	/usr
	"extern/cairo"
)

FIND_LIBRARY(CAIRO_LIBRARIES
    NAMES cairo
	PATH_SUFFIXES cairo lib/x64
	PATHS
	/usr
	"extern/cairo"
)


INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Cairo DEFAULT_MSG CAIRO_INCLUDE_DIRS CAIRO_LIBRARIES)
