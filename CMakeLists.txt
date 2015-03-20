CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)
PROJECT(CheckImgLinearity)

## Default install path; user doesn't have to specify
## will cause the gui will be not updated, and unavailable for changing by user
SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
MESSAGE(STATUS "Install path set to ${CMAKE_INSTALL_PREFIX}.")

## Include cmake stuff (functions/macros) : Modules files
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

## Allow C++11
include(CheckCXXCompilerFlag)
get_filename_component(currentBuildTool ${CMAKE_BUILD_TOOL} NAME_WE)
if(${currentBuildTool} MATCHES "(msdev|devenv|nmake|MSBuild)")
    add_definitions(/W3 /DNOMINMAX)
elseif(${currentBuildTool} MATCHES "(make|gmake)")
    add_definitions(-Wall -g)
	CHECK_CXX_COMPILER_FLAG("-std=gnu++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=gnu++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		add_definitions(-std=gnu++11)
	elseif(COMPILER_SUPPORTS_CXX0X)
		add_definitions(-std=gnu++0x)
	else()
		message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
elseif(APPLE) ## TODO: do a better test and send error on unsupported c++11 compiler
    add_definitions(-std=c++11 -stdlib=libc++)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

## FIND PACKAGES

## Qt5
if(WIN32)
    file(GLOB qt5versionPathList "C:/Qt/Qt5.*")
    if(NOT Qt5_DIR AND qt5versionPathList)
        list(LENGTH qt5versionPathList qt5versionPathListCount)
        if(${qt5versionPathListCount} GREATER "1")
            message("Many Qt5 version auto detected (check manually the right one with Qt5_DIR cmake variable).")
        endif()
        foreach(qt5versionPath ${qt5versionPathList})           
            ## go deep to look for any qt5 install dir (sdk include/lib dirs)
            file(GLOB qt5versionSubPathList "${qt5versionPath}/5.*")
            file(GLOB qt5versionSubPathList "${qt5versionSubPathList}/*")
            if(qt5versionSubPathList)
                foreach(qt5versionSubPath ${qt5versionSubPathList})
                    get_filename_component(redistArch ${qt5versionSubPath} NAME)
                    string(REGEX MATCH 	"[A-Za-z_0-9-]+64[A-Za-z_0-9-]+" 64archMatched ${redistArch})
                    if(64archMatched)
                        set(qtArch x64)
                    else()
                        set(qtArch x86)
                    endif()
                    message("Plausible Qt5 instllation dir [${qtArch}] : ${qt5versionSubPath}")
                    if(CMAKE_SIZEOF_VOID_P MATCHES "8")
                        if("${qtArch}" MATCHES "x64")
                            set(Qt5_DIR "${qt5versionSubPath}/lib/cmake/Qt5") ## choose last one
                        endif()
                    else()
                        if("${qtArch}" MATCHES "x86")
                            set(Qt5_DIR "${qt5versionSubPath}/lib/cmake/Qt5") ## choose last one
                        endif()
                    endif()
                endforeach()
            endif()
        endforeach()
    endif()
    ## WORK AROUND: QT5 5.2.1 win32 OpenGL problem to find glu32... we need WINSDK to let qt find it
    find_package(WindowsSDK 7 QUIET)
    if(WindowsSDK_FOUND)
        list(APPEND CMAKE_LIBRARY_PATH ${WindowsSDK_LIBRARY_DIRS})
    else()
        message(WARNING "You need Windows SDK to let Qt5 find OpenGL glu32.")
    endif()
endif()
set(Qt5_DIR ${Qt5_DIR} CACHE PATH "Path to <Qt5 installation>/lib/cmake/Qt5")
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)
if(NOT Qt5_FOUND)
    message(SEND_ERROR "Qt5 not found, please set Qt5_DIR to <Qt5 installation>/lib/cmake/Qt5")
elseif(WIN32 AND "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" MATCHES "5.2.1")
    message(WARNING "Be careful, there were bugs on 5.2.1 win32 version : http://stackoverflow.com/questions/14008737/qt5-qconfig-h-weird")
endif()
if(POLICY CMP0020)
	cmake_policy(SET CMP0020 NEW) ## auto link to qtmain under windows
endif()

## OpenImageIO
find_package(OpenImageIO REQUIRED)
if(NOT OpenImageIO_FOUND)
	message(SEND_ERROR "OpenImageIO NOT FOUND!!")
else()
	include_directories(${OPENIMAGEIO_INCLUDE_DIR})
endif()


## CONFIG our project files

set(SOURCES 
#    .h
#    .cpp
	src/mainwindow.h
	src/mainwindow.cpp
	
	src/qcustomplot.h
	src/qcustomplot.cpp
	
	src/main.cpp
		
	src/SwatchMainWindow.h
	src/SwatchMainWindow.cpp
    
    src/ImagePlugin.h
    src/ImagePlugin.cpp
    
    src/ColorSwatch.h
    src/ColorSwatch.cpp
    
    src/ColorSwatchPatch.h
    src/ColorSwatchPatch.cpp
    
    src/MunsellColor.h
    src/MunsellColor.cpp
    
    src/PreBuildUtil.h
    
    src/ColorSwatchMask.h
    src/ColorSwatchMask.cpp
)

QT5_WRAP_UI(UIS_HDRS
#    .ui
	src/mainwindow.ui
)

add_executable(${PROJECT_NAME} 
    #WIN32 
    ${SOURCES}
    ${UIS_HDRS}
)
    
target_link_libraries(${PROJECT_NAME} 
	Qt5::Widgets
	Qt5::PrintSupport
	${OPENIMAGEIO_LIBRARIES}
)