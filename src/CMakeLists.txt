cmake_minimum_required(VERSION 3.5)

set(PYTHON_EXECUTABLE "C:/ProgramData/VMTK/python.exe")
set(PYTHON_LIBRARY "C:/ProgramData/VMTK/libs/python36.lib")
set(PYTHON_INCLUDE_DIR "C:/ProgramData/VMTK/include")

project(pyCmpr)

# define prepcompiler flag
# add_compile_definitions(DYNAMIC_VMTK)
set(USE_PYTHON_INCLUDE_DIR ON)
set(CMAKE_CONFIGURATION_TYPES Release)
set(VMTK_DIR "C:/ProgramData/VMTK")
set(VTK_DIR "C:/ProgramData/VMTK/Library/lib/cmake/vtk-8.1")
set(ITK_DIR "C:/ProgramData/VMTK/Library/lib/cmake/ITK-4.13")
set(PYBIND_DIRECTORY "D:/GitHub/pybind11-2.6.2")

# include vtk headers
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# include itk headers
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# include pybind11
find_path(PYBIND_DIRECTORY ${PYBIND_DIRECTORY} pybind11.egg-info)
# add pybind11
add_subdirectory(${PYBIND_DIRECTORY} ${PYBIND_DIRECTORY})

# include vmtk headers
find_path(VMTK_DIRECTORY ${VMTK_DIRECTORY} vmtk_env.sh)
include_directories(${VMTK_DIRECTORY}/include/vmtk)

include_directories("C:/ProgramData/VMTK/Library/include/vmtk")

# adds a library pybindings that can be called by python
pybind11_add_module(pyCmpr CurvedReformation.cpp)

# link vmtk libraries
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(${VMTK_DIRECTORY}/lib)
    FILE(GLOB vmtk ${VMTK_DIRECTORY}/lib/*.a)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    link_directories(${VMTK_DIRECTORY}/lib)
    FILE(GLOB vmtk ${VMTK_DIRECTORY}/lib/*.a)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  link_directories(${VMTK_DIRECTORY}/lib)
  FILE(GLOB vmtk ${VMTK_DIRECTORY}/lib/*.lib)

  include_directories("C:/ProgramData/VMTK/include")
  link_directories("C:/ProgramData/VMTK/Library/lib" "C:/ProgramData/VMTK/Library/bin")
  FILE(GLOB additional_libs "C:/ProgramData/VMTK/Library/lib/*.lib")

  FILE(GLOB additional_dlls "C:/ProgramData/VMTK/Library/bin/*.dll")
  foreach(dll ${additional_dlls})
    add_custom_command(TARGET pyCmpr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll} $<TARGET_FILE_DIR:pyCmpr>)
  endforeach()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# link external libraries
set_property(TARGET pyCmpr PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(pyCmpr PRIVATE ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${vmtk} )
