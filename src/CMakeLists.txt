cmake_minimum_required(VERSION 3.5)

project(RealStress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RealStress main.cpp
    stress.h stress.cpp
    iperf.h iperf.cpp)

include(GNUInstallDirs)
install(TARGETS RealStress
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
