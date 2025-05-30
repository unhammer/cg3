find_package(SWIG 3.0 REQUIRED)
find_package(Python 3.8 REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

set(PYTHON_FILE "constraint_grammar.py")
set(CPP_WRAP_FILE "constraint_grammar_wrap.cpp")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cg3.py" "from constraint_grammar import *\n")

set(BUILD_DEFS "")
get_directory_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
foreach(d ${_defs})
	set(BUILD_DEFS "${BUILD_DEFS} -D${d}")
endforeach()
configure_file(constraint_grammar.i.in constraint_grammar.i @ONLY)
configure_file(setup.py.in setup.py)

add_custom_command(OUTPUT ${CPP_WRAP_FILE} ${PYTHON_FILE}
	COMMAND ${PYTHON_EXECUTABLE} setup.py build
	DEPENDS cg3
	COMMENT "Building ${PYTHON_FILE}"
)

add_custom_target(wrapper ALL
	DEPENDS ${CPP_WRAP_FILE} ${PYTHON_FILE}
	VERBATIM
)

if(NOT PYTHON_INSTALL_PARAMS)
	set(PYTHON_INSTALL_PARAMS "--prefix=${CMAKE_INSTALL_PREFIX} --root=\$ENV{DESTDIR}/")
endif()

set(INSTALL_WRAPPER "${PYTHON_EXECUTABLE} setup.py install ${PYTHON_INSTALL_PARAMS}")
install(CODE "execute_process(COMMAND ${INSTALL_WRAPPER} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})")
