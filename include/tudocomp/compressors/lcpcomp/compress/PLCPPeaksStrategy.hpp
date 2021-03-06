#pragma once

#include <vector>

#include <tudocomp/ds/IntVector.hpp>

#include <tudocomp/Algorithm.hpp>
#include <tudocomp/ds/TextDS.hpp>

#include <tudocomp/compressors/lzss/LZSSFactors.hpp>

#include <tudocomp_stat/StatPhase.hpp>

namespace tdc {
namespace lcpcomp {

/// A very naive selection strategy for LCPComp.
///
/// TODO: Describe
class PLCPPeaksStrategy : public Algorithm {
public:
    using Algorithm::Algorithm;

    inline static Meta meta() {
        Meta m("lcpcomp_comp", "plcppeaks", "using peaks of PLCP array");
        return m;
    }

    inline static ds::dsflags_t textds_flags() {
        return ds::SA | ds::ISA | ds::PLCP;
    }

    template<typename text_t>
    inline void factorize(text_t& text,
                   size_t threshold,
                   lzss::FactorBuffer& factors) {

		// Construct SA, ISA and LCP
        StatPhase::wrap("Construct text ds", [&]{
            text.require(text_t::SA | text_t::ISA | text_t::PLCP);
        });

		StatPhase::wrap("Search Peaks", [&]{
            const auto& sa = text.require_sa();
            const auto& isa = text.require_isa();

            auto plcp = text.release_plcp();

            //
            const size_t n = sa.size();

		    len_t last_replacement_pos = 0;
		    for(len_t i = 0; i+1 < n; ) {
			    if( (i == last_replacement_pos || plcp[i] > plcp[i-1]) && plcp[i] > plcp[i+1] && plcp[i] >= threshold) {
				    DCHECK_NE(isa[i], 0u);
				    const len_t& target_position = i;
				    const len_t factor_length = plcp[target_position];
				    DCHECK_LT(target_position+factor_length,n);
				    const len_t source_position = sa[isa[target_position]-1];
				    factors.emplace_back(i, source_position, factor_length);
				    // for(size_t k = 0; k < factor_length; ++k) {
				    // 	plcp[target_position + k] = 0;
				    // }
				    // const len_t affected_length = std::min(factor_length, target_position);
				    // for(size_t k = 0; k < affected_length; ++k) {
				    // 	const len_t affected_position = target_position - k - 1;
				    // 	if(target_position < affected_position + plcp[affected_position]) {
				    // 		const len_t affected_length = target_position - affected_position;
				    // 		plcp[affected_position] = affected_length;
				    // 	}
				    // }
				    i+= factor_length;
				    last_replacement_pos = i-1;
			    }
			    else {
				    ++i;
			    }
		    }
        });
    }
};

}}//ns

