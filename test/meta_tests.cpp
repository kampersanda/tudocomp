#include <glog/logging.h>
#include <gtest/gtest.h>

#include <tudocomp/meta/AlgorithmDecl.hpp>
#include <tudocomp/meta/ast/Parser.hpp>
#include <tudocomp/meta/AlgorithmConfig.hpp>

using namespace tdc::meta;

TEST(Sandbox, example) {
    auto lz77 = AlgorithmDecl("lz77", "compressor", "LZ77 online compressor.");
    lz77.add_param(AlgorithmDecl::Param("window"));
    lz77.add_param(AlgorithmDecl::Param("coder", false, false, "coder"));
    //DLOG(INFO) << lz77.str();

    auto binary = AlgorithmDecl("binary", "coder", "Binary coder.");
    //DLOG(INFO) << binary.str();

    // registry
    AlgorithmDict dict;
    dict.emplace("lz77", std::move(lz77));
    dict.emplace("binary", std::move(binary));

    // parse
    DLOG(INFO) << "parse...";
    auto v = ast::Parser::parse("lz77(coder=binary,window=10)");
    DLOG(INFO) << v->str();

    // attempt to create config
    DLOG(INFO) << "config...";
    auto cfg = AlgorithmConfig(dict.at("lz77"), v.get(), dict);
    DLOG(INFO) << cfg.str();
}
