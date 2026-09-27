# Build jh-simple-cucumber once so TDD and BDD consumers share its target.
add_subdirectory(
        ${PROJECT_SOURCE_DIR}/simple-cucumber
        ${CMAKE_CURRENT_BINARY_DIR}/simple-cucumber
)

# Build jh-tiny-test beside jh-simple-cucumber so both testing packages expose
# their shared jh::test:: target namespace and self-checks to repository builds.
add_subdirectory(
        ${PROJECT_SOURCE_DIR}/tiny-test
        ${CMAKE_CURRENT_BINARY_DIR}/tiny-test
)

add_subdirectory(tdd)
add_subdirectory(static)
add_subdirectory(bdd)
