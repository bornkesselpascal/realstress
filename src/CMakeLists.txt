cmake_minimum_required(VERSION 3.5)

project(RealStress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NETWOKRK STRESS TESTING (iPerf3)
option(WITH_NETWORK "Build with support for network stress testing." OFF)

add_executable(RealStress main.cpp
    stress.h stress.cpp)

if(WITH_NETWORK)
    target_sources(RealStress PRIVATE network.h network.cpp)
    target_compile_definitions(RealStress PRIVATE NETWORK_ENABLED)
endif()

include(GNUInstallDirs)
install(TARGETS RealStress
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
