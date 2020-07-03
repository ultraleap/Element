#include "testsetup_jit_native.h"

#include "test_maths_scalar.h"
#include "test_maths_vector.h"


int main(int argc, char** argv)
{
    register_suite_maths_scalar();
    register_suite_maths_vector();

    return CU_CI_main(argc, argv);
}