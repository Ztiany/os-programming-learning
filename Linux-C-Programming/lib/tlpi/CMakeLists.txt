aux_source_directory(. SOURCE_FILES)
add_library(tlpi ${SOURCE_FILES})
install(TARGETS tlpi DESTINATION lib)
