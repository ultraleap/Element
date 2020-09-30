add_executable(lmnt_testapp
    "main.c"
    "simple.h"
    "circle.h"
    "circle_ht.h"
    "simple125.h"
)

target_link_libraries(lmnt_testapp PRIVATE lmnt)
