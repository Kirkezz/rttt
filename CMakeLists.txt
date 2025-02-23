cmake_minimum_required(VERSION 3.16)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(rttt)

include(FetchContent)

if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
    endif()
endif()

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

# Create an executable or a shared library based on the platform and add our sources to it
if (ANDROID)
	# The SDL java code is hardcoded to load libmain.so on android, so we need to change EXECUTABLE_NAME
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
	add_executable(${EXECUTABLE_NAME})
endif()

# Add your sources to the target
target_sources(${EXECUTABLE_NAME}
PRIVATE
    src/main.cpp
    src/RTTT.cpp
    src/RTTT_Engine.cpp
    src/RTTT_Engine.hpp
    src/RTTT.hpp
    src/iosLaunchScreen.storyboard
    src/RmlUi/RmlUi_Backend.h
    src/RmlUi/RmlUi_Platform_SDL.cpp
    src/RmlUi/RmlUi_Renderer_SDL.cpp
    src/RmlUi/RmlUi_Backend_SDL_SDLrenderer.cpp
    src/RmlUi/RmlUi_Platform_SDL.h
    src/RmlUi/RmlUi_Renderer_SDL.h
    src/RmlUi/BackendData.cpp
    src/RmlUi/BackendData.hpp
    src/RmlUi/EventListener.cpp
    src/RmlUi/EventListener.hpp
)
# What is iosLaunchScreen.storyboard? This file describes what Apple's mobile platforms
# should show the user while the application is starting up. If you don't include one,
# then you get placed in a compatibility mode that does not allow HighDPI.
# This file is referenced inside Info.plist.in, where it is marked as the launch screen file.
# It is also ignored on non-Apple platforms.

# To get an app icon on Apple platforms, add it to your executable.
# Afterward, the image file in Info.plist.in.
if(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "src/logo.png")
endif()

# Set C++ version
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_20)

# on Web targets, we need CMake to generate a HTML webpage.
if(EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# If you don't want SDL_ttf, then remove this section.
set(SDLTTF_VENDORED ON) # tell SDL_ttf to build its own dependencies
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)     # disable formats we don't use to make the build faster and smaller. Also some of these don't work on all platforms so you'll need to do some experimentation.
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_VENDORED ON)   # tell SDL_mixer to build its own dependencies
add_subdirectory(SDL_mixer EXCLUDE_FROM_ALL)

# SDL_image (used for loading various image formats)
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)

FetchContent_Declare(RmlUi
        GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
        GIT_TAG master
)
FetchContent_MakeAvailable(RmlUi)
add_library(Freetype::Freetype ALIAS freetype)

# Link SDL to our executable. This also makes its include directory available to us.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC
    RmlUi::RmlUi
        SDL3_ttf::SDL3_ttf      # remove if you are not using SDL_ttf
        SDL3_mixer::SDL3_mixer  # remove if you are not using SDL_mixer
        SDL3_image::SDL3_image  # remove if you are not using SDL_image
    SDL3::SDL3              # If using satelite libraries, SDL must be the last item in the list.
)

# SDL_Image bug: https://github.com/libsdl-org/SDL_image/issues/506
if (APPLE AND NOT BUILD_SHARED_LIBS)
    find_library(IO_LIB ImageIO REQUIRED)
    find_library(CS_LIB CoreServices REQUIRED)
    find_library(CT_LIB CoreText REQUIRED)
    find_library(CG_LIB CoreGraphics REQUIRED)
    find_library(CF_LIB CoreFoundation REQUIRED)
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${CF_LIB} ${CT_LIB} ${IO_LIB} ${CS_LIB} ${CG_LIB})
endif()
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# Dealing with assets
# We have some non-code resources that our application needs in order to work. How we deal with those differs per platform.
if (APPLE)
    set(input_root "${CMAKE_CURRENT_LIST_DIR}/src/resources")
    macro(add_resource FILE)
        file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
        get_filename_component(relpath "${relpath}" DIRECTORY)
        target_sources(${EXECUTABLE_NAME} PRIVATE ${FILE})
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${relpath}")
    endmacro()
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/player_1.bmp")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/player_2.bmp")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/player_3.bmp")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/player_4.bmp")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/active_game.rml")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/default.rcss")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-BoldItalic.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Bold.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Italic.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Regular.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LICENSE.txt")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/menu.rml")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/new_game.rml")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/NotoEmoji-Regular.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/settings.rml")
elseif(EMSCRIPTEN)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/player_1.bmp@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/player_2.bmp@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/player_3.bmp@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/player_4.bmp@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/active_game.rml@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/default.rcss@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-BoldItalic.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Bold.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Italic.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LatoLatin-Regular.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/LICENSE.txt@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/menu.rml@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/new_game.rml@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/NotoEmoji-Regular.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/src/resources/RmlAssets/settings.rml@/\""
    )
else()
    if (ANDROID)
        file(MAKE_DIRECTORY "${MOBILE_ASSETS_DIR}")
        file(MAKE_DIRECTORY "${MOBILE_ASSETS_DIR}/RmlAssets")
    endif()
    macro(copy_helper filename)
        if (ANDROID)
            set(outname "${MOBILE_ASSETS_DIR}/${filename}")
        else()
            set(outname "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/${filename}")
        endif()
        add_custom_command(POST_BUILD
            TARGET "${EXECUTABLE_NAME}"
		   COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/src/resources/${filename}" "${outname}"
		   DEPENDS "${filename}"
	   )
    endmacro()
    copy_helper("player_1.bmp")
    copy_helper("player_2.bmp")
    copy_helper("player_3.bmp")
    copy_helper("player_4.bmp")
    copy_helper("RmlAssets/active_game.rml")
    copy_helper("RmlAssets/default.rcss")
    copy_helper("RmlAssets/LatoLatin-BoldItalic.ttf")
    copy_helper("RmlAssets/LatoLatin-Bold.ttf")
    copy_helper("RmlAssets/LatoLatin-Italic.ttf")
    copy_helper("RmlAssets/LatoLatin-Regular.ttf")
    copy_helper("RmlAssets/LICENSE.txt")
    copy_helper("RmlAssets/menu.rml")
    copy_helper("RmlAssets/new_game.rml")
    copy_helper("RmlAssets/NotoEmoji-Regular.ttf")
    copy_helper("RmlAssets/settings.rml")
endif()

# set some extra configs for each platform
set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch. 
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.ravbug.sdl3-sample"
	XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
# note that fixup_bundle does not work on iOS, so you will want to use static libraries 
# or manually copy dylibs and set rpaths
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    # tell Install about the target, otherwise fixup won't know about the transitive dependencies
    install(TARGETS ${EXECUTABLE_NAME}
    	BUNDLE DESTINATION ./install COMPONENT Runtime
   	    RUNTIME DESTINATION ./install/bin COMPONENT Runtime
    )
	
    set(DEP_DIR "${CMAKE_BINARY_DIR}")  # where to look for dependencies when fixing up
    INSTALL(CODE 
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        " 
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()
