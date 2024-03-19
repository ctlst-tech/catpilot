cmake_minimum_required(VERSION 3.15)

SET(CMAKE_SYSTEM_NAME QNX)

SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")
SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

set(QNX_VERSION "650")
set(QNX_VERSION_FULL "6.5.0")
set(QNX_GCC_VERSION "4.4.2")
set(QNX_ARCH "armv7")

set(QNX_HOST /opt/qnx${QNX_VERSION}/host/linux/x86)
set(QNX_TARGET /opt/qnx${QNX_VERSION}/target/qnx6)
set(QNX_MAP_CREATION_FLAG ON)

string(CONCAT COMMON_FLAGS
    # "-Werror -pedantic-errors "
    "-Wall -Wextra "
    "-std=gnu99 "
    # "-Wpedantic "
    # "-Wcast-align "
    # "-Wcast-qual "
    # "-Wduplicated-branches "
    # "-Wduplicated-cond "
    # "-Wfloat-equal "
    # "-Wlogical-op "
    "-Wredundant-decls "
    # "-Wsign-conversion "
    # "-Wconversion "
    "-fdata-sections -ffunction-sections -Wl,--gc-sections "
    "-Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-missing-braces "
)

FIND_PATH(QNX_HOST
    NAME usr/bin/qcc${HOST_EXECUTABLE_SUFFIX}
    PATHS $ENV{QNX_HOST} /opt/qnx${CMAKE_SYSTEM_VERSION_WD}/host/linux/
    NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH
)

FIND_PATH(QNX_TARGET
    NAME usr/include/qnx_errno.h
    PATHS $ENV{QNX_TARGET} /opt/qnx${CMAKE_SYSTEM_VERSION_WD}/target/qnx6/
    NO_CMAKE_PATH
    NO_CMAKE_ENVIRONMENT_PATH
)

SET(CMAKE_MAKE_PROGRAM "${QNX_HOST}/usr/bin/make"                        CACHE PATH "QNX make Program")
SET(CMAKE_SH           "${QNX_HOST}/usr/bin/sh${HOST_EXECUTABLE_SUFFIX}" CACHE PATH "QNX shell Program")
SET(CMAKE_AR           "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-ar"           CACHE PATH "QNX ar Program")
SET(CMAKE_RANLIB       "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-ranlib"       CACHE PATH "QNX ranlib Program")
SET(CMAKE_NM           "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-nm"           CACHE PATH "QNX nm Program")
SET(CMAKE_OBJCOPY      "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-objcopy"      CACHE PATH "QNX objcopy Program")
SET(CMAKE_OBJDUMP      "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-objdump"      CACHE PATH "QNX objdump Program")
SET(CMAKE_LINKER       "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-ld"           CACHE PATH "QNX linker Program")
SET(CMAKE_STRIP        "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-strip"        CACHE PATH "QNX strip Program")
SET(CMAKE_SIZE         "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-size"         CACHE PATH "QNX size Program")

SET(CMAKE_C_COMPILER_WORKS TRUE)
SET(CMAKE_C_COMPILER "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-gcc-${QNX_GCC_VERSION}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g3 ${COMMON_FLAGS}")
SET(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG ${COMMON_FLAGS}")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_FLAGS}")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g3 ${COMMON_FLAGS}")

SET(CMAKE_CXX_COMPILER_WORKS TRUE)
SET(CMAKE_CXX_COMPILER "${QNX_HOST}/usr/bin/nto${QNX_ARCH}-g++-${QNX_GCC_VERSION}")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${COMMON_FLAGS}")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g3 ${COMMON_FLAGS}")

if (QNX_MAP_CREATION_FLAG STREQUAL "ON")
    set(LINKER_MAP_CREATION_FLAG "-Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map")
endif()

set(LINKER_FLAGS "${LINKER_MAP_CREATION_FLAG}")
SET(CMAKE_EXE_LINKER_FLAGS_INIT "${LINKER_FLAGS} -O0  -g0")
    
SET(CMAKE_FIND_ROOT_PATH ${QNX_TARGET})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
