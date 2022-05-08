#include "properties.h"
#include "func_types.h"
#include <iostream>

using std::cout;
using std::endl;

// Demonstration of simple accumulation and the resulting overflow
void show_simple_accum()
{
    int iarr[] = { 1,2,3,4,5 };
    int i_sum = simple_accumulate<int>(iarr, iarr + sizeof(iarr) / sizeof(int));
    cout << "simple accumulate integer: " << i_sum << endl;

    char name[] = "templates";
    char ch_sum = simple_accumulate<char>(name, name + sizeof(name) - 1);
    cout << "simple accumulate character: " << ch_sum << endl;
}

// Demonstration of type traits
void show_traits()
{
    int iarr[] = { 1,2,3,4,5 };
    int i_sum = traits_accumulate<int>(iarr, iarr + sizeof(iarr) / sizeof(int));
    cout << "traits accumulate integer: " << i_sum << endl;

    typedef simple_accumulator_traits<char>::AccType RetType;
    char name[] = "templates";
    RetType ch_sum = traits_accumulate<char>(name, name + sizeof(name) - 1);
    cout << "traits accumulate character: " << ch_sum << endl;
}

// Demonstration of  value traits
void show_value_traits()
{
    int iarr[] = { 1,2,3,4,5 };
    int i_sum = traits_vals_accumulate<int>(iarr, iarr + sizeof(iarr) / sizeof(int));
}

// Demonstration of  traits with params
void show_parametrics_traits()
{
    int iarr[] = { 1,2,3,4,5 };
    int i_sum = custom_accumulator_traits<int>::accum(iarr, iarr + sizeof(iarr) / sizeof(int));

}

// Demonstration of strategy class
void show_strategies()
{
    int iarr[] = { 1,2,3,4,5 };
    int isum = strategy_accumulator_traits<int>::accum(iarr, iarr + sizeof(iarr) / sizeof(int));

    // As you can see, the accumulation by multiplication gives 0, because the accumulation element is initialized by zero
    // Probably need an element = 1, as well as a zero element, which is part of the strategy
    int imlpl = strategy_accumulator_traits<int, MultPolicy, value_multiply_traits<int>>::accum(iarr, iarr + sizeof(iarr) / sizeof(int));
}

// Strategy with template argument 
void show_strategies_tmpl_args()
{
    int iarr[] = { 1,2,3,4,5 };
    int isum = strategy_tmpl_accumulator_traits<int>::accum(iarr, iarr + sizeof(iarr) / sizeof(int));
    int imlpl =
        strategy_tmpl_accumulator_traits<int, MultPolicyT >::accum(iarr, iarr + sizeof(iarr) / sizeof(int), 1);
}

int main()
{

    show_simple_accum();
    show_traits();
    show_value_traits();
    show_parametrics_traits();
    show_strategies();
    show_strategies_tmpl_args();

    return 0;
}
