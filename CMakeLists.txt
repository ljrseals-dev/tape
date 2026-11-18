cmake_minimum_required(VERSION 3.16)
project(jh24-tape-model VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(JH24_BUILD_EXAMPLES "Build example applications" ON)
option(JH24_BUILD_SHARED   "Build shared library" OFF)

# Library sources
set(JH24_SOURCES
    src/TapeMagnetization.cpp
    src/TapeEqualization.cpp
    src/TapeLossModel.cpp
    src/TapeTransport.cpp
    src/TapeNoise.cpp
    src/TapeProcessor.cpp
    src/Oversampler.cpp
    src/Utilities.cpp
)

set(JH24_HEADERS
    include/jh24/TapeTypes.hpp
    include/jh24/TapeCalibration.hpp
    include/jh24/TapeMagnetization.hpp
    include/jh24/TapeEqualization.hpp
    include/jh24/TapeLossModel.hpp
    include/jh24/TapeTransport.hpp
    include/jh24/TapeNoise.hpp
    include/jh24/TapeProcessor.hpp
    include/jh24/Oversampler.hpp
    include/jh24/Utilities.hpp
    include/jh24/jh24.hpp
)

if(JH24_BUILD_SHARED)
    add_library(jh24 SHARED ${JH24_SOURCES} ${JH24_HEADERS})
else()
    add_library(jh24 STATIC ${JH24_SOURCES} ${JH24_HEADERS})
endif()

target_include_directories(jh24
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(jh24 PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -O3>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)

# Example
if(JH24_BUILD_EXAMPLES)
    add_executable(jh24_demo examples/demo.cpp)
    target_link_libraries(jh24_demo PRIVATE jh24)
    
    add_executable(jh24_offline examples/offline_render.cpp)
    target_link_libraries(jh24_offline PRIVATE jh24)
endif()

# Install
include(GNUInstallDirs)
install(TARGETS jh24
    EXPORT jh24Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT jh24Targets
    FILE jh24Targets.cmake
    NAMESPACE jh24::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jh24
)