macro(add_target_properties _target _name)
	set(_properties)
	foreach(_prop ${ARGN})
		set(_properties "${_properties} ${_prop}")
	endforeach(_prop)
	get_target_property(_old_properties ${_target} ${_name})
	##message(STATUS "adding property to ${_target} ${_name}:" ${_properties})
	if(NOT _old_properties)
		# in case it's NOTFOUND
		set(_old_properties)
	endif(NOT _old_properties)
	set_target_properties(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")
endmacro(add_target_properties)

#
# Avro compiler
#
macro(avro_compile _src_files_var)
	set(_new_src_files)
	foreach(_src_file ${${_src_files_var}})
		if(_src_file MATCHES ".avro$")
			string(REGEX REPLACE ".avro$" "_avro.c" _src_file_out ${_src_file})
			string(REGEX REPLACE ".avro$" "_avro.h" _header_file_out ${_src_file})
#			add_custom_command(OUTPUT ${_src_file_out} ${_header_file_out}
#				COMMAND protoc --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${_src_file}
#				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#				DEPENDS ${_src_file}
#			)
			set_source_files_properties(${_src_file_out} PROPERTIES GENERATED TRUE)
			set_source_files_properties(${_header_file_out} PROPERTIES GENERATED TRUE)
#			set(_new_src_files ${_new_src_files} ${_src_file_out})
		else(_src_file MATCHES ".avro$")
			set(_new_src_files ${_new_src_files} ${_src_file})
		endif(_src_file MATCHES ".avro$")
	endforeach(_src_file)
	set(${_src_files_var} ${_new_src_files})
endmacro(avro_compile _src_files_var)

#
# protobuf compiler
#
macro(protobuf_compile _src_files_var)
	set(_new_src_files)
	foreach(_src_file ${${_src_files_var}})
		if(_src_file MATCHES ".proto$")
			string(REGEX REPLACE ".proto$" ".pb.cc" _src_file_out ${_src_file})
			string(REGEX REPLACE ".proto$" ".pb.h" _header_file_out ${_src_file})
			add_custom_command(OUTPUT ${_src_file_out} ${_header_file_out}
				COMMAND protoc --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${_src_file}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				DEPENDS ${_src_file}
			)
			set_source_files_properties(${_src_file_out} PROPERTIES GENERATED TRUE)
			set_source_files_properties(${_header_file_out} PROPERTIES GENERATED TRUE)
			set(_new_src_files ${_new_src_files} ${_src_file_out})
		else(_src_file MATCHES ".proto$")
			set(_new_src_files ${_new_src_files} ${_src_file})
		endif(_src_file MATCHES ".proto$")
	endforeach(_src_file)
	set(${_src_files_var} ${_new_src_files})
endmacro(protobuf_compile _src_files_var)

