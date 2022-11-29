function(add_shader TARGET SHADER)
	# Find glslc shader compiler.
	find_program(GLSLC glslc)

	# All shaders for a sample are found here.
	set(current-shader-path ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/shaders/${SHADER})

	# Output path is inside the assets folder.
	set(current-output-path ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/assets/shaders/${SHADER}.spv)

	# Add a custom command to compile GLSL to SPIR-V.
	get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
	file(MAKE_DIRECTORY ${current-output-dir})
	add_custom_command(
			OUTPUT ${current-output-path}
			COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
			DEPENDS ${current-shader-path}
			IMPLICIT_DEPENDS CXX ${current-shader-path}
			VERBATIM)

	#	add_custom_target(${SHADER} DEPENDS ${current-output-path})
	#	add_dependencies(${TARGET} ${SHADER})

	add_custom_command(
			TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${current-output-path} $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/shaders/${SHADER}.spv
			DEPENDS ${current-output-path}
			IMPLICIT_DEPENDS CXX ${current-shader-path}
			VERBATIM)

	set(shaders ${shaders} ${SHADER} PARENT_SCOPE)

	# Make sure our native build depends on this output.
	set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
	target_sources(${TARGET} PRIVATE ${current-output-path})
	#	configure_file(${current-output-path} ${current-output-path} COPYONLY)
endfunction(add_shader)
