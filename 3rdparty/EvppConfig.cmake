# third_party/EvppConfig.cmake

# 自适配路径
set(_Evpp_ROOT "${CMAKE_CURRENT_LIST_DIR}/evpp" )
set(Evpp_ROOT ${_Evpp_ROOT})
set(Evpp_INCLUDE_DIRS
    "${_Evpp_ROOT}/include"
    "${_Evpp_ROOT}/include/evpp"  # 适配evpp头文件层级
)
message(STATUS "Found package evpp and it's root = ${Evpp_ROOT}")
# 库文件处理

set(Evpp_LIBRARY_DIR "${_Evpp_ROOT}/lib")
set(Evpp_LIBS "${Evpp_LIBRARY_DIR}/libevpp_static.a")


## evpp, 在项目主CMakeLists 但是evpp需要使用libevnet ，所以都得包含
#####################
#  find_package(Libevent REQUIRED)
#  find_package(Evpp REQUIRED)
#  # 链接示例
#target_include_directories(your_target PRIVATE
#      ${Evpp_INCLUDE_DIRS}
#  )
#  
#  target_link_libraries(your_target
#      ${Libevent_LIBS}
#      ${Evpp_LIBS}
#  )
