# 3rdparty/JsoncppConfig.cmake

# Auto Define Path
set(_Jsoncpp_ROOT "${CMAKE_CURRENT_LIST_DIR}/jsoncpp" )
set(Jsoncpp_ROOT "${_Jsoncpp_ROOT}")
message(STATUS "Found package jsoncpp and it's root = ${Jsoncpp_ROOT}")
# Header Config
set(Jsoncpp_INCLUDE_DIRS
    "${_Jsoncpp_ROOT}/include"
)

# Library Config
set(Jsoncpp_LIBRARY_DIR "${_Jsoncpp_ROOT}/lib")
file(GLOB Jsoncpp_LIBS 
    "${Jsoncpp_LIBRARY_DIR}/libjsoncpp.a"
)
message(STATUS "Found package jsoncpp lib and it's root = ${Jsoncpp_LIBS}")
