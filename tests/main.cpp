#include "choc_tests.hpp"
#include "choc_ArgumentList.hpp"

//==============================================================================
int main (int argc, const char** argv)
{
    choc::ArgumentList args (argc, argv);
    choc::test::TestProgress progress;
    return choc_unit_tests::runAllTests (progress, args.contains ("--multithread")) ? 0 : 1;
}
