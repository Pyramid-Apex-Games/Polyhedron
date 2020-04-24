#CMake Configuration for Tesseract "The Right Way (tm)"
cmake_minimum_required(VERSION 3.13)
project(Polyhedron)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(PYTHON python3)
if (APPLE)
    set(PYTHON /usr/local/bin/python3)
endif()

option(SCHIZOMANIA_BUILD_CLIENT "Build the game client" ON)
option(SCHIZOMANIA_BUILD_SERVER "Build the game server" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

#For OSX, find the Shared libraries and ignore the static ones
if (APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
    list(REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 ".dylib" )

    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_LANGUAGE_STANDARD "c++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++14")
endif()

#Libraries
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(WEBP REQUIRED)
find_package(Freetype REQUIRED)
#For Linux, prime the SDL2 finder by setting the include dir
if (UNIX AND NOT APPLE)
    set(SDL2_INCLUDE_DIR "/usr/include/SDL2")
endif()
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(OpenGL REQUIRED)

if (APPLE)
    find_library(COREAUDIO CoreAudio REQUIRED)
    find_library(AVFOUNDATION AVFoundation REQUIRED)
    find_library(COCOA Cocoa REQUIRED)
    find_library(COREFOUNDATION CoreFoundation REQUIRED)
    find_library(APPLICATIONSERVICES ApplicationServices REQUIRED)
    find_library(FOUNDATION Foundation REQUIRED)
    find_library(COREFOUNDATION CoreFoundation REQUIRED)
    find_library(AUDIOTOOLBOX AudioToolbox REQUIRED)
    find_library(BULLSHITLIB BullShitLib REQUIRED)
endif()

#defines, project wide
#Silence OpenGL deprecations .. yes Apple, you like Metal and you can't shutup about it .. :rolleyes:
add_definitions("-DGL_SILENCE_DEPRECATION")
include_directories(
    ext/json-develop/single_include
)

#The engine, server and shared sub projects
add_subdirectory(enet)

list(APPEND fileListEngine
    engine/aa.cpp
    engine/animmodel.h
    engine/bih.cpp
    engine/bih.h
    engine/blend.cpp
    engine/client.cpp
    engine/command.cpp
    engine/console.cpp
    engine/dynlight.cpp
    engine/engine.h
    engine/explosion.h
    engine/grass.cpp
    engine/help.h
    engine/help.cpp
    engine/hitzone.h
    engine/iqm.h
    engine/lensflare.h
    engine/light.cpp
    engine/light.h
    engine/lightning.h
    engine/log.h
    engine/log.cpp
    engine/main.cpp
    # engine/master.cpp
    engine/material.cpp
    engine/md2.h
    engine/md3.h
    engine/md5.h
    engine/menus.cpp
    engine/model.h
    engine/movie.cpp
    engine/mpr.h
    engine/normal.cpp
    engine/obj.h
    engine/octa.cpp
    engine/octa.h
    engine/octaedit.cpp
    engine/octarender.cpp
    engine/pch.cpp
    engine/physics.cpp
    engine/pvs.cpp
    engine/ragdoll.h
    engine/rendergl.cpp
    engine/renderlights.cpp
    engine/rendermodel.cpp
    engine/renderparticles.cpp
    engine/rendersky.cpp
    engine/rendertext.cpp
    engine/renderva.cpp
    engine/scriptexport.h
    engine/scriptexport.cpp
    engine/server.cpp
    engine/serverbrowser.cpp
    engine/shader.cpp
    engine/skelmodel.h
    engine/smd.h
    engine/sound.cpp
    engine/stain.cpp
    engine/textedit.h
    engine/texture.cpp
    engine/texture.h
    engine/ui.cpp
    engine/vertmodel.h
    engine/water.cpp
    engine/world.cpp
    engine/world.h
    engine/worldio.cpp
)

list(APPEND fileListShared
    shared/command.h
    shared/crypto.cpp
    shared/cube.h
    shared/ents.h
    shared/geom.cpp
    shared/geom.h
    shared/glemu.cpp
    shared/glemu.h
    shared/glexts.h
    shared/iengine.h
    shared/igame.h
    shared/pch.cpp
    shared/stream.cpp
    # shared/tessfont.c
    shared/tools.cpp
    shared/tools.h
    shared/zip.cpp
)

list(APPEND fileListGame
    game/entities.cpp
    game/game.cpp
    game/game.h
    game/render.cpp
    game/server.cpp
    game/commands.cpp
)

list(APPEND fileListGameEntities
    game/entities/basemapmodel.cpp
    game/entities/basemapmodel.h
    game/entities/door.cpp
    game/entities/door.h
    game/entities/dynamiclight.cpp
    game/entities/dynamiclight.h
    game/entities/basemonster.cpp
    game/entities/basemonster.h
    game/entities/playerstart.cpp
    game/entities/playerstart.h
    game/entities/player.cpp
    game/entities/player.h
)

list(APPEND fileListSharedEntities
    shared/entities/animinfo.h
    shared/entities/attributes.h
    shared/entities/attributes.cpp
    shared/entities/coreentity.h
    shared/entities/coreentity.cpp
    shared/entities/baseentity.h
    shared/entities/baseentity.cpp
    shared/entities/basedynamicentity.cpp
    shared/entities/basedynamicentity.h
    shared/entities/basephysicalentity.cpp
    shared/entities/basephysicalentity.h
    shared/entities/decalentity.h
    shared/entities/decalentity.cpp
    shared/entities/entityfactory.h
    shared/entities/entityfactory.cpp
)

list(APPEND fileList
    ${fileListEngine}
    ${fileListShared}
    ${fileListGame}
    ${fileListSharedEntities}
    ${fileListGameEntities}
)

function(GenerateBindingFor input_file ide_folder)
    if(${input_file} MATCHES "\.cpp$")
        execute_process(
            COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py cmake_output ${input_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            OUTPUT_VARIABLE input_file_target
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        get_filename_component(output_file_target ${CMAKE_CURRENT_BINARY_DIR}/${input_file_target} ABSOLUTE)

        message(">> ${input_file} => ${output_file_target} (${input_file_target})")

        add_custom_command(
            OUTPUT ${output_file_target}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py gen ${CMAKE_CURRENT_BINARY_DIR} ${input_file} ${output_file_target}
            COMMENT "Preprocessing ${input_file}"
            DEPENDS ${input_file}
        )

        set_source_files_properties(
            ${output_file_target}
            PROPERTIES
                GENERATED TRUE
        )

        list(APPEND generatedFileList ${output_file_target})
        set(generatedFileList ${generatedFileList} PARENT_SCOPE)

        source_group(${ide_folder} FILES ${output_file_target})
    endif()
endfunction()


function(StoreCompileFlagsFor input_file)
    if(${input_file} MATCHES "\.cpp$")
        execute_process(
            COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py cmake_output ${input_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            OUTPUT_VARIABLE input_file_target
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        get_filename_component(output_file_target ${CMAKE_CURRENT_BINARY_DIR}/${input_file_target} ABSOLUTE)
        get_filename_component(output_file_target ${CMAKE_CURRENT_BINARY_DIR}/${input_file_target} ABSOLUTE)

        get_source_file_property(input_file_loc ${input_file} LOCATION)
        get_source_file_property(input_file_cxxflags ${input_file_loc} ABSTRACT)
        message("${input_file} >>> ${input_file_cxxflags}")
        get_target_property(target_cxxflags Polyhedron INCLUDE_DIRECTORIES)
        message("target >>> ${target_cxxflags}")

        add_custom_command(
            OUTPUT ${output_file_target}_cxxflags
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py setflags ${output_file_target} \"${input_file_cxxflags}\"
            COMMENT "Set CXXFLAGS ${input_file} \"${input_file_cxxflags}\""
            DEPENDS ${input_file}
            VERBATIM
        )
    endif()
endfunction()

source_group("Sources\\Engine" FILES ${fileListEngine})
source_group("Sources\\Shared" FILES ${fileListShared})
source_group("Sources\\Game" FILES ${fileListGame})
source_group("Sources\\Entities\\Shared" FILES ${fileListSharedEntities})
source_group("Sources\\Entities\\Game" FILES ${fileListGameEntities})

# --- BINDINGS
foreach(input_file ${fileListEngine})
    GenerateBindingFor(${input_file} "Generated\\\\Engine")
endforeach()

foreach(input_file ${fileListShared})
    GenerateBindingFor(${input_file} "Generated\\\\Shared")
endforeach()

foreach(input_file ${fileListGame})
    GenerateBindingFor(${input_file} "Generated\\\\Game")
endforeach()

foreach(input_file ${fileListSharedEntities})
    GenerateBindingFor(${input_file} "Generated\\\\Entities\\\\Shared")
endforeach()

foreach(input_file ${fileListGameEntities})
    GenerateBindingFor(${input_file} "Generated\\\\Entities\\\\Game")
endforeach()


#Everything ready, now define the executables
if (APPLE)
    list(APPEND fileListXCode
        xcode/English.lproj/InfoPlist.strings
        xcode/English.lproj/MainMenu.xib
        xcode/Launcher.h
        xcode/Launcher.m
        xcode/main.m
        xcode/Polyhedron.plist    
        xcode/Polyhedron.xcassets
    )
    add_executable(Polyhedron MACOSX_BUNDLE
        ${generatedFileList}
        ${fileList}
        ${fileListXCode}
    )

    source_group("XCode Files" FILES ${fileListXCode})
else()
    add_executable(Polyhedron
        ${generatedFileList}
        ${fileList}
    )
endif()

#Link the libraries
target_include_directories(Polyhedron
    PRIVATE
        .
        shared
        game
        engine
		${CMAKE_SOURCE_DIR}
		${SDL2_INCLUDE_DIRS}
		${SDL2_MIXER_INCLUDE_DIRS}
		${SDL2_IMAGE_INCLUDE_DIRS}
		${FREETYPE_INCLUDE_DIRS}
		${ZLIB_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIRS}        
)

if (APPLE)
    target_link_libraries(Polyhedron
        enet

        ${COCOA}
        ${COREFOUNDATION}
        ${AUDIOTOOLBOX}
        ${COREAUDIO}

        ${SDL2_LIBRARIES}
        ${SDL2_MAIN_LIBRARIES}
		${SDL2_MIXER_LIBRARIES}
		${SDL2_IMAGE_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${OPENGL_LIBRARIES}     
        # ${AVFOUNDATION}
        # ${APLICATIONSERVICES}
        # ${FOUNDATION}
        # ${COREFOUNDATION}
    )
else()
    target_link_libraries(Polyhedron
        enet
        rt
        ${SDL2_LIBRARIES}
		${SDL2_MIXER_LIBRARIES}
		${SDL2_IMAGE_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${OPENGL_LIBRARIES}
    )
endif()

#Apple specific stuff
if (APPLE)
    set_property(TARGET Polyhedron PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/xcode/tesseract.plist.in)
    set_target_properties(Polyhedron PROPERTIES RESOURCE "${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.xib")
    target_compile_features(Polyhedron PUBLIC cxx_std_14)

    # set_source_files_properties(
    #     ${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.nib/designable.nib
    #     PROPERTIES
    #     MACOSX_PACKAGE_LOCATION Resources/MainMenu.nib
    # )

    # set_source_files_properties(
    #     ${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.nib/keyedobjects.nib
    #     PROPERTIES
    #     MACOSX_PACKAGE_LOCATION Resources/MainMenu.nib
    # )
endif()
