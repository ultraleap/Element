#include "testsetup_interpreter.h"

#include "test_maths_scalar.h"
#include "test_maths_vector.h"
#include "test_util.h"
#include "test_trig.h"


int main(int argc, char** argv)
{
    register_suite_maths_scalar();
    register_suite_maths_vector();
    register_suite_util();
    register_suite_trig();

    return CU_CI_main(argc, argv);
}