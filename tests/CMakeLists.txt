# Build jh-simple-cucumber once so TDD and BDD consumers share its target.
add_subdirectory(
        ${PROJECT_SOURCE_DIR}/simple-cucumber
        ${CMAKE_CURRENT_BINARY_DIR}/simple-cucumber
)

add_subdirectory(tdd)
add_subdirectory(static)
add_subdirectory(bdd)
