cmake_minimum_required(VERSION 2.6)
project(SmartsCompiler)

# set the module path
set(CMAKE_MODULE_PATH ${SmartsCompiler_SOURCE_DIR}/cmake/modules)

# find and setup OpenBabel
find_package(OpenBabel2 REQUIRED)
include_directories(${OPENBABEL2_INCLUDE_DIR})

# unit tests
enable_testing()
add_subdirectory(test)

# Should Python bindings be built?
option(PYTHON_BINDINGS "Build Python bindings" OFF)
if(PYTHON_BINDINGS)
  #include(FindSWIG)
  find_package(SWIG 2.0)
  if(NOT SWIG_FOUND)
    message(STATUS "SWIG 2.0 NOT found")
  endif()

  find_package(PythonLibs)
  if(NOT PYTHONLIBS_FOUND)
    message(STATUS "Python libraries NOT found")
  endif()

  find_package(PythonInterp)
  if(NOT PYTHONINTERP_FOUND)
    message(STATUS "Python interpreter NOT found")
  endif()

  if(PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND)
    add_definitions(-DHAVE_PYTHON)
    set(HAVE_PYTHON ON BOOL)
    message(STATUS "Python bindings will be compiled")
  endif()

  include_directories(${PYTHON_INCLUDE_PATH})
endif()


set(libsmartscompiler_hdrs
    src/openbabel.h
    src/smartscodegenerator.h
    src/smartsmatcher.h
    src/smartsoptimizer.h
    src/smartspattern.h
    src/smartsprint.h
    src/smartsscores.h
  )

set(libsmartscompiler_srcs
    #    src/openbabel.cpp
    #    src/smartsmatcher.cpp
    src/smartsscores.cpp
    src/smartsoptimizer.cpp
    #    src/smartscodegenerator.cpp
    src/smartsprint.cpp
    src/smarts.cpp
)

add_library(smartscompiler SHARED ${libsmartscompiler_srcs})
target_link_libraries(smartscompiler ${OPENBABEL2_LIBRARIES} ${PYTHON_LIBRARIES})
install(TARGETS smartscompiler
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)

              #add_subdirectory(tools)





if(SWIG_FOUND)
  add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/smartscompiler_wrap.cpp ${CMAKE_SOURCE_DIR}/src/smartscompiler.py
        COMMAND ${SWIG_EXECUTABLE} -python -c++ -small -O -templatereduce -naturalvar
        -I${OPENBABEL2_INCLUDE_DIR} -I${CMAKE_SOURCE_DIR} -o ${CMAKE_SOURCE_DIR}/src/smartscompiler_wrap.cpp
        -outdir ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/smartscompiler.i
        MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/src/smartscompiler.i ${libsmartscompiler_hdrs} VERBATIM)
  
  add_library(bindings_python MODULE ${CMAKE_SOURCE_DIR}/src/smartscompiler_wrap.cpp)
  target_link_libraries(bindings_python ${PYTHON_LIBRARIES} smartscompiler)
  if(NOT WIN32)
    set_target_properties(bindings_python PROPERTIES OUTPUT_NAME _smartscompiler PREFIX "" SUFFIX .so)
    add_dependencies(bindings_python smartscompiler)
    install(TARGETS bindings_python LIBRARY DESTINATION lib)
    install(FILES ${CMAKE_SOURCE_DIR}/src/smartscompiler.py DESTINATION lib)
    install(FILES ${CMAKE_SOURCE_DIR}/src/smartscompiler.py DESTINATION lib)
  else()
    set_target_properties(bindings_python PROPERTIES OUTPUT_NAME _smartscompiler PREFIX "" SUFFIX .pyd )
    add_custom_command(TARGET bindings_python POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/smartscompiler.py ${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR})
  endif()
endif()


