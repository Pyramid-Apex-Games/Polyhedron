cmake_minimum_required(VERSION 3.13)
project(Polyhedron)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(PYTHON python3)
if (APPLE)
    set(PYTHON /usr/local/bin/python3)
endif()
if (WIN32)
    set(PYTHON py)
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#For OSX, find the Shared libraries and ignore the static ones
if (APPLE)
#    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
#    list(REMOVE_ITEM CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#    list(INSERT CMAKE_FIND_LIBRARY_SUFFIXES 0 ".dylib" )

    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_LANGUAGE_STANDARD "c++20")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++17")
endif()

include(../cmake/thirdparty.cmake)

#defines, project wide
#Silence OpenGL deprecations .. yes Apple, you like Metal and you can't shutup about it .. :rolleyes:
add_definitions("-DGL_SILENCE_DEPRECATION")
include_directories(
    ext/json-develop/include
    ext/nuklear
    ext/renderdoc
    ext/glad/include
    ext/imgui
)

CHECK_INCLUDE_FILE(renderdoc.h HAVE_RENDERDOC_H)
if (HAVE_RENDERDOC_H)
    add_definitions("-DHAVE_RENDERDOC_H")
    message("Found renderdoc.h -- including support")
    list(APPEND THIRDPARTY_LIBRARIES dl)
endif()

#The engine, server and shared sub projects
add_subdirectory(enet)

list(APPEND fileListEngine
    engine/aa.cpp
    engine/animmodel.h
    engine/animmodel.cpp
    engine/bih.cpp
    engine/bih.h
    engine/blend.cpp
    engine/Camera.cpp
    engine/Camera.h
    engine/client.cpp
    engine/command.cpp
    engine/console.cpp
    engine/dynlight.cpp
    engine/engine.h
    engine/explosion.h
    engine/GLFeatures.cpp
    engine/GLFeatures.h
    engine/engine/font.h
    engine/engine/font.cpp
    engine/grass.cpp
    engine/help.h
    engine/help.cpp
    engine/hitzone.h
    engine/hud.cpp
    engine/hud.h
    engine/iqm.h
    engine/lensflare.h
    engine/light.cpp
    engine/light.h
    engine/lightning.h
    engine/log.h
    engine/log.cpp
    engine/main.cpp
    engine/main/AppConfig.cpp
    engine/main/AppConfig.h
    engine/main/Application.cpp
    engine/main/Application.h
    engine/main/Clock.cpp
    engine/main/Clock.h
    engine/main/CommandlineArguments.cpp
    engine/main/CommandlineArguments.h
    engine/main/Compatibility.cpp
    engine/main/Compatibility.h
    engine/main/FPS.cpp
    engine/main/FPS.h
    engine/main/GLContext.cpp
    engine/main/GLContext.h
    engine/main/Input.cpp
    engine/main/Input.h
    engine/main/Renderer.cpp
    engine/main/Renderer.h
    engine/main/SoundConfig.cpp
    engine/main/SoundConfig.h
    engine/main/Window.cpp
    engine/main/Window.h
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
    engine/nui/nui.cpp
    engine/nui/nui.h
    engine/obj.h
    engine/octa.cpp
    engine/octa.h
    engine/octaedit.cpp
    engine/octarender.cpp
    engine/pch.cpp
    engine/physics.cpp
    engine/pvs.cpp
    engine/ragdoll.h
    engine/ragdoll.cpp
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
    shared/Easing.cpp
    shared/Easing.h
    shared/ents.h
    shared/geom.cpp
    shared/geom.h
    shared/geom/bvec.cpp
    shared/geom/bvec.h
    shared/geom/bvec4.cpp
    shared/geom/bvec4.h
    shared/geom/constants.h
    shared/geom/dualquat.cpp
    shared/geom/dualquat.h
    shared/geom/dvec4.cpp
    shared/geom/dvec4.h
    shared/geom/half.cpp
    shared/geom/half.h
    shared/geom/hvec.cpp
    shared/geom/hvec.h
    shared/geom/hvec2.cpp
    shared/geom/hvec2.h
    shared/geom/hvec4.cpp
    shared/geom/hvec4.h
    shared/geom/ivec.cpp
    shared/geom/ivec.h
    shared/geom/ivec2.cpp
    shared/geom/ivec2.h
    shared/geom/ivec4.cpp
    shared/geom/ivec4.h
    shared/geom/matrix.cpp
    shared/geom/matrix.h
    shared/geom/matrix2.cpp
    shared/geom/matrix2.h
    shared/geom/matrix3.cpp
    shared/geom/matrix3.h
    shared/geom/matrix4.cpp
    shared/geom/matrix4.h
    shared/geom/matrix4x3.cpp
    shared/geom/matrix4x3.h
    shared/geom/plane.cpp
    shared/geom/plane.h
    shared/geom/quat.cpp
    shared/geom/quat.h
    shared/geom/squat.cpp
    shared/geom/squat.h
    shared/geom/svec.cpp
    shared/geom/svec.h
    shared/geom/triangle.cpp
    shared/geom/triangle.h
    shared/geom/usvec.cpp
    shared/geom/usvec.h
    shared/geom/vec.cpp
    shared/geom/vec.h
    shared/geom/vec2.cpp
    shared/geom/vec2.h
    shared/geom/vec4.cpp
    shared/geom/vec4.h
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
    shared/tools/cubestr.h
    shared/tools/databuf.h
    shared/tools/endianness.h
    shared/tools/hash.h
    shared/tools/isclass.h
    shared/tools/loop.h
    shared/tools/macros.h
    shared/tools/packetbuf.h
    shared/tools/queue.h
    shared/tools/sort.h
    shared/tools/stream.h
    shared/tools/streambuf.h
    shared/tools/vector.h
    shared/tools/vector_util.cpp
    shared/tools/vector_util.h
    shared/zip.cpp
)

if (WITH_PYTHON)
    list(APPEND fileListShared
        shared/python/PythonScript.cpp
        shared/python/PythonScript.h
        shared/python/PythonPipeIO.cpp
        shared/python/PythonPipeIO.h
    )
endif()


list(APPEND fileListSharedGeom
    shared/geom/bvec.cpp
    shared/geom/bvec.h
    shared/geom/bvec4.cpp
    shared/geom/bvec4.h
    shared/geom/constants.h
    shared/geom/dualquat.cpp
    shared/geom/dualquat.h
    shared/geom/dvec4.cpp
    shared/geom/dvec4.h
    shared/geom/half.cpp
    shared/geom/half.h
    shared/geom/hvec.cpp
    shared/geom/hvec.h
    shared/geom/hvec2.cpp
    shared/geom/hvec2.h
    shared/geom/hvec4.cpp
    shared/geom/hvec4.h
    shared/geom/ivec.cpp
    shared/geom/ivec.h
    shared/geom/ivec2.cpp
    shared/geom/ivec2.h
    shared/geom/ivec4.cpp
    shared/geom/ivec4.h
    shared/geom/matrix.cpp
    shared/geom/matrix.h
    shared/geom/matrix2.cpp
    shared/geom/matrix2.h
    shared/geom/matrix3.cpp
    shared/geom/matrix3.h
    shared/geom/matrix4.cpp
    shared/geom/matrix4.h
    shared/geom/matrix4x3.cpp
    shared/geom/matrix4x3.h
    shared/geom/plane.cpp
    shared/geom/plane.h
    shared/geom/quat.cpp
    shared/geom/quat.h
    shared/geom/squat.cpp
    shared/geom/squat.h
    shared/geom/svec.cpp
    shared/geom/svec.h
    shared/geom/triangle.cpp
    shared/geom/triangle.h
    shared/geom/usvec.cpp
    shared/geom/usvec.h
    shared/geom/vec.cpp
    shared/geom/vec.h
    shared/geom/vec2.cpp
    shared/geom/vec2.h
    shared/geom/vec4.cpp
    shared/geom/vec4.h
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
        game/entities/ModelEntity.cpp
        game/entities/ModelEntity.h
        game/entities/DoorEntity.cpp
        game/entities/DoorEntity.h
        game/entities/LightEntity.cpp
        game/entities/LightEntity.h
        game/entities/MonsterEntity.cpp
        game/entities/MonsterEntity.h
        game/entities/PlayerSpawnEntity.cpp
        game/entities/PlayerSpawnEntity.h
        game/entities/SkeletalEntity.cpp
        game/entities/SkeletalEntity.h
        game/entities/SoundEntity.cpp
        game/entities/SoundEntity.h
)

list(APPEND fileListSharedEntities
    shared/entities/DecalEntity.cpp
    shared/entities/DecalEntity.h
    shared/entities/DynamicEntity.cpp
    shared/entities/DynamicEntity.h
    shared/entities/Entity.cpp
    shared/entities/Entity.h
    shared/entities/EntityEditorMenu.cpp
    shared/entities/EntityEditorMenu.h
    shared/entities/EntityFactory.cpp
    shared/entities/EntityFactory.h
    shared/entities/EnvMapEntity.cpp
    shared/entities/EnvMapEntity.h
    shared/entities/MovableEntity.cpp
    shared/entities/MovableEntity.h
    shared/entities/animinfo.h
    shared/entities/attributes.cpp
    shared/entities/attributes.h
)

list(APPEND extRenderdocFiles
    ext/renderdoc/renderdoc_api.h
    ext/renderdoc/renderdoc_api.cpp
)

list(APPEND extNuklearFiles
    ext/nuklear/nuklear.h
    ext/nuklear/nuklear_polyhedron.h
    ext/nuklear/nuklear_polyhedron.cpp
)

list(APPEND extGladFiles
    ext/glad/src/glad.c
    ext/glad/include/glad/glad.h
    ext/glad/include/KHR/khrplatform.h
)

list(APPEND extImguiFiles
    ext/imgui/imconfig.h
    ext/imgui/imgui.cpp
    ext/imgui/imgui.h
    ext/imgui/imgui_demo.cpp
    ext/imgui/imgui_draw.cpp
    ext/imgui/imgui_internal.h
    ext/imgui/imgui_widgets.cpp
    ext/imgui/imstb_rectpack.h
    ext/imgui/imstb_textedit.h
    ext/imgui/imstb_truetype.h
    ext/imgui/examples/imgui_impl_sdl.h
    ext/imgui/examples/imgui_impl_sdl.cpp
    ext/imgui/examples/imgui_impl_opengl3.h
    ext/imgui/examples/imgui_impl_opengl3.cpp
)

list(APPEND engineEditorFiles
    engine/editor/ui.cpp
    engine/editor/ui.h
)

list(APPEND fileList
    ${fileListEngine}
    ${fileListShared}
    ${fileListSharedGeom}
    ${fileListGame}
    ${fileListSharedEntities}
    ${fileListGameEntities}
    ${engineEditorFiles}
    ${extRenderdocFiles}
    ${extNuklearFiles}
    ${extGladFiles}
    ${extImguiFiles}
)

function(GenerateBindingFor input_file ide_folder)
    if (${LIBCLANG_PATH})
        set(ENV{LIBCLANG_PATH} "${LIBCLANG_PATH}")
    else()
        if (WIN32)
            set(LIBCLANG_PATH "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libclang/bin/libclang.dll")
            set(ENV{LIBCLANG_PATH} "${CMAKE_CURRENT_BINARY_DIR}/thirdparty_sources/libclang/bin/libclang.dll")
        endif()
    endif()
    if(${input_file} MATCHES "\\.cpp$")
        execute_process(
            COMMAND ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py cmake_output ${input_file}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            OUTPUT_VARIABLE input_file_target
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        get_filename_component(output_file_target ${CMAKE_CURRENT_BINARY_DIR}/${input_file_target} ABSOLUTE)
        get_filename_component(input_file_full ${CMAKE_CURRENT_LIST_DIR}/${input_file} ABSOLUTE)

        message(">> ${input_file} => ${output_file_target} (${input_file_target})")

        add_custom_command(
            OUTPUT ${input_file_target}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            COMMAND ${CMAKE_COMMAND} -E env "LIBCLANG_PATH=${LIBCLANG_PATH}" ${PYTHON} ${CMAKE_CURRENT_LIST_DIR}/binding/index.py gen ${CMAKE_CURRENT_BINARY_DIR} ${input_file} ${output_file_target}
            COMMENT "Preprocessing ${input_file}"
#            DEPENDS ${input_file_full}
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
    if(${input_file} MATCHES "\\.cpp$")
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
source_group("Sources\\Shared\\Geom" FILES ${fileListSharedGeom})
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

foreach(input_file ${engineEditorFiles})
    GenerateBindingFor(${input_file} "Generated\\\\Engine\\\\Editor")
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
elseif(ANDROID)
    add_library(Polyhedron SHARED
        ${generatedFileList}
        ${fileList}
    )
else()
    add_executable(Polyhedron
        ${generatedFileList}
        ${fileList}
    )
endif()

#Options
target_compile_definitions(Polyhedron
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:RelWithDebInfo>:_DEBUG>
)

#Link the libraries
target_compile_features(Polyhedron
    PUBLIC
        cxx_std_20)
target_compile_options(Polyhedron
        PUBLIC
            -Wno-attributes)

target_include_directories(Polyhedron
    PRIVATE
        .
        shared
        game
        engine
        ${THIRDPARTY_INCLUDE_DIRS}
)

    target_link_libraries(Polyhedron
    PUBLIC
        ${THIRDPARTY_LIBRARIES}
        enet
)

if (NOT APPLE AND NOT ANDROID)
    target_link_libraries(Polyhedron
        PUBLIC
        rt
    )
endif()

target_link_directories(Polyhedron
    PUBLIC
    ${THIRDPARTY_LINK_DIRS}
)

#Apple specific stuff
if (APPLE)
    set_property(TARGET Polyhedron PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/xcode/tesseract.plist.in)

    find_program(IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin")
    if (${IBTOOL} STREQUAL "IBTOOL-NOTFOUND")
        message(SEND_ERROR "ibtool can not be found and is needed to compile the .xib files. It should have been installed with
                    the Apple developer tools. The default system paths were searched in addition to ${OSX_DEVELOPER_ROOT}/usr/bin")
    endif()

    add_custom_command (TARGET Polyhedron PRE_BUILD
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/Polyhedron.app/Contents/Resources)

    add_custom_command (TARGET Polyhedron POST_BUILD
        COMMAND ${IBTOOL} --errors --warnings --notices --output-format human-readable-text
        --compile ${CMAKE_CURRENT_BINARY_DIR}/Polyhedron.app/Contents/Resources/MainMenu.nib
        ${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.xib
        OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/Polyhedron.app/Contents/Resources/MainMenu.nib
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.xib
        COMMENT "Compiling ${CMAKE_CURRENT_LIST_DIR}/xcode/English.lproj/MainMenu.xib")

    #for Valgrind
    target_link_options(Polyhedron
        PRIVATE
            -Wl,-stack_size -Wl,4000000)
endif()
