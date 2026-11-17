#pragma once

// The diamond problem and how `virtual` inheritance fixes it.
//
// Non-virtual diamond: base_v is inherited twice (once through each of
// left_n/right_n), so bottom_n contains TWO base_v sub-objects, and any
// unqualified access to a base_v member through bottom_n is ambiguous.
//
// Virtual diamond: left_v and right_v inherit base_v virtually, so
// bottom_v contains exactly ONE shared base_v sub-object -- and it is
// bottom_v (the most-derived class), not left_v or right_v, that is
// responsible for constructing it.

class base_v
{
public:
    base_v();
    virtual ~base_v();
    virtual void identify() const;

protected:
    int _tag;
};

class left_n : public base_v
{
};

class right_n : public base_v
{
};

class bottom_n : public left_n, public right_n
{
};

class left_v : virtual public base_v
{
};

class right_v : virtual public base_v
{
};

// bottom_v must initialize base_v itself, even though neither left_v nor
// right_v looks like it needs to -- that is the price of sharing a virtual
// base: only the most-derived class gets to decide how it is built.
class bottom_v : public left_v, public right_v
{
public:
    bottom_v();
};
