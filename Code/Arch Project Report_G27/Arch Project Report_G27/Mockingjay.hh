#include "mem/cache/replacement_policies/base.hh"

namespace gem5
{

struct MockingjayParams : BaseReplacementPolicyParams
{
    size_t size;
    size_t assoc;
};

}
