set(
        public_src
        apue.c
)

add_library(apue ${public_src})
install(TARGETS apue DESTINATION lib)