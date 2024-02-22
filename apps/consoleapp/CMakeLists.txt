add_executable(consoleapp main.cpp)
target_link_libraries(consoleapp 
    PRIVATE appcommon
)

# automatically copy all dll's deps.
add_custom_command(TARGET consoleapp POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:consoleapp> $<TARGET_RUNTIME_DLLS:consoleapp>
  COMMAND_EXPAND_LISTS
)
# copy hip kernels
populate_rprsdk_kernels(consoleapp)

# copy assets
_copy_file_to_target(consoleapp ${CMAKE_SOURCE_DIR}/apps/assets/teapot.obj)
_copy_file_to_target(consoleapp ${CMAKE_SOURCE_DIR}/apps/assets/envLightImage.exr)



