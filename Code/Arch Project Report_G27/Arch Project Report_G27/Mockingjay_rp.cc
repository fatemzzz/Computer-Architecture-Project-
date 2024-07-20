#include "mem/cache/replacement_policies/Mockingjay_rp.hh"
#include "mem/cache/replacement_policies/MockingjayReplacementData.hh"
#include "base/random.hh"
#include "params/MockingjayRP.hh"
#include "sim/cur_tick.hh"
#include <cassert>
#include <memory>

namespace gem5
{

namespace replacement_policy
{

Mockingjay::Mockingjay(const Params *p)
    : Base(p), sampled_cache(512, 5), rdp(2048), clock(0)
{
    etr_counters.resize(p->size, std::vector<uint8_t>(p->assoc, 0));
}

void Mockingjay::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
{
    auto data = std::static_pointer_cast<MockingjayReplacementData>(replacement_data);
    data->ctr = 0;
}

void Mockingjay::touch(const std::shared_ptr<ReplacementData>& replacement_data)
{
    auto data = std::static_pointer_cast<MockingjayReplacementData>(replacement_data);

    Addr block_address = replacement_data->pkt->getBlockAddr(7);
    uint16_t block_address_hash = hashBlockAddress(block_address);
    uint16_t pc_signature = getPCSignature(replacement_data->pkt);
    Tick last_touch_tick;
    Tick current_tick = curTick();

    if (sampled_cache.access(block_address_hash, pc_signature, current_tick, last_touch_tick)) {
        uint8_t reuse_distance = (current_tick - last_touch_tick) / FACTOR;
        rdp.update(pc_signature, reuse_distance);
    } else {
        rdp.update(pc_signature, INF_RD);
        sampled_cache.update(block_address_hash, pc_signature, current_tick);
    }
    data->ctr = rdp.access(pc_signature);
}

void Mockingjay::reset(const std::shared_ptr<ReplacementData>& replacement_data)
{
    auto data = std::static_pointer_cast<MockingjayReplacementData>(replacement_data);
    uint16_t pc_signature = getPCSignature(replacement_data->pkt);
    data->ctr = rdp.access(pc_signature);
}

ReplaceableEntry* Mockingjay::getVictim(const ReplacementCandidates& candidates) const
{
    ReplaceableEntry* victim = nullptr;
    uint8_t max_etr = 0;

    for (const auto& candidate : candidates) {
        auto data = std::static_pointer_cast<MockingjayReplacementData>(candidate->replacementData);
        if (data->ctr > max_etr) {
            victim = candidate;
            max_etr = data->ctr;
        }
    }
    return victim;
}

std::shared_ptr<ReplacementData> Mockingjay::instantiateEntry()
{
    return std::make_shared<MockingjayReplacementData>();
}

uint16_t Mockingjay::getPCSignature(PacketPtr pkt) const
{
    if (pkt->req->hasPC()) {
        Addr pc = pkt->req->getPC();
        uint16_t pc_hash = pc & 0x7FF;
        uint16_t signature = pc_hash << 1;
        return signature;
    }
    return 0;
}

uint16_t Mockingjay::hashBlockAddress(Addr block_address) const
{
    return (block_address >> 4) & 0x3FF;
}

} // namespace replacement_policy
} // namespace gem5
