#include <gtest/gtest.h>

#include <sdsl/cst_sada.hpp>
#include <tudocomp/compressors/lz78u/SuffixTree.hpp>
#include "test/util.hpp"

using namespace tdc;
using namespace tdc::lz78u;

void test_strdepth(const std::string& str) {
	if(str.length() == 0) return;
	sdsl::cst_sada<> cst;
	sdsl::construct_im(cst, str, 1);
    SuffixTree st(cst);
	root_childrank_support rrank(cst.bp_support);
	for(auto node : cst) {
		ASSERT_EQ(st.str_depth(node), rrank.str_depth(st,node));
	}
}

TEST(SuffixTree, strdepth)     {
	test::roundtrip_batch(test_strdepth);

    //this never terminates. see bug #18662
	//test::on_string_generators(test_strdepth,11);
}
