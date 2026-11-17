#pragma once

// C++ has no built-in multimethods (Stroustrup rejected them for the
// language -- see this chapter's README) but double dispatch emulates
// one: resolving `multiop` needs TWO virtual calls, the first on the
// receiver's dynamic type, the second on the argument's dynamic type,
// which together pick the right overload out of all (receiver, argument)
// type combinations.

class derived_a;
class derived_b;

class base
{
public:
    virtual ~base();

    // Each overload here is itself an ordinary single-dispatch virtual
    // call. Dispatching on BOTH operands' dynamic types is achieved by
    // chaining two such calls together, not by any single language
    // feature.
    virtual void multiop(base& other) = 0;
    virtual void multiop(derived_a& other) = 0;
    virtual void multiop(derived_b& other) = 0;
};

class derived_a : public base
{
public:
    // First dispatch: picks derived_a::multiop(base&) based on *this*
    // object's dynamic type. It then calls back into `other`, which
    // performs the second dispatch based on `other`'s dynamic type,
    // now knowing the first operand is specifically a derived_a.
    void multiop(base& other) override;
    void multiop(derived_a& other) override;
    void multiop(derived_b& other) override;
};

class derived_b : public base
{
public:
    void multiop(base& other) override;
    void multiop(derived_a& other) override;
    void multiop(derived_b& other) override;
};
