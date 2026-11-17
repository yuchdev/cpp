#include "codegen.h"

#include <iostream>

using std::cout;
using std::endl;

code_generator_base::code_generator_base(int id) : _id(id)
{
}

code_generator_base::~code_generator_base()
{
}

void code_generator_base::generate_base() const
{
    cout << "code_generator_base::generate_base id=" << _id << '\n';
}

cpp_code_generator::cpp_code_generator(int id) : code_generator_base(id)
{
}

void cpp_code_generator::generate_cpp() const
{
    cout << "cpp_code_generator::generate_cpp id=" << _id << '\n';
}

php_code_generator::php_code_generator(int id) : code_generator_base(id)
{
}

void php_code_generator::generate_php() const
{
    cout << "php_code_generator::generate_php id=" << _id << '\n';
}

// code_formatter is the most-derived class, so it alone is responsible
// for initializing the shared virtual base code_generator_base; the
// code_generator_base(id) mentioned in cpp_code_generator's and
// php_code_generator's own constructors is skipped here.
code_formatter::code_formatter(int id)
    : code_generator_base(id), cpp_code_generator(id), php_code_generator(id)
{
}

void code_formatter::generate(language lang) const
{
    if (lang == cpp_lang)
        generate_cpp();
    else
        generate_php();
}

finalizer::~finalizer()
{
}
