FIND_PATH(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
	HINTS
	${SDL2}
	$ENV{SDL2}
	$ENV{SDL2_TTF}
	PATH_SUFFIXES include/SDL2 include SDL2
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include/SDL2
	/usr/include/SDL2
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	"extern/SDL2_ttf-2.0.15"
)

FIND_LIBRARY(SDL2_TTF_LIBRARY_TEMP
	NAMES SDL2_ttf
	HINTS
	${SDL2}
	$ENV{SDL2}
	$ENV{SDL2_TTF}
	PATH_SUFFIXES lib64 lib lib/x64
	PATHS
	/sw
	/opt/local
	/opt/csw
	/opt
	"extern/SDL2_ttf-2.0.15"
)

SET(SDL2_TTF_FOUND "NO")
	IF(SDL2_TTF_LIBRARY_TEMP)
	# Set the final string here so the GUI reflects the final state.
	SET(SDL2_TTF_LIBRARY ${SDL2_TTF_LIBRARY_TEMP} CACHE STRING "Where the SDL2_ttf Library can be found")
	# Set the temp variable to INTERNAL so it is not seen in the CMake GUI
	SET(SDL2_TTF_LIBRARY_TEMP "${SDL2_TTF_LIBRARY_TEMP}" CACHE INTERNAL "")
	SET(SDL2_TTF_FOUND "YES")
ENDIF(SDL2_TTF_LIBRARY_TEMP)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_TTF REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)

