#include <Vc/Vc>
#include <Vc/IO>
#include <Vc/common/support.h>

using namespace Vc;
float_v foo0(float_v::AsArg a)
{
    const float_v b = a + float_v::One();
    std::cerr << b;
    return b;
}
