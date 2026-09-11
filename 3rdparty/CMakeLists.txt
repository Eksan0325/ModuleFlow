
SET(PRO_3RDPARTY_PATH ${ROKAE_3RD_DIR})

message(STATUS "In 3rdparty dir, SYSROOT_PATH = ${SYSROOT_PATH}")
message(STATUS "xCore HINT DIR = ${ROKAE_3RD_DIR}")

# ----------------------------Prepare other required 3rdparty libs--------------------------
ADD_LIBRARY(3rdparty_libs INTERFACE)

find_package(G3log REQUIRED HINTS ${ROKAE_3RD_DIR})
ADD_LIBRARY(g3log_lib STATIC IMPORTED)
SET_TARGET_PROPERTIES(g3log_lib
    PROPERTIES
        IMPORTED_LOCATION ${G3log_LIBS}
        INTERFACE_INCLUDE_DIRECTORIES ${G3log_INCLUDE_DIRS}
        # xenomai平台下，使用linker wrapper规避gettimeofday/clock_gettime导致的live lock问题
        #INTERFACE_LINK_OPTIONS $<$<STREQUAL:${OROCOS_TARGET},xenomai>:-Wl,-wrap,gettimeofday>
        NO_SYSTEM_FROM_IMPORTED ON
)

TARGET_COMPILE_DEFINITIONS(3rdparty_libs INTERFACE
        #SQLITE_HAS_CODEC
)



find_package(Boost REQUIRED COMPONENTS filesystem thread wave regex iostreams serialization HINTS ${ROKAE_3RD_DIR})
find_package(GFlags REQUIRED HINTS ${ROKAE_3RD_DIR})
find_package(Libevent REQUIRED HINTS ${ROKAE_3RD_DIR})
find_package(Evpp REQUIRED HINTS ${ROKAE_3RD_DIR})
find_package(Glog REQUIRED HINTS ${ROKAE_3RD_DIR})

find_package(Openssl REQUIRED HINTS ${ROKAE_3RD_DIR})
find_package(Jsoncpp REQUIRED HINTS ${ROKAE_3RD_DIR})

find_package(Coredumper REQUIRED HINTS ${ROKAE_3RD_DIR})


TARGET_INCLUDE_DIRECTORIES(3rdparty_libs INTERFACE
        ${ROKAE_3RD_DIR}
        ${Jsoncpp_INCLUDE_DIRS}
        ${Evpp_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${Openssl_INCLUDE_DIRS}
        ${Glog_INCLUDE_DIRS}
        ${GFlags_INCLUDE_DIRS}
        ${Coredumper_INCLUDE_DIRS}
        ${Libevent_INCLUDE_DIRS}
        ${ROKAE_3RD_DIR}/evpp/include
        
)

TARGET_INCLUDE_DIRECTORIES(3rdparty_libs INTERFACE
    /usr/include
)

message(STATUS "Libevent = ${Libevent_LIBS}, EVPP = ${Evpp_LIBS}")
TARGET_LINK_LIBRARIES(3rdparty_libs INTERFACE
            ${Jsoncpp_LIBS}
            ${ROKAE_3RD_DIR}/jsoncpp/lib/libjsoncpp.a
            ${GFlags_LIBS}
            ${Glog_LIBS}
            ${Coredumper_LIBS}
            ${Openssl_LIBS}
            ${Evpp_LIBS}
            ${Libevent_LIBS}
            ${Boost_LIBRARIES}
            g3log_lib
            pthread
            z
            m
            dl
        rt
        tinfo
        dl
)
