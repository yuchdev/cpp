#pragma once
#include <iostream>
using std::cout;
using std::endl;

// Chapter 16.3 Nicolai Josuttis
// Named arguments, dominance rule

// If a class has a large number of parameters by default, 
// it is more convenient to specify one of them by name during overriding
struct DefaultPolicy1
{
    static void doprint()
    {
        cout << "DefaultPolicy1::doprint" << endl;
    }
};

struct DefaultPolicy2
{
    static void doprint()
    {
        cout << "DefaultPolicy2::doprint" << endl;
    }
};

struct DefaultPolicy3
{
    static void doprint()
    {
        cout << "DefaultPolicy3::doprint" << endl;
    }
};

struct DefaultPolicy4
{
    static void doprint()
    {
        cout << "DefaultPolicy4::doprint" << endl;
    }
};

struct CustomPolicy
{
    static void doprint()
    {
        cout << "CustomPolicy::doprint" << endl;
    }
};

// The bread_slicer class accepts a set of default strategies
// The purpose is to specify only strategies different from the defaults
template<	typename Policy1 = DefaultPolicy1,
    typename Policy2 = DefaultPolicy2,
    typename Policy3 = DefaultPolicy3,
    typename Policy4 = DefaultPolicy4 >
    class bread_slicer
{
public:
    bread_slicer() {}
};

// The solution:
// Place the default types in a base class
// the discriminator class is only needed to pass several 
// identical classes as a base class (they differ in number only).
template <typename Base, int D>
class discriminator : public Base {};

template <typename Setter1, typename Setter2, typename Setter3, typename Setter4 >
class policy_selector : public discriminator<Setter1, 1>,
    public discriminator<Setter2, 2>,
    public discriminator<Setter3, 3>,
    public discriminator<Setter4, 4>
{};

class default_policies
{
public:
    typedef DefaultPolicy1 P1;
    typedef DefaultPolicy2 P2;
    typedef DefaultPolicy3 P3;
    typedef DefaultPolicy4 P4;
};

// use the fact that typedef overrides the template parameter and is inherited

// Default strategies
class default_policy_args : virtual public default_policies {};

// Redefine type P1 as a Policy (template parameter)
template <typename Policy>
struct policy1_is : virtual public default_policies
{
    typedef Policy P1;
};

// Redefine type P2 as a Policy
template <typename Policy>
struct policy2_is : virtual public default_policies
{
    typedef Policy P2;
};

// Redefine type P3 as a Policy
template <typename Policy>
struct policy3_is : virtual public default_policies
{
    typedef Policy P3;
};

// Redefine type P4 as a Policy
template <typename Policy>
struct policy4_is : virtual public default_policies
{
    typedef Policy P4;
};

// policyN_is and default_policy_args inherit from default_policies
// policyN_is redefines one of the strategies 
// according to the Rule of Dominance 
template <	typename PolicySetter1 = default_policy_args,
    typename PolicySetter2 = default_policy_args,
    typename PolicySetter3 = default_policy_args,
    typename PolicySetter4 = default_policy_args >
    class bread_slicer2
{
public:

    typedef policy_selector<PolicySetter1, PolicySetter2, PolicySetter3, PolicySetter4> policies;

    bread_slicer2() {}

    void doprint()
    {
        // Call different strategies
        policies::P1::doprint();
        policies::P2::doprint();
        policies::P3::doprint();
        policies::P4::doprint();
        // Since the strategy classes are virtual, memory usage is minimal
    }
};

// Now when you instantiate it, it will be something like this
void show_name_params()
{
    // Use with an explicit set of parameters
    bread_slicer< DefaultPolicy1, DefaultPolicy2, CustomPolicy > bs1;

    // Using the "pattern" Policy Selector - redefining strategy #3
    bread_slicer2< policy3_is<CustomPolicy> > bs2;

    // Execute strategies
    bs2.doprint();
}
