project(KoebeIteration)

if(APPLE)
  add_definitions(-DMAC_OS)
endif(APPLE)

# Include the 'include' of the project itself
include_directories("include")
# Include MeshLib core and Eigen in 3rdparty
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/include"
                    "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/include"
                    "${MeshLib_DIR}/core"
                    "${CMAKE_SOURCE_DIR}/3rdparty/eigen")

# Include the directory of OpenGL and GLUT
if(MSVC)
  include_directories("${freeglut_DIR}/include")
else(MSVC)
  include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
endif(MSVC)

# Manually add the sources
set(SOURCES_ONE ${MeshLib_DIR}/core/bmp/RgbImage.cpp)

# Also, the file(GLOB...) allows for wildcard additions
file(GLOB SOURCES_TWO
    "include/*.h"
    "src/*.cpp")

# Add an executable target called KoebeIteration to be built from 
# the source files.
add_executable(KoebeIteration ${SOURCES_ONE} ${SOURCES_TWO})

# Link the libraries of freeglut
if(MSVC)
  if(CMAKE_CL_64)
    target_link_libraries(KoebeIteration 
                  "${freeglut_DIR}/lib/x64/freeglut.lib"
            debug "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/windows/x64/Delaunayd.lib"
        optimized "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/windows/x64/Delaunay.lib"
            debug "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/windows/x64/Hodged.lib"
        optimized "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/windows/x64/Hodge.lib")
  else(CMAKE_CL_64)
    target_link_libraries(KoebeIteration 
                  "${freeglut_DIR}/lib/freeglut.lib"
            debug "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/windows/Win32/Delaunayd.lib"
        optimized "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/windows/Win32/Delaunay.lib"
            debug "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/windows/Win32/Hodged.lib"
        optimized "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/windows/Win32/Hodge.lib")
  endif(CMAKE_CL_64)
else(MSVC)
  target_link_libraries(KoebeIteration ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
  if(APPLE)
    # not support now
    message(FATAL_ERROR "Not support MAC OS now.")
  else(APPLE)
    target_link_libraries(KoebeIteration
                    debug "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/linux/libDelaunayd.so"
                optimized "${CMAKE_SOURCE_DIR}/3rdparty/Delaunay/lib/linux/libDelaunay.so"
                    debug "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/linux/libHodged.so"
                optimized "${CMAKE_SOURCE_DIR}/3rdparty/Hodge/lib/linux/libHodge.so")
  endif(APPLE)
endif(MSVC)

# Install the executeable program in the bin folder of the
# current project directory.
install(TARGETS KoebeIteration DESTINATION ${CMAKE_SOURCE_DIR}/bin)
