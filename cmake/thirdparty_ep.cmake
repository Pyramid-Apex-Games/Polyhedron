set(BUILD_SHARED_LIBS OFF)
set(DEPENDENCY_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)

include(FetchContent)
include(ExternalProject)

if (NOT ANDROID)
    find_package(OpenGL REQUIRED)
    find_package(Iconv REQUIRED)
endif()

ExternalProject_Add(SDL2_proj
    URL             https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2-2.0.12"
)

ExternalProject_Get_Property(SDL2_proj install_dir)
add_library(SDL2 STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY ${install_dir}/include/SDL2)
set_target_properties(SDL2 PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libSDL2.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/SDL2
)

add_dependencies(SDL2 SDL2_proj)


ExternalProject_Add(OGG_proj
    URL             http://downloads.xiph.org/releases/ogg/libogg-1.3.4.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libogg-1.3.4"
    CMAKE_CACHE_ARGS
                    -DBUILD_FRAMEWORK:BOOL=OFF
                    -DINSTALL_DOCS:BOOL=OFF
                    -DINSTALL_PKG_CONFIG_MODULE:BOOL=OFF
                    -DINSTALL_CMAKE_PACKAGE_MODULE:BOOL=ON
    PATCH_COMMAND   patch -p0 -i "${CMAKE_CURRENT_LIST_DIR}/patch-libogg-add-stdint-h.diff"
)

ExternalProject_Get_Property(OGG_proj install_dir)
add_library(OGG STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY ${install_dir}/include/ogg)
set_target_properties(OGG PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libogg.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/ogg
)

add_dependencies(OGG OGG_proj)


ExternalProject_Add(Vorbis_proj
    URL             https://github.com/xiph/vorbis/archive/v1.3.6.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libvorbis-1.3.6"
)

ExternalProject_Get_Property(Vorbis_proj install_dir)
add_library(Vorbis STATIC IMPORTED GLOBAL)
add_library(Vorbisfile STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY ${install_dir}/include/vorbis)
set_target_properties(Vorbis PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libvorbis.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/vorbis
)
set_target_properties(Vorbisfile PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libvorbisfile.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/vorbis
)

add_dependencies(Vorbis Vorbis_proj)


ExternalProject_Add(Flac_proj
    URL             https://github.com/xiph/flac/archive/1.3.3.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libflac-1.3.3"
    CMAKE_CACHE_ARGS
                    -DBUILD_EXAMPLES:BOOL=OFF
                    -DBUILD_TESTING:BOOL=OFF
                    -DBUILD_CXXLIBS:BOOL=OFF
                    -DENABLE_64_BIT_WORDS:BOOL=ON
)

ExternalProject_Get_Property(Flac_proj install_dir)
add_library(Flac STATIC IMPORTED GLOBAL)
file(MAKE_DIRECTORY ${install_dir}/include/flac)
set_target_properties(Flac PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libflac.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include
)

add_dependencies(Flac Flac_proj)

ExternalProject_Add(SDL2_mixer_proj
    URL             https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
                    -DCMAKE_MODULE_PATH:PATH=${install_dir}/lib/cmake
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2_mixer-2.0.4"
    DEPENDS         SDL2 OGG Flac
    UPDATE_COMMAND  cp
                    "${CMAKE_CURRENT_LIST_DIR}/SDL2_mixer-2.0.4_CMakeLists.txt"
                    "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2_mixer-2.0.4/CMakeLists.txt"
)

ExternalProject_Get_Property(SDL2_mixer_proj install_dir)
add_library(SDL2_mixer STATIC IMPORTED GLOBAL)
set_target_properties(SDL2_mixer PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libsdl2_mixer.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/SDL2
)

add_dependencies(SDL2_mixer SDL2_mixer_proj)


ExternalProject_Add(SDL2_image_proj
    URL             https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
    PREFIX          "${CMAKE_INSTALL_PREFIX}"
    INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}"
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
                    -DCMAKE_MODULE_PATH:PATH=${install_dir}/lib/cmake
    DOWNLOAD_DIR    "${DEPENDENCY_DOWNLOAD_DIR}"
    SOURCE_DIR      "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2_image-2.0.5"
    DEPENDS         SDL2
    UPDATE_COMMAND  cp
                    "${CMAKE_CURRENT_LIST_DIR}/SDL2_Image-2.0.5_CMakeLists.txt"
                    "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2_image-2.0.5/CMakeLists.txt"
    &&
                    cp
                    "${CMAKE_CURRENT_LIST_DIR}/jpeg-9b_CMakeLists.txt"
                    "${CMAKE_CURRENT_BINARY_DIR}/thirdparty/SDL2_image-2.0.5/external/jpeg-9b/CMakeLists.txt"
)

ExternalProject_Get_Property(SDL2_image_proj install_dir)
add_library(SDL2_image STATIC IMPORTED GLOBAL)
set_target_properties(SDL2_image PROPERTIES
    IMPORTED_LOCATION ${install_dir}/lib/libSDL2_image.a
    INTERFACE_INCLUDE_DIRECTORIES ${install_dir}/include/SDL2
)

add_dependencies(SDL2_image SDL2_image_proj)


#find_package(SDL2 CONFIG REQUIRED)
#find_package(SDL_image CONFIG REQUIRED)
#find_package(libogg CONFIG REQUIRED)
#find_package(vorbis CONFIG REQUIRED)
#find_package(FLAC CONFIG REQUIRED)
#find_package(SDL_mixer CONFIG REQUIRED)
#find_package(OpenGL REQUIRED)

list(APPEND THIRDPARTY_LIBRARIES
    ${OPENGL_LIBRARIES}
    ${Iconv_LIBRARY}
    SDL2
    SDL2_image
    SDL2_mixer
    Flac
    ogg
    vorbis
    vorbisfile
    webp
    tiff
    jpeg
    z
    png
#    mpg123
)

list(APPEND THIRDPARTY_LINK_DIRECTORIES
    ${install_dir}/lib
)

if (ANDROID)
    list(APPEND THIRDPARTY_LIBRARIES
        log
        GLESv2
        EGL
        android
    )
endif()

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