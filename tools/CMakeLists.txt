
set(tools
  smartscodegenerator
  smartsprint
  smartsmatch
  smartsscores
  smartsasm
  benchmark
  convert
  )


foreach(tool ${tools})
  add_executable(${tool} ${tool}.cpp)
  target_link_libraries(${tool} smartscompiler pthread)
  install(TARGETS ${tool}
                  RUNTIME DESTINATION bin
                  LIBRARY DESTINATION lib
                  ARCHIVE DESTINATION lib)
endforeach(tool ${tools})


