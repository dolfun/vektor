add_subdirectory(vektor)

if (EMSCRIPTEN)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public/modules")

  add_executable(vektor vektor_bindings.cc)
  target_link_libraries(vektor PRIVATE vektor_lib)
  target_include_directories(vektor PRIVATE ${EMSCRIPTEN_ROOT_PATH}/system/include)
  set_target_properties(vektor PROPERTIES SUFFIX ".mjs")

  set(vektor_tsd_file "${CMAKE_SOURCE_DIR}/src/vektor/types.d.ts")
  target_link_options(vektor PRIVATE
    "SHELL:-lembind"
    "SHELL:-sMODULARIZE=1"
    "SHELL:-sEXPORT_ES6=1"
    "SHELL:-sENVIRONMENT=web"
    "SHELL:--emit-tsd=${vektor_tsd_file}"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"

    "$<$<CONFIG:Debug>:SHELL:-O0>"
    "$<$<CONFIG:Release>:SHELL:-O3 -flto>"
  )
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

  add_executable(vektor vektor_bin.cc)
  target_link_libraries(vektor PRIVATE vektor_lib)
endif()