#pragma once

// A small virtual-inheritance diamond, built specifically to give
// dynamic_cast something non-trivial to do: code_formatter sits at the
// bottom, with cpp_code_generator and php_code_generator as sibling
// branches that both share one code_generator_base. Because the shared
// base is virtual, RTTI can navigate from either sibling to the other
// (crosscasting) or down to code_formatter (downcasting).

enum language
{
    cpp_lang,
    php_lang
};

class code_generator_base
{
public:
    explicit code_generator_base(int id);
    virtual ~code_generator_base();
    virtual void generate_base() const;

protected:
    int _id;
};

class cpp_code_generator : public virtual code_generator_base
{
public:
    explicit cpp_code_generator(int id);
    virtual void generate_cpp() const;
};

class php_code_generator : public virtual code_generator_base
{
public:
    explicit php_code_generator(int id);
    virtual void generate_php() const;
};

class code_formatter : public cpp_code_generator, public php_code_generator
{
public:
    explicit code_formatter(int id);
    void generate(language lang) const;
};

// An unrelated polymorphic type, used only to show that dynamic_cast
// correctly refuses to cast to a type that is not actually part of the
// hierarchy being queried.
class finalizer
{
public:
    virtual ~finalizer();
};
