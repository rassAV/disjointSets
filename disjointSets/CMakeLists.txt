add_executable (disjointSets "disjointSets.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.14)
  set_property(TARGET disjointSets PROPERTY CXX_STANDARD 17)
endif()