
set(OFFLINE_MODE 1)

set(BUILD_SHARED_LIBS OFF)
set(DEPENDENCY_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)

if (APPLE)
    cmake_policy(SET CMP0042 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0042 NEW)
endif()

include(FetchContent)
include(ExternalProject)

macro(CopyFileIfDifferent FromFile ToFile)
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files
        ${FromFile} ${ToFile} RESULT_VARIABLE compare_result
    )
    if(NOT compare_result EQUAL 1 OR NOT EXISTS "${ToFile}")
        get_filename_component(ToFileDir ${ToFile} DIRECTORY)
        execute_process(COMMAND mkdir -p ${ToFileDir})
        execute_process(COMMAND cp -r ${FromFile} ${ToFile} RESULT_VARIABLE cp_result)
        message("cp ${FromFile} ${ToFile}: ${cp_result}")
    else()
        message("Skip Copy[${compare_result}]: ${FromFile} ${ToFile}")
    endif()
endmacro()

if (OFFLINE_MODE)
    message("Reusing python")
    FetchContent_Declare(
            PYTHON
            SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/python
            INSTALL_COMMAND     ""
    )
else()
    message("Fetching python")
    FetchContent_Declare(
            PYTHON
            SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/python
            GIT_REPOSITORY      https://github.com/python-cmake-buildsystem/python-cmake-buildsystem
            INSTALL_COMMAND     ""
    )
endif()
FetchContent_GetProperties(PYTHON)
if (NOT python_POPULATED)
    source_group(TREE python)

    set(INSTALL_MANUAL OFF)
    set(USE_SYSTEM_LIBRARIES OFF)
    set(INSTALL_WINDOWS_TRADITIONAL OFF)
    set(BUILD_WININST OFF)
    set(BUILD_WININST_ALWAYS OFF)
    set(INSTALL_DEVELOPMENT OFF)
    set(INSTALL_MANUAL OFF)
    set(INSTALL_TEST OFF)
    set(_use_builtin_zlib_default OFF)
    set(_use_system_zlib_default OFF)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
    list(APPEND THIRDPARTY_INCLUDE_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Include
        ${CMAKE_CURRENT_BINARY_DIR}/_deps/python-build/bin
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/abc.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/abc.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/codecs.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/codecs.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/encodings/__init__.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/encodings/__init__.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/encodings/aliases.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/encodings/aliases.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/encodings/ascii.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/encodings/ascii.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/encodings/latin_1.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/encodings/latin_1.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/encodings/utf_8.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/encodings/utf_8.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/io.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/io.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/os.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/os.py"
    )

    CopyFileIfDifferent(
        "${CMAKE_CURRENT_BINARY_DIR}/_deps/Python-3.6.7/Lib/_weakrefset.py"
        "${CMAKE_CURRENT_LIST_DIR}/../config/site-py/_weakrefset.py"
    )

    FetchContent_MakeAvailable(PYTHON)
endif()

if (OFFLINE_MODE)
    message("Reusing dependency: SDL2")
    FetchContent_Declare(
        SDL2
        SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2-2.0.12"
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
    )
else()
    message("Fetching dependency: SDL2")
    FetchContent_Declare(
        SDL2
        SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2-2.0.12"
        URL                 https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
    )
endif()
FetchContent_GetProperties(SDL2)
if (NOT sdl2_POPULATED)
    source_group(TREE SDL2)

    FetchContent_MakeAvailable(SDL2)

    message("SDL2_SOURCE_DIR ${sdl2_SOURCE_DIR} ABSOLUTE BASE_DIR \"${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2-2.0.12\"")
    get_filename_component(SDL2_SOURCE_DIR ${sdl2_SOURCE_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2-2.0.12" CACHE)
    message("sdl2_SOURCE_DIR ${SDL2_SOURCE_DIR}")
    get_filename_component(SDL2_BINARY_DIR ${sdl2_BINARY_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2-2.0.12" CACHE)
    message("sdl2_BINARY_DIR ${SDL2_BINARY_DIR}")

    set(SDL2_DIR ${SDL2_SOURCE_DIR} CACHE PATH "SDL2_DIR" FORCE)
    set(SDL2_SOURCE_DIR ${SDL2_SOURCE_DIR} CACHE PATH "SDL2_SOURCE_DIR" FORCE)
    message("SDL2_DIR ${SDL2_DIR}")
#    add_subdirectory(${sdl2_SOURCE_DIR} ${sdl2_BINARY_DIR})
    list(APPEND THIRDPARTY_INCLUDE_DIRS
        ${SDL2_SOURCE_DIR}/include
        ${SDL2_BINARY_DIR}/include
    )
endif()

if (OFFLINE_MODE)
    message("Reusing dependency: OGG")
    FetchContent_Declare(
            OGG
            SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libogg-1.3.4"
            DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
            INSTALL_COMMAND     ""
    )
else()
    message("Fetching dependency: OGG")
    FetchContent_Declare(
            OGG
            SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libogg-1.3.4"
            URL                 https://ftp.osuosl.org/pub/xiph/releases/ogg/libogg-1.3.4.tar.gz
                                #http://downloads.xiph.org/releases/ogg/libogg-1.3.4.tar.gz
            DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
            INSTALL_COMMAND     ""
    )
endif()
FetchContent_GetProperties(OGG)
if (NOT ogg_POPULATED)
    source_group(TREE OGG)
    FetchContent_Populate(OGG)
    execute_process(COMMAND
            cmake -E chdir "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libogg-1.3.4"
            patch -p0 -i "${CMAKE_CURRENT_LIST_DIR}/patch-libogg-add-stdint-h.diff"
        RESULT_VARIABLE RESULT_PATCH
    )
    message(STATUS "Patching libogg: ${RESULT_PATCH}, SOURCE DIR: ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libogg-1.3.4")

    add_subdirectory(${ogg_SOURCE_DIR} ${ogg_BINARY_DIR})
    FetchContent_MakeAvailable(OGG)
    set(OGG_INCLUDE_DIRS "${ogg_SOURCE_DIR}/include" "${ogg_BINARY_DIR}/include")
    set(OGG_LIBRARIES ogg)
    set(OGG_INCLUDE_DIR ${OGG_INCLUDE_DIRS})
    set(OGG_LIBRARY ${OGG_LIBRARIES})
endif()

message("Fetching dependency: Vorbis")
FetchContent_Declare(
        VORBIS
        SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libvorbis-1.3.6"
        URL                 https://ftp.osuosl.org/pub/xiph/releases/vorbis/libvorbis-1.3.6.tar.gz
                            #https://github.com/xiph/vorbis/archive/v1.3.6.tar.gz
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
)
FetchContent_GetProperties(VORBIS)
if (NOT vorbis_POPULATED)
    source_group(TREE VORBIS)
    FetchContent_Populate(VORBIS)
    execute_process(COMMAND
        cmake -E chdir "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libvorbis-1.3.6"
        patch -p0 -i "${CMAKE_CURRENT_LIST_DIR}/patch-libvorbis-CMakeLists.txt.diff"
        RESULT_VARIABLE RESULT_PATCH1
    )
    execute_process(COMMAND
        cmake -E chdir "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libvorbis-1.3.6"
        patch -p0 -i "${CMAKE_CURRENT_LIST_DIR}/patch-libvorbis-lib-CMakeLists.txt.diff"
        RESULT_VARIABLE RESULT_PATCH2
    )
    message(STATUS "Patching libvorbis: 1: ${RESULT_PATCH1}, 2: ${RESULT_PATCH2}, SOURCE DIR: ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libogg-1.3.4")

    FetchContent_MakeAvailable(VORBIS)
endif()

if (NOT ANDROID)
    message("Fetching dependency: Flac")
    FetchContent_Declare(
            FLAC
            SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/flac-1.3.3"
            URL                 https://ftp.osuosl.org/pub/xiph/releases/flac/flac-1.3.2.tar.xz
                                #https://github.com/xiph/flac/archive/1.3.3.tar.gz
            DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
            INSTALL_COMMAND     ""
    )
    FetchContent_GetProperties(FLAC)
    if (NOT flac_POPULATED)
        source_group(TREE FLAC)
        set(BUILD_EXAMPLES OFF)
        set(BUILD_TESTING OFF)
        set(BUILD_CXXLIBS OFF)
        set(ENABLE_64_BIT_WORDS ON)
        FetchContent_MakeAvailable(FLAC)
    endif()
endif()

message("Fetching dependency: SDL2_mixer")
FetchContent_Declare(
        SDL2_mixer
        SOURCE_DIR          "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_mixer-2.0.4"
        URL                 https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
)
FetchContent_GetProperties(SDL2_mixer)
if (NOT sdl2_mixer_POPULATED)
    source_group(TREE SDL2_mixer)
    CopyFileIfDifferent(
        "${CMAKE_CURRENT_LIST_DIR}/SDL2_mixer-2.0.4_CMakeLists.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_mixer-2.0.4/CMakeLists.txt"
    )
    set(SDL_MIXER_INCLUDES ${sdl2_SOURCE_DIR}/include)
    set(SDL_MIXER_LIBRARIES SDL2-static)
    FetchContent_MakeAvailable(SDL2_mixer)

    get_filename_component(SDL2_MIXER_SOURCE_DIR ${sdl2_mixer_SOURCE_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_mixer-2.0.4" CACHE)
    get_filename_component(SDL2_MIXER_BINARY_DIR ${sdl2_mixer_BINARY_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_mixer-2.0.4" CACHE)

    list(APPEND THIRDPARTY_INCLUDE_DIRS
        ${SDL2_MIXER_SOURCE_DIR}
    )

endif()


if (WIN32)
    message("Fetching zlib")
    FetchContent_Declare(
        ZLIB
        SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/zlib
        URL                 https://www.zlib.net/fossils/zlib-1.2.11.tar.gz
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
    )
    FetchContent_GetProperties(ZLIB)
    if (NOT zlib_POPULATED)
        source_group(TREE ZLIB)
        FetchContent_Populate(ZLIB)

        set(ZLIB_LIBRARY zlibstatic)
        set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR})

        list(APPEND THIRDPARTY_LIBRARIES
            zlibstatic
        )
    endif()
endif()

message("Fetching dependency SDL2_image")
FetchContent_Declare(
    SDL2_image
    SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_image-2.0.5
    URL                 https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
    DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
    INSTALL_COMMAND     ""
)
FetchContent_GetProperties(SDL2_image)
if (NOT sdl2_image_POPULATED)
    FetchContent_Populate(SDL2_image)
    FetchContent_MakeAvailable(SDL2_image)
    CopyFileIfDifferent(
        "${CMAKE_CURRENT_LIST_DIR}/jpeg-9b_CMakeLists.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_image-2.0.5/external/jpeg-9b/CMakeLists.txt"
    )
    CopyFileIfDifferent(
        "${CMAKE_CURRENT_LIST_DIR}/SDL2_Image-2.0.5_CMakeLists.txt"
        "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_image-2.0.5/CMakeLists.txt"
    )

    get_filename_component(SDL2_IMAGE_SOURCE_DIR ${sdl2_image_SOURCE_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_image-2.0.5" CACHE)
    get_filename_component(SDL2_IMAGE_BINARY_DIR ${sdl2_image_BINARY_DIR} ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/SDL2_image-2.0.5" CACHE)

    list(APPEND THIRDPARTY_INCLUDE_DIRS
        ${SDL2_IMAGE_SOURCE_DIR}/include
        ${SDL2_IMAGE_BINARY_DIR}/include
    )

    set(SKIP_INSTALL_ALL ON)

    add_subdirectory(${sdl2_image_SOURCE_DIR} ${sdl2_image_BINARY_DIR})
endif()

if (WIN32)
    message("Fetching iconv")
    FetchContent_Declare(
        ICONV
        SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/iconv
        GIT_REPOSITORY      https://github.com/vovythevov/libiconv-cmake
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
    )
    FetchContent_GetProperties(ICONV)
    if (NOT iconv_POPULATED)
        source_group(TREE iconv)

        FetchContent_MakeAvailable(ICONV)
        set(Iconv_LIBRARY libiconv)
        set(Iconv_INCLUDE_DIR ${iconv_SOURCE_DIR})
    endif()
endif()

if(WIN32)
    message("downloading libclang prebuilt")
    FetchContent_Declare(
        LIBCLANG
        SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libclang
        URL                 https://ziglang.org/deps/llvm%2bclang%2blld-10.0.0-x86_64-windows-msvc-release-mt.tar.xz
        DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
        INSTALL_COMMAND     ""
    )
    FetchContent_GetProperties(LIBCLANG)
    if (NOT libclang_POPULATED)
        set(LIBCLANG_PATH "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libclang/bin/libclang.dll")
        FetchContent_MakeAvailable(LIBCLANG)
    endif()
endif()

message("downloading fmt")
FetchContent_Declare(
    LIBFMT
    SOURCE_DIR          ${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libfmt
    GIT_REPOSITORY      https://github.com/fmtlib/fmt.git
    GIT_TAG             6.2.1
    DOWNLOAD_DIR        ${DEPENDENCY_DOWNLOAD_DIR}
    INSTALL_COMMAND     ""
)
FetchContent_GetProperties(LIBFMT)
if (NOT libfmt_POPULATED)
    FetchContent_MakeAvailable(LIBFMT)
endif()

list(APPEND THIRDPARTY_INCLUDE_DIRS
)

list(APPEND THIRDPARTY_LINK_DIRS
)

list(APPEND THIRDPARTY_LIBRARIES
    SDL2-static
#    SDL2main
    SDL2_image
    SDL2_mixer
    libpython-static
    fmt::fmt
#    mpg123
)

if (APPLE)
    find_library(COREFOUNDATION CoreFoundation REQUIRED)
    find_library(FOUNDATION Foundation REQUIRED)

    find_library(COREAUDIO CoreAudio REQUIRED)
    find_library(COREVIDEO CoreVideo REQUIRED)
    find_library(AVFOUNDATION AVFoundation REQUIRED)
    find_library(COCOA Cocoa REQUIRED)
    find_library(APPLICATIONSERVICES ApplicationServices REQUIRED)
    find_library(AUDIOTOOLBOX AudioToolbox REQUIRED)
    find_library(FORCEFEEDBACK ForceFeedback REQUIRED)
    find_library(IOKIT IOKit REQUIRED)
    find_library(CARBON Carbon REQUIRED)

    list(APPEND THIRDPARTY_LIBRARIES
        ${COREFOUNDATION}
        ${FOUNDATION}
        ${COREAUDIO}
        ${COREVIDEO}
        ${AVFOUNDATION}
        ${COCOA}
        ${APPLICATIONSERVICES}
        ${AUDIOTOOLBOX}
        ${FORCEFEEDBACK}
        ${IOKIT}
        ${CARBON}
    )
endif()

if (ANDROID)
    list(APPEND THIRDPARTY_LIBRARIES
        log
        GLESv3
        EGL
        android
    )
endif()


if (NOT ANDROID)
    find_package(OpenGL REQUIRED)
    find_package(Iconv REQUIRED)

    list(APPEND THIRDPARTY_LIBRARIES
        ${OPENGL_LIBRARIES}
        ${Iconv_LIBRARY}
    )
endif()
