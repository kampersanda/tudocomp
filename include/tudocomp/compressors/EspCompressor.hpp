#ifndef _INCLUDED_ESP_COMPRESSOR_HPP
#define _INCLUDED_ESP_COMPRESSOR_HPP

#include <tudocomp/util.hpp>
#include <tudocomp/Env.hpp>
#include <tudocomp/Compressor.hpp>
#include <tudocomp/ds/GenericIntVector.hpp>

namespace tdc {

using int_vector::GenericIntVector;

class EspCompressor: public Compressor {

// Implementation that covers all of 64 bit
// TODO: Does the Paper mean base-e or base-2 ?
inline size_t iter_log(size_t n) {
    if (n < 3) return 1;
    if (n < 16) return 2;
    if (n < 3814280) return 3;
    return 4;
}

struct MetaBlock {
    size_t type;
    View view;
};

void meta_blocks_debug(const std::vector<MetaBlock> meta_blocks, View in) {
    {
        std::stringstream ss;
        for (auto& mb : meta_blocks) {
            ss << mb.view;
        }
        CHECK(ss.str() == std::string(in));
    }

    std::cout << "|";
    for (auto& mb : meta_blocks) {
        size_t w = mb.view.size() / 2;
        std::cout << std::setw(w) << "";
        std::cout << std::left << std::setw(mb.view.size() - w) << mb.type;
        std::cout << "|";
    }
    std::cout << "\n";

    std::cout << "|";
    for (auto& mb : meta_blocks) {
        std::cout << mb.view << "|";
    }
    std::cout << "\n";
}

template<class T>
uint64_t calc_alphabet_size(const T& t) {
    Counter<typename T::value_type> c;
    for (auto v : t) {
        c.increase(v);
    }
    return c.getNumItems();
}

template<class T>
bool no_adjacent_identical(const T& t) {
    for(size_t i = 1; i < t.size(); i++) {
        if (t[i] == t[i - 1]) return false;
    }
    return true;
}

uint64_t label(uint64_t left, uint64_t right) {
    auto diff = left ^ right;

    //std::cout << "l: " << std::setbase(2) << left << "\n";
    //std::cout << "r: " << std::setbase(2) << right << "\n";
    //std::cout << "d: " << std::setbase(2) << diff << "\n";
    //std::cout << "\n";


    DCHECK(diff != 0);

    auto l = __builtin_ctz(diff);

    auto bit = [](uint8_t l, uint64_t v) {
        // TODO: test
        return (v >> l) & 1;
    };

    // form label(A[i])
    return 2*l + bit(l, right);
};

template<class T, class F>
void for_neigbors(T& t, F f) {
    for (size_t i = 0; i < t.size(); i++) {
        typename T::value_type neighbors[2];
        uint8_t neighbor_len = 0;

        if (i == 0 && i == t.size() - 1) {
            neighbor_len = 0;
        } else if (i == 0) {
            neighbor_len = 1;
            neighbors[0] = t[i + 1];
        } else if (i == t.size() - 1) {
            neighbor_len = 1;
            neighbors[0] = t[i - 1];
        } else {
            neighbor_len = 2;
            neighbors[0] = t[i - 1];
            neighbors[1] = t[i + 1];
        }

        f(i, neighbors, neighbor_len);
    }
}

template<class T>
bool check_landmarks(const T& t) {
    size_t last = 0;
    size_t i = 0;
    for(; i < t.size(); i++) {
        if (t[i] == 1u) {
            if (i > 2) return false;
            last = i;
            i++;
            break;
        }
    }
    for(; i < t.size(); i++) {
        if (t[i] == 1u) {
            if ((i - last) > 3 || (i - last) < 2) return false;
            last = i;
        }
    }
    return true;
}

template<class F>
inline void handle_meta_block_2(View A,
                                uint64_t alphabet_size,
                                std::vector<uint8_t>& buf,
                                F debug_push_meta_block) {
    debug_push_meta_block(3, A.substr(0, iter_log(alphabet_size)));
    debug_push_meta_block(2, A.substr(iter_log(alphabet_size)));
    buf.clear();
    buf.insert(buf.cbegin(), A.cbegin(), A.cend());

    std::cout << "  " << vec_to_debug_string(buf) << "\n";
    std::cout << "  " << "Reduce to 6:\n";

    for (uint shrink_i = 0; shrink_i < iter_log(alphabet_size); shrink_i++) {
        for (size_t i = 1; i < buf.size(); i++) {
            auto left  = buf[i - 1];
            auto right = buf[i];
            buf[i - 1] = label(left, right);
        }
        buf.pop_back();

        std::cout << "  " << vec_to_debug_string(buf) << "\n";
    }

    DCHECK(calc_alphabet_size(buf) <= 6);

    std::cout << "  " << "Reduce to 3:\n";

    // TODO: This would benefit from a general, mutable, slice type

    // final pass: reduce to alphabet 3
    for(uint to_replace = 3; to_replace < 6; to_replace++) {
        for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
            auto& e = buf[i];
            if (e == to_replace) {
                e = 0;
                for (uint8_t j = 0; j < neighbor_len; j++) {
                    if (neighbors[j] == e) {
                        e++;
                    }
                }
                for (uint8_t j = 0; j < neighbor_len; j++) {
                    if (neighbors[j] == e) {
                        e++;
                    }
                }
            }
        });

        std::cout << "  " << vec_to_debug_string(buf) << "\n";
    }

    DCHECK(calc_alphabet_size(buf) <= 3);
    DCHECK(no_adjacent_identical(buf));

    // find landmarks:

    // TODO: Maybe store in high bits of buf to reduce memory?
    // buf gets reduced to 2 bit values anyway, and stays around long enough
    GenericIntVector<uint_t<1>> landmarks(buf.size());

    for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
        bool is_high_landmark = true;
        for (uint8_t j = 0; j < neighbor_len; j++) {
            if (neighbors[j] > buf[i]) {
                is_high_landmark = false;
            }
        }
        if (is_high_landmark) {
            landmarks[i] = 1;
        }
    });

    std::cout << "  High Landmarks:\n";
    std::cout << "  " << vec_to_debug_string(landmarks) << "\n";

    for_neigbors(buf, [&](size_t i, uint8_t neighbors[], uint8_t neighbor_len) {
        bool is_low_landmark = true;
        for (uint8_t j = 0; j < neighbor_len; j++) {
            if (neighbors[j] < buf[i]) {
                is_low_landmark = false;
            }
        }
        if (is_low_landmark) {
            if (   (!(i > 0)              || (landmarks[i - 1] == 0u))
                && (!(i < buf.size() - 1) || (landmarks[i + 1] == 0u))
            ) {
                landmarks[i] = 1;
            }
        }
    });

    std::cout << "  High and Low Landmarks:\n";
    std::cout << "  " << vec_to_debug_string(landmarks) << "\n";

    DCHECK(check_landmarks(landmarks));

}

template<class F>
inline void handle_meta_block_13(uint type,
                                 View A,
                                 uint64_t alphabet_size,
                                 std::vector<uint8_t>& buf,
                                 F debug_push_meta_block) {
    debug_push_meta_block(type, A);
}

public:
    inline static Meta meta() {
        Meta m("compressor", "esp", "ESP based grammar compression");
        //m.option("coder").templated<coder_t>();
        //m.option("min_run").dynamic("3");
        return m;
    }

    using Compressor::Compressor;

    inline virtual void compress(Input& input, Output& output) override {
        auto in = input.as_view();

        size_t alphabet_size = 256;

        std::vector<MetaBlock> meta_blocks;
        std::vector<uint8_t> buf;
        std::vector<View> blocks;

        {
            auto push_meta_block = [&](size_t type, View A) {
                meta_blocks.push_back(MetaBlock { type, A });
            };

            size_t i = 0;

            while(i < in.size()) {
                auto type_1_start = i;
                while ((i < (in.size() - 1)) && (in[i] == in[i + 1])) {
                    i++;
                }
                if ((i - type_1_start) > 0) {
                    View A = in.substr(type_1_start, i + 1);
                    handle_meta_block_13(1, A, alphabet_size, buf, push_meta_block);
                    std::cout << "  ---\n";
                    i++;
                }

                auto type_23_start = i;
                while (i < (in.size() - 1) && in[i] != in[i + 1]) {
                    i++;
                }

                if (i == in.size() - 1) {
                    i++;
                }
                size_t type_23_len = i - type_23_start;
                if (type_23_len > 0) {
                    View A = in.substr(type_23_start, i);

                    if (type_23_len >= iter_log(alphabet_size)) {
                        handle_meta_block_2(A, alphabet_size, buf, push_meta_block);
                        std::cout << "  ---\n";
                    } else {
                        handle_meta_block_13(3, A, alphabet_size, buf, push_meta_block);
                        std::cout << "  ---\n";
                    }
                }
            }

            std::cout << "Final meta blocks:\n";
            meta_blocks_debug(meta_blocks, in);
        }
    }

    inline virtual void decompress(Input& input, Output& output) override {

    }
};

}

#endif
