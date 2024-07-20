#ifndef MOCKINGJAY_RP_HH
#define MOCKINGJAY_RP_HH

#include "mem/cache/replacement_policies/base.hh"
#include "mem/cache/replacement_policies/MockingjayReplacementData.hh"
#include <vector>
#include <unordered_map>
#include <algorithm>

namespace gem5
{

namespace replacement_policy
{
struct MuckingjayParams;
class Mockingjay : public Base
{
  private:
    struct SampledLine {
        bool valid;
        uint16_t block_address_hash;
        uint16_t pc_signature;
        Tick lastTouchTick;

        SampledLine() : valid(false), block_address_hash(0), pc_signature(0), lastTouchTick(0) {}
    };

    class SampledCache {
      private:
        std::vector<std::vector<SampledLine>> sets;
        size_t set_count;
        size_t associativity;

      public:
        SampledCache(size_t set_count, size_t associativity);

        bool access(uint16_t block_address_hash, uint16_t pc_signature, Tick current_tick, Tick& last_touch_tick);
        void update(uint16_t block_address_hash, uint16_t pc_signature, Tick current_tick);
    };

    class RDP {
      private:
        std::vector<uint8_t> table;

      public:
        RDP(size_t size);

        void update(uint16_t pc_signature, uint8_t reuse_distance);
        uint8_t access(uint16_t pc_signature) const;
    };

    SampledCache sampled_cache;
    RDP rdp;
    std::vector<std::vector<uint8_t>> etr_counters;
    uint8_t clock;

    static constexpr uint8_t FACTOR = 8;
    static constexpr uint8_t INF_RD = 127;

    uint16_t getPCSignature(PacketPtr pkt) const;
    uint16_t hashBlockAddress(Addr block_address) const;

  public:
    typedef MockingjayParams Params;
    Mockingjay(const Params *p);

    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data) override;
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) override;
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) override;
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace replacement_policy
} // namespace gem5

#endif
