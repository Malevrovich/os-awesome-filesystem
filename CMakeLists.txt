cmake_minimum_required(VERSION 3.0)
set(CMAKE_CURRENT_BINARY_DIR, ./buildx)
project(os-afs-server)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(SOURCES
  ${llp-lab1_SOURCE_DIR}/src/main.c
  ${llp-lab1_SOURCE_DIR}/src/api/ast_api.c
  ${llp-lab1_SOURCE_DIR}/src/ast/ast.c
  ${llp-lab1_SOURCE_DIR}/src/parser/parser_res.c
  ${BISON_parser_OUTPUT_SOURCE}
  ${FLEX_lexer_OUTPUTS}
)

set(INCLUDES 
  ${llp-lab1_SOURCE_DIR}/includes/
  ${llp-lab1_SOURCE_DIR}/includes/api/
  ${llp-lab1_SOURCE_DIR}/includes/ast/
  ${llp-lab1_SOURCE_DIR}/includes/parser/
  ${llp-lab1_SOURCE_DIR}/includes/lexer/
)
