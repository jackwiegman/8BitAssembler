cmake_minimum_required(VERSION 3.10.0)
project(Assembler VERSION 0.1.0 LANGUAGES C CXX)

SET(EXECTUABLE_NAME "Assembler")

SET (SOURCES SourceFiles/JackWiegman_AssemblerPart9.c)


add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
# target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ASMFiles/)
# add_subdirectory(ASMFiles)
# target_link_libraries(${CMAKE_PROJECT_NAME} mylib)

# cmrc_add_resource_library(ASM_Files "ASMFiles/")

# Enable generation of compilation database
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create symlink to project root during configure stage
# For LSP functionality on linux
if (UNIX)
   execute_process(
       COMMAND ${CMAKE_COMMAND} -E create_symlink
       ${CMAKE_BINARY_DIR}/compile_commands.json
       ${CMAKE_SOURCE_DIR}/compile_commands.json
   )
endif()

# FILE ()

add_custom_command(
       TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_directory
       "${CMAKE_CURRENT_SOURCE_DIR}/ASMFiles" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
       COMMENT "Copying .asm file to build directory from source files."
   )
# FILE (GLOB ASM_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/SourceFiles/*.asm")
# FILE (GLOB ASM_ROOT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

# if (ASM_SOURCE_FILES)
#    add_custom_command(
#        TARGET Assembler POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "${ASM_SOURCE_FILES}" $<TARGET_FILE_DIR:Assembler>
#        COMMENT "Copying .asm file to build directory from source files."
#    )
# elseif(ASM_ROOT_FILES)
#    add_custom_command(
#        TARGET Assembler POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        "${ASM_ROOT_FILES}" $<TARGET_FILE_DIR:Assembler>
#        COMMENT "Copying .asm file to build directory from source files."
#    )
# else()
#    message(WARNING "ASM file not found in SourceFiles")
# endif()
