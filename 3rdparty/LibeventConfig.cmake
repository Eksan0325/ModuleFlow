# third_party/eventConfig.cmake

# 自动路径定位
set(_LIBEVENT_ROOT "${CMAKE_CURRENT_LIST_DIR}/event" )
set(Libevent_ROOT "${_LIBEVENT_ROOT}")
message(STATUS "Found package LIBEVENT and it's root = ${Libevent_ROOT}")

# 头文件配置
set(Libevent_INCLUDE_DIRS
    "${_LIBEVENT_ROOT}/include"
    "${_LIBEVENT_ROOT}/include/event2"  # 适配libevent特殊头文件结构
)

# 库文件配置
set(Libevent_LIBRARY_DIR "${_LIBEVENT_ROOT}/lib")
file(GLOB Libevent_LIBS 
    "${Libevent_LIBRARY_DIR}/libevent.a"
)

## 使用libevent, 在项目主CMakeLists
#####################
#  find_package(Libevent REQUIRED)
#  # 链接示例
#  target_include_directories(your_target PRIVATE
#      ${Libevent_INCLUDE_DIRS}
#  )
#  
#  target_link_libraries(your_target
#      ${LIBEVENT_LIBS}
#  )
