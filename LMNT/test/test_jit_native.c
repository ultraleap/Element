#include "testsetup_jit_native.h"

#include "test_maths_scalar.h"
#include "test_maths_vector.h"
#include "test_util.h"
#include "test_trig.h"
#include "test_misc.h"


int main(int argc, char** argv)
{
    register_suite_maths_scalar();
    register_suite_maths_vector();
    register_suite_util();
    register_suite_trig();
    register_suite_misc();

    return CU_CI_main(argc, argv);
}