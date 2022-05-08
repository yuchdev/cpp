#pragma once

// Static branching pattern IfThenElse,
// returning a type depending on the condition

// Basic template is undefined
template <bool B, typename Tr, typename Fls>
struct IfThenElse;

// Partial specialization for True
template <typename Tr, typename Fls>
struct IfThenElse<true, Tr, Fls>
{
    typedef Tr Result;
};

// Partial specialization for False
template <typename Tr, typename Fls>
struct IfThenElse<false, Tr, Fls>
{
    typedef Fls Result;
};

