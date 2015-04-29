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

if(POLICY CMP0020)
	cmake_policy(SET CMP0020 NEW) ## auto link to qtmain under windows
endif()

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
    find_package(WindowsSDK QUIET)
    if(WindowsSDK_FOUND)
        list(APPEND CMAKE_LIBRARY_PATH ${WindowsSDK_LIBRARY_DIRS})
    endif()
endif()
set(Qt5_DIR ${Qt5_DIR} CACHE PATH "Path to <Qt5 installation>/lib/cmake/Qt5")
set(CMAKE_AUTOMOC ON)
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED) ## for qcustomplot dependency
if(NOT Qt5_FOUND)
    message(SEND_ERROR "Qt5 not found, please set Qt5_DIR to <Qt5 installation>/lib/cmake/Qt5")
elseif(WIN32 AND "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" MATCHES "5.2.1")
    message(WARNING "Be careful, there were bugs on 5.2.1 win32 version : http://stackoverflow.com/questions/14008737/qt5-qconfig-h-weird")
endif()


## OpenImageIO
find_package(OpenImageIO REQUIRED)
if(NOT OpenImageIO_FOUND)
	message(SEND_ERROR "OpenImageIO NOT FOUND!!")
else()
	include_directories(${OPENIMAGEIO_INCLUDE_DIR})
endif()

## Boost (needed by oiio)
find_package(Boost REQUIRED COMPONENTS thread system chrono) ## to use imagebufalgo of OIIO
if(Boost_FOUND OR BOOST_FOUND)
    include_directories(${BOOST_INCLUDEDIR} ${Boost_INCLUDE_DIRS})
    link_directories(${BOOST_LIBRARYDIR} ${Boost_LIBRARY_DIRS})
    if(WIN32)
        add_definitions(-DBOOST_ALL_DYN_LINK -DBOOST_ALL_NO_LIB)
    endif()
endif()

## IlmBase (needed by oiio)
set(ILMBASE_HOME "" CACHE PATH "OPENEXR install dir")
find_package (IlmBase REQUIRED)
include_directories ("${ILMBASE_INCLUDE_DIR}")
include_directories ("${ILMBASE_INCLUDE_DIR}/OpenEXR")

## OpenEXR (needed by oiio)
set(OPENEXR_HOME "" CACHE PATH "OPENEXR install dir")
find_package (OpenEXR REQUIRED)
if (EXISTS "${OPENEXR_INCLUDE_DIR}/OpenEXR/ImfMultiPartInputFile.h")
    add_definitions (-DUSE_OPENEXR_VERSION2=1)
endif()
include_directories ("${OPENEXR_INCLUDE_DIR}")
include_directories ("${OPENEXR_INCLUDE_DIR}/OpenEXR")


## Prepare external qcustomplot 3rdParty (optional different way)
set(QCP_PREFIX "${CMAKE_BINARY_DIR}/external/qcustomplot")
macro(AS_EXTERNAL type)

    if(${type} MATCHES "GIT")
        set(DL_TYPE_ARG     GIT_REPOSITORY  https://gitorious.org/qcustomplot/qcustomplot.git)
        set(FROM_AMALGAMATE OFF)
    elseif(${type} MATCHES "ZIP")
        set(DL_TYPE_ARG     URL "${CMAKE_SOURCE_DIR}/external/QCustomPlot.tar.gz")
        set(FROM_AMALGAMATE ON)
    endif()    
    include(ExternalProject)
    ExternalProject_Add(qcustomplot
        PREFIX          ${QCP_PREFIX}
        ${DL_TYPE_ARG}
		PATCH_COMMAND   ${GIT_EXECUTABLE} apply "${CMAKE_SOURCE_DIR}/external/QCustomPlot.patch"
        INSTALL_DIR     ${QCP_PREFIX}/install
        CMAKE_ARGS      -DCMAKE_INSTALL_PREFIX=${QCP_PREFIX}/install
                        -DQt5_DIR=${Qt5_DIR}
                        -DBUID_FROM_AMALGAMATE=${FROM_AMALGAMATE}
                        -DBUILD_SHARED=OFF
    )
    ExternalProject_Get_Property(qcustomplot SOURCE_DIR)
    ExternalProject_Add_Step(qcustomplot rmCMakeLists 
        COMMAND     ${CMAKE_COMMAND} -E remove -f ${SOURCE_DIR}/CMakeLists.txt
        DEPENDERS   patch
    )
    set(QCustomPlot_DIR ${QCP_PREFIX}/install CACHE PATH "" FORCE)
    ## we already know in advance where are include and lib
    include_directories(${QCustomPlot_DIR}/include)
    set(QCUSTOMPLOT_LIBRARIES debug ${QCustomPlot_DIR}/lib/qcustomplotd.lib optimized ${QCustomPlot_DIR}/lib/qcustomplot.lib)
    ## or we can run-run cmake (1st for externalProject build only, second time with find_packages and normal build)
    
endmacro()
set(QCustomPlot_BUILD AS_EXTERNAL_GIT CACHE STRING "which way we build qcustomplot")
set_property(CACHE QCustomPlot_BUILD PROPERTY STRINGS AS_EXTERNAL_GIT AS_EXTERNAL_ZIP AS_FIND_PACKAGE AS_EMBEDED)
if(${QCustomPlot_BUILD} MATCHES "AS_EXTERNAL_GIT")      ## use External_Project from public git url (master branch)
    
    AS_EXTERNAL(GIT)
    
elseif(${QCustomPlot_BUILD} MATCHES "AS_EXTERNAL_ZIP")  ## use External_Project from a local zip install file
    
    AS_EXTERNAL(ZIP)
    
elseif(${QCustomPlot_BUILD} MATCHES "AS_FIND_PACKAGE")  ## "classic" 3rdParty cmake find (based on our CMakeLists.txt from patch)
    
    set(QCustomPlot_DIR "" CACHE PATH "")
    find_package(QCustomPlot REQUIRED)
    include_directories(${QCUSTOMPLOT_INCLUDE_DIRS})
    
elseif(${QCustomPlot_BUILD} MATCHES "AS_EMBEDED")       ## build our project with qcustomplot.h/.cpp directly embeded (part of our sources)
    
    set(PROGRAMFILESx86 "PROGRAMFILES(x86)")
    if(NOT SEVEN_ZIP_CMD)
		find_program(SEVEN_ZIP_CMD NAMES 7z 7za p7zip DOC "7-zip executable" PATHS "$ENV{PROGRAMFILES}/7-Zip" "$ENV{${PROGRAMFILESx86}}/7-Zip" "$ENV{ProgramW6432}/7-Zip")
	endif()
    if(SEVEN_ZIP_CMD)
        execute_process(COMMAND ${SEVEN_ZIP_CMD} x "${CMAKE_SOURCE_DIR}/external/QCustomPlot.tar.gz" -y
						WORKING_DIRECTORY ${QCP_PREFIX} TIMEOUT 200 RESULT_VARIABLE resVar OUTPUT_VARIABLE outVar ERROR_VARIABLE errVar )
        if(${resVar} MATCHES "0")
            execute_process(COMMAND ${SEVEN_ZIP_CMD} x "${QCP_PREFIX}/QCustomPlot.tar" -y
                WORKING_DIRECTORY ${QCP_PREFIX} TIMEOUT 200 RESULT_VARIABLE resVar OUTPUT_VARIABLE outVar ERROR_VARIABLE errVar )
        endif()
        if(${resVar} MATCHES "0")
            message(STATUS "unzip QCustomPlot.tar.gz")
        else()
            message("unzip: resVar=${resVar}\nunzip: outVar=${outVar}\nunzip: errVar=${errVar}\nunzip: failed or canceled or timeout")
        endif()
    else()
        message(WARNING "SEVEN_ZIP_CMD not found, set it or switch QCustomPlot_BUILD method")
    endif()
    if(EXISTS "${QCP_PREFIX}/qcustomplot/qcustomplot.h")
        set(qcustomSourcesFiles 
            "${QCP_PREFIX}/qcustomplot/qcustomplot.h"
            "${QCP_PREFIX}/qcustomplot/qcustomplot.cpp"
        )
        message(STATUS "add qcustomplot.h/.cpp to be part of our project sources.")
    else()
        message(WARNING "qcustomplot sources files missing, switch QCustomPlot_BUILD method")
    endif()
    
endif()



## CONFIG our project files

set(SOURCES 
	${qcustomSourcesFiles}
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

QT5_WRAP_UI(UIS_HDRS src/mainwindow.ui)

add_executable(${PROJECT_NAME} 
    #WIN32 ## uncomment to hide terminal
    ${SOURCES}
    ${UIS_HDRS}
)

target_link_libraries(${PROJECT_NAME} 
	Qt5::Widgets Qt5::PrintSupport
    ${QCUSTOMPLOT_LIBRARIES}
	${OPENIMAGEIO_LIBRARIES} 
    ${Boost_LIBRARIES}
    ${OPENEXR_LIBRARIES} ${ILMBASE_LIBRARIES}
)

if(TARGET qcustomplot)
    add_dependencies(${PROJECT_NAME} qcustomplot)
endif()

## handle documentation
ADD_SUBDIRECTORY(doc)