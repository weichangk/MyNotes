# 复制文件或目录
function(MY_COPY_FILE_FUNC)
    set(dstPath ${ARGV0})
    set(srcPaths ${ARGN})
    list(REMOVE_ITEM srcPaths ${dstPath})
    foreach(arg ${srcPaths})
        # 如果是目录，使用cp -fR命令递归复制整个目录到目标路径的子目录中，如果不是目录，直接复制文件到目标路径
        if(IS_DIRECTORY ${arg})
            get_filename_component(base_name ${arg} NAME)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND cp -fR ${arg} ${dstPath}/${base_name}
            )
        else()
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND cp -fR ${arg} ${dstPath}
            )
        endif()
    endforeach()
endfunction(MY_COPY_FILE_FUNC)

# 创建目录
function(MY_MKDIR_FUNC)
    foreach(arg ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${arg}
        )
    endforeach()
endfunction(MY_MKDIR_FUNC)

# 删除文件或目录
function(MY_DELFILE_FUNC)
    foreach(arg ${ARGN})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND  rm -fr ${arg}
        )
    endforeach()
endfunction(MY_DELFILE_FUNC)