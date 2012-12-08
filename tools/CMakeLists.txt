# the smartscodegenerator executable
add_executable(smartscodegenerator_exe smartscodegenerator.cpp)
set_target_properties(smartscodegenerator_exe PROPERTIES OUTPUT_NAME smartscodegenerator)
target_link_libraries(smartscodegenerator_exe smartscompiler pthread)
install(TARGETS smartscodegenerator_exe
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)

# the smartsprint executable
add_executable(smartsprint smartsprint.cpp)
target_link_libraries(smartsprint smartscompiler pthread)
install(TARGETS smartsprint
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)

# the smartsmatch executable
add_executable(smartsmatch smartsmatch.cpp)
target_link_libraries(smartsmatch smartscompiler pthread)
install(TARGETS smartsmatch
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)

# the smartsscores executable
add_executable(smartsscores smartsscores.cpp)
target_link_libraries(smartsscores smartscompiler pthread)
install(TARGETS smartsscores
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)

# the smartsasm executable
add_executable(smartsasm smartsasm.cpp)
target_link_libraries(smartsasm smartscompiler pthread)
install(TARGETS smartsasm
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
