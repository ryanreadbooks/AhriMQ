# cmake helper function to add test binary
function(ahrimq_add_cc_test)
  if (NOT BUILD_TESTING)
    return()
  endif()

  cmake_parse_arguments(AHRIMQ_CC_TEST
    ""
    "NAME"
    "SRCS;LINKS"
    ${ARGN}
  )

  set(cc_test_target_name "${AHRIMQ_CC_TEST_NAME}")
  set(cc_test_srcs "${AHRIMQ_CC_TEST_SRCS}")
  set(cc_test_links "${AHRIMQ_CC_TEST_LINKS}")

  add_executable(${cc_test_target_name} ${cc_test_srcs})

  target_link_libraries(
    ${cc_test_target_name} 
  PRIVATE 
    GTest::GTest
    GTest::Main
    ${cc_test_links}
  )

  set_property(TARGET ${cc_test_target_name} PROPERTY LINKER_LANGUAGE "CXX")

  add_test(NAME ${cc_test_target_name} COMMAND ${cc_test_target_name})

endfunction(ahrimq_add_cc_test)

# cmake helper function to create dependency
function(ahrimq_create_dependency)
  cmake_parse_arguments(AHRIMQ_OBJ_LIB
    ""
    "NAME"
    "SRCS;INCS;LINKS"
    ${ARGN}
  )

  set(objname ${AHRIMQ_OBJ_LIB_NAME})
  set(objincs ${AHRIMQ_OBJ_LIB_INCS})
  set(objsrcs ${AHRIMQ_OBJ_LIB_SRCS} ${objincs})
  set(objlinks ${AHRIMQ_OBJ_LIB_LINKS})

  add_library(${objname} SHARED ${objsrcs})
  add_library(ahrimq::${objname} ALIAS ${objname})
  target_link_libraries(${objname} PRIVATE ${objlinks})

endfunction(ahrimq_create_dependency)