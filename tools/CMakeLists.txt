# the smartscompiler executable
add_executable(smartscompiler_exe smartscompiler.cpp)
set_target_properties(smartscompiler_exe PROPERTIES OUTPUT_NAME smartscompiler)
target_link_libraries(smartscompiler_exe smartscompiler pthread)
install(TARGETS smartscompiler_exe
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


