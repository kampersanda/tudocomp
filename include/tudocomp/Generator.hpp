#pragma once

#include <functional>
#include <memory>

#include <tudocomp/pre_header/Registry.hpp>
#include <tudocomp/pre_header/Env.hpp>
#include <tudocomp/Algorithm.hpp>

namespace tdc {

/// \brief Base for string generators.
class Generator : public Algorithm {
public:
    using Algorithm::Algorithm;

    /// \brief Generates a string based on the environment settings.
    /// \return the generated string.
    virtual std::string generate() = 0;
};

}

