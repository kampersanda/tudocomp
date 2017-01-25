file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include/tudocomp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/tudocomp/config.h.in ${CMAKE_BINARY_DIR}/include/tudocomp/config.h)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/tudocomp_algorithms.cpp
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/etc/genregistry.py ${CMAKE_BINARY_DIR}/include/tudocomp/config.h "tudocomp_algorithms.cpp" ${CMAKE_BINARY_DIR}/tudocomp_algorithms.cpp
    COMMENT "Generating tudocomp_algorithms.cpp..."
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/etc/genregistry.py ${CMAKE_BINARY_DIR}/include/tudocomp/config.h
)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/include/tudocomp/tudocomp.hpp
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/etc/genregistry.py ${CMAKE_BINARY_DIR}/include/tudocomp/config.h "tudocomp.hpp" ${CMAKE_BINARY_DIR}/include/tudocomp/tudocomp.hpp
    COMMENT "Generating tudocomp.hpp..."
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/etc/genregistry.py ${CMAKE_BINARY_DIR}/include/tudocomp/config.h
)

add_custom_target(
    generate_version
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/etc/genversion.sh ${CMAKE_BINARY_DIR}/include/tudocomp/version.hpp
)

add_custom_target(
    generate_tudocomp_hpp
    DEPENDS
        ${CMAKE_BINARY_DIR}/include/tudocomp/tudocomp.hpp
        generate_version
)

include_directories(${CMAKE_BINARY_DIR}/include)
