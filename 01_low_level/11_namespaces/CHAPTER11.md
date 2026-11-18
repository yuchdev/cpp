# Chapter 11. Namespaces: Lookup, Identity, and the Architecture of C++ Programs

When a C++ library grows from one source file into dozens of components, the first problem is rarely that it cannot invent enough names. The problem is that the same good names are needed for different things. Every subsystem wants a `Parser`, a `Result`, a `begin`, a `swap`, and a `detail`. At first, prefixes seem sufficient: `network_parse`, `graphics_parse`, `storage_parse`. Eventually the prefixes form a second, informal language of their own.

Namespaces solve a more fundamental problem. They provide a language-level context in which declarations are named and found. That sounds like a simple grouping mechanism, but in C++ the consequences reach much further: namespaces participate in overload resolution, argument-dependent lookup, customization of generic algorithms, compatibility versioning, and the relationship between source APIs and compiled symbols. Their design shapes whether independently developed components can coexist without surprises.

A namespace is also defined by what it **does not** do. It does not create a runtime object, automatically hide a symbol from a shared-library export table, protect memory, govern ownership, or isolate preprocessing macros. C++20 modules add a different abstraction for organizing source dependencies; modules do not make namespaces obsolete.

This chapter develops namespace rules historically, from C++98 naming and lookup to C++26-era generic programming. The eight companion programs in this directory cover [basic namespaces](01_basic_namespaces/basic_namespaces.cpp), [using and lookup](02_using_and_lookup/using_and_lookup.cpp), [argument-dependent lookup](03_adl/adl.cpp), [namespaces and operators](04_namespaces_and_operators/namespaces_and_operators.cpp), [wrapping C interfaces](05_wrapping_c/wrapping_c.cpp), [inline namespaces](06_inline_namespaces/inline_namespaces.cpp), [namespaces versus modules](07_namespaces_vs_modules/namespaces_vs_modules.cpp), and [scopes](08_scopes/scopes.cpp). The repository's C++20 baseline remains in effect; C++23 and C++26 features are discussed with appropriate version boundaries.

## 1. The problem namespaces actually solve

Suppose two libraries choose an entirely reasonable function name:

~~~cpp
void open();
~~~

Without additional qualification, those declarations would compete in the global namespace. Adding more global functions with the same name may produce overloads, but unrelated libraries should not have to coordinate their overload sets. One library's header can otherwise change how another library's call resolves.

Namespaces let each library establish its own naming context:

~~~cpp
namespace network {
    void open();
}

namespace storage {
    void open();
}

void initialize()
{
    network::open();
    storage::open();
}
~~~

The qualification expresses more than a prefix. `network::open` is found through namespace-aware lookup and belongs to an overload and declaration environment associated with `network`. The compiler can enforce those relationships, whereas the prefix `network_open` is merely part of an ordinary identifier.

Named namespaces are also open for extension: a namespace can be defined in several locations, even in separate translation units, and compatible namespace definitions contribute declarations to the same namespace.

~~~cpp
namespace graphics {
    struct Point { int x, y; };
}

namespace graphics {
    double distance(Point, Point);
}
~~~

This is not a second namespace `graphics`. It extends the same named namespace. It allows headers, libraries, and implementation units to contribute related declarations without putting them all in one physical file.

But the compiler does not automatically order declarations by logical dependency merely because they are in the same namespace. Each translation unit still requires declarations to be reachable when they are used; the linker still needs the corresponding definitions. A namespace is a source-level naming mechanism, not a source inclusion or dependency manager.

## 2. Namespace membership and the name lookup process

To understand namespace-related bugs, start by separating **declaration membership** from **name lookup**.

A declaration belongs to a particular scope. Lookup decides what declarations a name refers to at a particular use. A namespace member might exist yet be inaccessible to ordinary unqualified lookup at a certain point, while a `using` directive can cause lookup to consider a declaration without moving the declaration out of its original namespace.

~~~cpp
namespace diagnostics {
    void report(int);
}

void run()
{
    diagnostics::report(7); // explicitly qualified
}
~~~

Qualification with `diagnostics::` asks lookup to find the member under that namespace. Without qualification, a call to `report(7)` would not normally find this particular function unless some additional lookup mechanism made it a candidate.

This difference matters as APIs evolve. Changing a namespace or adding a `using` directive does not merely affect spelling; it can change overload candidate sets, ambiguity, and whether previously hidden functions become visible.

### 2.1 Scope is not linkage

A declaration at namespace scope often has external linkage, but not always:

~~~cpp
namespace telemetry {
    int messages = 0;        // ordinarily external linkage
    static int retries = 0;  // internal linkage
    const int limit = 10;    // ordinarily internal linkage
}
~~~

The namespace groups all three names for lookup. Their linkage differs. A separate translation unit may refer to `telemetry::messages` through a suitable declaration, while the internal-linkage objects have separate identities in each translation unit that defines them.

The key distinction from Chapter 10 is worth repeating. **Scope controls where the name can be found; linkage controls when declarations identify the same entity across translation units; binary symbol visibility controls what an executable or shared library exposes to other binary components.** Namespace nesting does not collapse these questions.

In particular, `telemetry::messages` may be externally linked yet intentionally hidden from the dynamic symbol table of a shared library compiled with hidden visibility.

### 2.2 Types and aliases do not behave like ordinary external objects

The original README's summary of default internal linkage mixes together namespace-scope const objects, type aliases, and `static` declarations. These are not the same category. A namespace-scope alias does not normally define a runtime object with an independent internal-linkage copy in every translation unit:

~~~cpp
namespace protocol {
    using Sequence = std::uint64_t;
}
~~~

`Sequence` is a type alias, not a separately linked variable. It participates in type naming and lookup, not in the binary data-symbol model of a const object.

This matters when designing headers. Defining a type alias in many translation units is normal; defining a mutable non-inline external variable in the same header is usually not. Similar syntactic placement does not imply identical linkage semantics.

## 3. Unnamed namespaces and translation-unit-local implementation

A named namespace organizes an API. An **unnamed namespace** serves a different primary purpose: it supplies a translation-unit-local namespace identity.

~~~cpp
namespace {
    int cache_hits = 0;

    void reset_cache()
    {
        cache_hits = 0;
    }
}
~~~

In the traditional C++ model, unnamed namespace members receive internal linkage through the relevant language rules, and each translation unit has its own unnamed namespace. The pattern is usually preferred over putting `static` on every namespace-scope implementation helper.

This is useful for functions, variables, types, and templates that should not participate in the library's externally linked API. It also reduces the risk of clashes with names chosen by unrelated implementation files.

An unnamed namespace is not a runtime sandbox, however. Its objects can be passed by pointer or reference to code outside the translation unit. And it does not prevent a dependency on a header that exposes those implementation details. It confines the entity's *name identity*, not its memory.

### 3.1 An unnamed namespace in a header is usually suspicious

Consider a header containing:

~~~cpp
namespace {
    int next_id = 0;
}
~~~

Every translation unit that includes the header gets an independent `next_id`. If the programmer intended a single application-wide counter, the result is wrong even though the program may compile and link cleanly.

There are legitimate reasons to put translation-unit-local definitions in headers, but they need an explicit design. More often, implementation helpers belong in an unnamed namespace in the `.cpp` file that uses them. Header-only internal helpers may instead use appropriately `inline` functions in a named `detail` namespace, with the ODR and API consequences understood.

### 3.2 `detail` is a convention, not access control

Libraries frequently use:

~~~cpp
namespace library::detail {
    void implementation_hook();
}
~~~

This signals “not part of the supported public API.” It does not make declarations private in the C++ access-control sense. Users can still name `library::detail::implementation_hook` if the declaration is reachable, and its linkage/visibility may expose a compiled symbol.

Treat `detail` as a maintenance policy backed by documentation and tests, not as a compiler-enforced secrecy boundary. If source or ABI hiding is required, use suitable implementation files, module boundaries, visibility controls, or an opaque public interface.

## 4. Reopening and nesting namespaces

Named namespace definitions are extensible:

~~~cpp
namespace app {
    void start();
}

namespace app {
    void stop();
}
~~~

The two blocks contribute to one namespace `app`. This flexibility is essential for large libraries: different headers can publish related functions and types without centralizing all declarations in one physical file.

Namespaces can also nest:

~~~cpp
namespace app {
    namespace network {
        void connect();
    }
}
~~~

Since C++17 the shorter nested-namespace spelling is available:

~~~cpp
namespace app::network {
    void disconnect();
}
~~~

This changes syntax, not the lookup or visibility model. `app::network` does not become a package or dynamically loadable module because the source uses a nested-namespace definition.

When deciding how deeply to nest namespaces, prioritize semantic usefulness. A namespace tree mirroring every directory in a source repository can become awkward when directories are reorganized; a public namespace tree is a source API and should change less frequently than an internal folder layout.

### 4.1 Qualified declarations still have context rules

A namespace may be reopened, but a namespace alias cannot be used as the name of a namespace-definition:

~~~cpp
namespace company::v2 {
    void run();
}

namespace current = company::v2;

// namespace current { void stop(); } // ill-formed
~~~

An alias makes it possible to *refer to* an existing namespace with a shorter name. It does not create a new namespace or a second definition context.

The distinction is important when a project migrates from one versioned namespace to another: a namespace alias is a source-level path for lookup, not a way to relocate compiled entities.

## 5. Namespace aliases and compatibility names

A long vendor or version namespace can be painful to repeat:

~~~cpp
namespace long_vendor_library {
    namespace processing {
        void run();
    }
}

namespace processing = long_vendor_library::processing;
~~~

Code can now refer to `processing::run()` without changing where the original function is declared. The alias names a namespace; it neither copies definitions nor changes the mangled names of compiled functions.

This makes namespace aliases useful for local readability and gradual source migration. For example, an application can choose the version of a library's API it wants to call:

~~~cpp
namespace api = vendor::version_2;
~~~

Such an alias does not magically make `vendor::version_1` and `vendor::version_2` binary-compatible. If code previously linked against functions in a different namespace, changing the alias can select different decorated symbols and require rebuilding or replacing libraries.

An alias is also not a tool for merging independent namespace trees. It directs lookup to an existing namespace. If multiple independent APIs need a common facade, consider explicit forwarding declarations or functions rather than relying on aliases to supply compatibility behavior.

## 6. Using declarations versus using directives

C++ provides two mechanisms with similar-looking syntax but materially different lookup behavior.

A **using declaration** introduces particular declarations or an overload set under a selected name:

~~~cpp
using std::vector;
using std::string;
~~~

A **using directive** makes declarations from a nominated namespace eligible for certain unqualified lookup processes:

~~~cpp
using namespace std;
~~~

It is inaccurate to say that a using directive physically copies “all names” into the current scope. Its effect is on unqualified name lookup, following rules that consider nominated namespaces and their transitive effects. Those rules are precisely why directives can be surprising.

### 6.1 Why a using directive in a header is dangerous

A public header is included in many unrelated translation units. If it contains:

~~~cpp
using namespace vendor;
~~~

then client source code that never opted into the vendor namespace can acquire additional unqualified lookup candidates. A previously unambiguous expression may become ambiguous after an unrelated library update adds a new overload or class name.

The damage can be subtle because the error appears in the client's source, while the cause was introduced by a header include many levels away.

Keep using directives out of public headers, especially at namespace scope. In a small function body or a test file, a narrow using directive can be acceptable when the benefit is clear and its effect remains local.

### 6.2 Using declarations offer narrower dependencies

A using declaration makes the intended name explicit:

~~~cpp
void example()
{
    using std::swap;
    // use swap according to the relevant lookup rules
}
~~~

In many cases, direct qualification is even clearer:

~~~cpp
std::vector<int> values;
~~~

The trade-off is not between “always qualify” and “never qualify.” It is between controlling the lookup surface and accidentally importing more candidates than the code expects.

Using declarations involving function names also interact with overload sets. A declaration can make several overloads of the selected name available rather than merely one signature. Adding an overload to a namespace can therefore still affect downstream calls even when the using declaration looks narrow.

### 6.3 `using enum` is a separate C++20 feature

C++20's `using enum` introduces named enumerators into the current scope:

~~~cpp
enum class State { idle, running, stopped };

void handle(State state)
{
    using enum State;

    switch (state) {
        case idle:    break;
        case running: break;
        case stopped: break;
    }
}
~~~

This is convenient in a small local scope but can recreate the collisions that scoped enums were designed to avoid. It should not be confused with a namespace using directive. Avoid broad `using enum` declarations in public headers where common names such as `idle` or `error` can conflict with unrelated code.

## 7. Lookup is more than walking outward through scopes

One intuitive lookup model says that the compiler searches the innermost block, then enclosing blocks, then the namespace, then the global namespace. That is a useful beginning for ordinary unqualified lookup, but it is incomplete for C++.

C++ also supports qualified lookup, class member lookup, dependent lookup in templates, argument-dependent lookup, and special lookup rules for constructors, operators, and friend functions.

Consider ordinary nested scopes:

~~~cpp
int value = 1;

namespace settings {
    int value = 2;

    void show()
    {
        int value = 3;
        use(value);           // local 3
        use(settings::value); // namespace 2
        use(::value);         // global 1
    }
}
~~~

Qualification communicates which layer of name identity is intended. It also avoids depending on the presence or absence of a local shadowing declaration.

Shadowing is not necessarily a defect. A function parameter named `value` may be entirely reasonable. But accidental shadowing inside large or deeply nested functions can make maintenance fragile, particularly when an added declaration changes which name an existing expression denotes.

Compiler warnings such as `-Wshadow` can help detect suspicious cases, but they cannot decide whether a particular shadowing relationship is semantically intended.

## 8. Argument-dependent lookup: the essential extension

Suppose a library declares a type and a function that works on it:

~~~cpp
namespace geometry {
    struct Point {
        double x;
        double y;
    };

    void normalize(Point&);
}
~~~

A call can use the function without explicitly naming its namespace:

~~~cpp
geometry::Point p{3, 4};
normalize(p);
~~~

Why can this work even if ordinary unqualified lookup does not find `geometry::normalize`? Because the call supplies an argument of type `geometry::Point`. **Argument-dependent lookup** (ADL, also called Koenig lookup) considers namespaces and classes associated with argument types, under precise language rules.

This mechanism is not a global search for every function with the right name. The associations are derived from the argument types and their class/template relationships. It is not based on matching source text or searching all imported headers.

The reason ADL exists is practical: operations naturally associated with a type should be discoverable where that type lives. This is especially important for operators and generic customization functions.

### 8.1 ADL makes independently developed generic code possible

The classic `swap` customization pattern is:

~~~cpp
template<class T>
void exchange_values(T& a, T& b)
{
    using std::swap;
    swap(a, b);
}
~~~

The using declaration makes the standard `swap` candidates available; ADL can additionally find a suitable `swap` declared alongside a user-defined type. The generic algorithm need not know the user's namespace in advance.

This pattern has shaped much of the standard library's history. Customization point objects and C++20 ranges refine some of these ideas, but the fundamental relationship between an operation and its associated types remains central.

For new code, member operations and ordinary named functions are often sufficient. When a customization function is intentionally designed for generic use, place it in an associated namespace and document its constraints, exception guarantee, and semantics.

### 8.2 ADL can add unexpected candidates

Imagine a template that calls `process(x)` without qualification. A type introduced later can bring a new associated namespace into ADL, supplying additional overload candidates. An otherwise innocent call may change its best match or become ambiguous.

This is one reason generic code should be deliberate about when it uses unqualified calls as customization hooks and when it uses explicit qualified calls to prevent ADL.

Qualified calls normally suppress ADL for that name:

~~~cpp
geometry::normalize(p);
~~~

This selects declarations through qualified lookup rather than asking argument-associated namespaces to contribute additional candidates.

An API designer should therefore recognize **unqualified call syntax** as part of an extensibility contract, not merely an abbreviated spelling.

## 9. Hidden friends and operators

C++ permits a function to be declared as a friend inside a class:

~~~cpp
namespace network {
    struct Packet {
        int id{};

        friend bool operator==(const Packet& a,
                               const Packet& b)
        {
            return a.id == b.id;
        }
    };
}

bool same(network::Packet a, network::Packet b)
{
    return a == b;
}
~~~

The friend is a namespace-scope non-member function associated with `Packet`. It can be found through ADL, even though it may not be found by ordinary unqualified or namespace-qualified lookup without a matching namespace-scope declaration.

This **hidden-friend idiom** keeps an operator discoverable for relevant operand types without making it a broadly visible overload candidate for unrelated calls.

It is not a privacy feature. A hidden friend can access private members because it is a friend, but it is still a non-member function and can be an important part of the library's public source contract. Its signature may also affect binary compatibility when the function has externally linked compiled use.

### 9.1 Why non-member operators are often appropriate

A binary member operator treats the left operand as the implicit object parameter. A non-member operator can allow conversions on both operands according to overload resolution rules.

For value-like mathematical types, a non-member operator is often the most symmetric design:

~~~cpp
namespace math {
    struct Vector {
        double x, y;
    };

    Vector operator+(Vector a, Vector b)
    {
        return {a.x + b.x, a.y + b.y};
    }
}
~~~

ADL can find `math::operator+` when one or both operands are `math::Vector`. It is better to place such operators in the namespace associated with the operands than in the global namespace or the unrelated namespace of a caller.

Not every operator should be non-member: assignment, conversion, and certain other operators have language-specific member requirements; mutating operators often fit naturally as members. The design depends on the operation and conversion behavior. The point is to preserve an understandable and constrained overload surface.

### 9.2 Hidden friends are useful for constraints

A constrained hidden-friend operator can avoid accidentally accepting conversions or unrelated operand types. Since C++20, concepts and `requires` clauses make these constraints more explicit.

The hidden-friend technique is especially valuable in generic libraries, where a broadly visible unconstrained operator template can pollute overload resolution for otherwise unrelated types. The friend attaches candidate visibility to type association, reducing accidental interference.

This is not a substitute for testing. Two independent libraries can still create surprising ADL interactions when a type has nested or template-related associations. When an overload becomes ambiguous, inspect both ordinary and ADL-contributed candidates.

## 10. Two-phase lookup in templates

Template definitions create an additional source of lookup complexity: the point at which a template is **defined** may differ from the point at which it is **instantiated**.

For many non-dependent names, lookup is performed in the template definition context. For dependent calls, additional ADL candidates can arise from the argument types and the appropriate definition/instantiation contexts under the language rules.

Consider:

~~~cpp
template<class T>
void invoke(T& value)
{
    process(value); // dependent call
}
~~~

The compiler cannot determine every possible associated namespace until it knows `T`. That is precisely why ADL and two-phase lookup are important to generic libraries.

But this does not mean template code can defer every name lookup until instantiation. If a name is not dependent, the compiler generally resolves it at definition time. Declaring an unrelated overload later cannot be assumed to make an earlier non-dependent call resolve differently.

Compiler conformance has historically differed in template lookup, notably across older MSVC and GCC/Clang implementations. Strict modern language modes and multi-compiler tests help reveal code accidentally relying on one compiler's delayed lookup behavior.

### 10.1 Customization should be intentional

Generic code frequently wants one of two contracts. An **explicitly qualified call** says which library's operation is required. An **unqualified dependent call** can be designed to invite ADL customization.

Conflating the two leads to fragile APIs. If an operation is not intended to be customizable, qualify it. If customization is part of the design, state the required expression, semantics, and constraints, then test it with a user-defined type in a separate namespace.

C++20 concepts can test an expression's validity, but they do not prevent all ambiguous or semantically incorrect customizations. The generic library still needs a clear customization protocol.

## 11. Inline namespaces and versioned APIs

C++11 introduced **inline namespaces**, which affect the relationship between a nested namespace and its enclosing namespace for lookup and related language rules.

~~~cpp
namespace library {
    inline namespace v2 {
        int parse(const char*);
    }

    namespace v1 {
        int parse(const char*);
    }
}
~~~

Clients can ordinarily write `library::parse(...)` to find the inline version, while an explicit `library::v1::parse(...)` selects the older declaration.

The namespace `v2` still exists as a named scope, and the functions can retain ABI-distinguishing qualified identities. The inline designation makes members available through enclosing-namespace lookup without deleting the version namespace.

This can support migration when a library wants a default API version while keeping an older interface available through explicit qualification.

### 11.1 Versioning is not achieved by spelling alone

An inline namespace does not automatically solve binary compatibility. Changing which namespace is inline may change source overload candidate sets, ADL associations, mangled symbols, and which implementation a newly compiled caller references.

A client compiled against an earlier version may still refer to older symbols. If the library removes those definitions, the client may fail to link or load even though new source code compiles with the current header.

A versioned binary library therefore needs a deliberate symbol-export and compatibility policy: perhaps preserving older versioned symbols, providing explicit adapters, or requiring consumers to rebuild. Test the resulting exports with platform tools rather than assuming inline namespace syntax guarantees it.

### 11.2 Inline namespaces can affect overload resolution

Because members of an inline namespace participate in enclosing-namespace lookup, a mixture of declarations in the enclosing namespace and multiple versioned namespaces can produce ambiguities or unexpectedly broad overload sets.

Avoid using a hierarchy of inline namespaces as an informal “last declaration wins” system. C++ name lookup and overload resolution are not chronological dispatch mechanisms. Design version transitions with explicit tests for calls that accept common convertible types.

The same rule applies to using declarations. A using declaration naming a function can introduce an overload set; it does not necessarily mean only one exact signature has been imported. Versioned APIs should be reviewed at the level of candidate functions, not just namespace text.

## 12. C interfaces inside namespaces

C++ permits language-linkage specifications, including `extern "C"`, and names with C language linkage can appear in namespace contexts:

~~~cpp
namespace bridge {
    extern "C" int c_entry(int);
}
~~~

On common implementations, the C language-linkage identity is appropriate for an unmangled C ABI symbol. The namespace still affects C++ source-level lookup of the declaration.

This can seem contradictory until the layers are separated: the namespace organizes how a C++ caller refers to `bridge::c_entry`, while C language linkage determines the applicable binary linkage convention.

C linkage does not convert arbitrary C++ types to C-compatible types, and it does not make overload sets of distinct C functions with the same C symbol name possible. Corresponding declarations must have compatible language linkage and types.

### 12.1 Wrapping an existing C header

Some older C headers do not protect declarations with `__cplusplus` linkage guards. A C++ client may then use a guarded linkage block:

~~~cpp
extern "C" {
#include <legacy_c_api.h>
}
~~~

This is a compatibility technique, not a universally safe recipe. A header may already contain proper guards, C++-specific declarations, macros, or constructs that should not be placed inside another linkage specification. Prefer using the C library's officially supported C++ inclusion method.

For headers maintained by the project, a common pattern is:

~~~cpp
#ifdef __cplusplus
extern "C" {
#endif

int legacy_open(void);

#ifdef __cplusplus
}
#endif
~~~

A source-level C namespace wrapper must not be mistaken for binary ABI versioning. The exported C symbol is governed by the toolchain's C linkage rules; any plugin ABI must still define parameter sizes, ownership, calling convention, and error behavior.

## 13. Header discipline and macro leakage

Namespaces offer no protection against the C preprocessor. A macro named `open` can affect tokens that appear as `network::open` because macro expansion occurs before C++ namespace lookup.

~~~cpp
#define open something_else

// The token open may be macro-expanded even in:
network::open();
~~~

This is a major difference between namespaces and true lexical runtime packaging systems. The preprocessor does not understand namespace boundaries or C++ access control.

Avoid generic macro names in public headers, especially names likely to be class members or common functions. When interoperability requires a macro, confine it to a small compatibility layer and `#undef` it when appropriate. Prefer constants, scoped enums, inline functions, and templates for ordinary source-level facilities.

### 13.1 The C header naming convention is not a namespace rule

C++ standard library headers such as `<cstring>` and `<cmath>` provide C library facilities through the C++ library's standard namespace interface, subject to precise header-specific rules. The corresponding C-style headers such as `<string.h>` are retained for compatibility; their placement of names in the global and `std` namespaces can differ by header and implementation.

Do not infer that a header lacking a `.h` suffix is physically stored differently or loaded in some special manner. Header spellings are part of the language/library interface; physical files and implementation details are not prescribed by their suffix.

~~~cpp
#include <cstring>

std::size_t length = std::strlen("hello");
~~~

Prefer the C++ header when writing C++ code that intentionally uses the `std` namespace interface. Avoid relying on incidental global names supplied by an implementation unless the relevant standard header guarantees them.

### 13.2 A using directive in a header exports surprising lookup effects

Even when no macro is involved, a public header containing `using namespace std;` can alter lookup for every including translation unit. The effect is transitive through includes, which makes the source of an ambiguity difficult to recognize.

Qualified names may be longer, but they keep ownership visible. A few extra characters in a public header are usually a small price for a stable lookup surface.

## 14. Namespaces are not C++20 modules

Named modules, introduced in C++20, organize declaration reachability and source dependencies rather than merely names. A module and a namespace can both have dotted or nested-looking identifiers, but they are different abstractions.

~~~cpp
// Illustrative C++20 module interface
export module example.math;

export namespace example {
    int calculate(int);
}
~~~

The module name `example.math` is not a namespace expression. Importing the module makes its exported declarations reachable according to module rules; it does not issue `using namespace example`.

Two modules can contribute to the same named namespace while remaining distinct module units. Conversely, a single module can publish declarations in several namespaces.

C++20 also introduces **module linkage** for eligible entities attached to a named module. That is distinct from the internal linkage associated with unnamed namespaces and from ordinary external linkage. The module's compiled interface and build graph do not eliminate the traditional need to design binary exports if its implementation is distributed as a shared library.

### 14.1 Namespace design outlives source organization

A namespace provides source-level identity. A module supplies a reachability and compilation boundary. A source directory provides a filesystem organization. These are three different things.

A stable public namespace may be implemented across several libraries, several modules, and many directories. It should not have to change whenever an internal build target is reorganized.

For migration from headers to modules, preserve documented namespace-qualified APIs where compatibility matters, while carefully controlling which declarations are exported from module interfaces. Unqualified calls that rely on ADL may be sensitive to what declarations are reachable in the new compilation environment, so template-heavy libraries deserve explicit migration tests.

## 15. ABI design and namespace names

Names within a namespace may affect mangled C++ symbols. Changing a function from `vendor::v1::parse` to `vendor::v2::parse` can change the linker-visible name even if the parameter and return types remain the same.

A namespace alias cannot fix that binary difference because it only redirects source lookup:

~~~cpp
namespace api = vendor::v2;
~~~

If an old binary refers to a symbol in `vendor::v1`, recompiling a source alias in a newer header does not rewrite that old binary's reference.

Inline namespaces help make a current version convenient to name, but maintaining compatibility still requires exporting the old compiled symbols or supplying a separately defined bridge.

For cross-compiler and long-lived plugin interfaces, a narrow C ABI with explicitly versioned structures and function tables is often simpler than a public namespace full of C++ classes. That design may keep rich C++ namespace hierarchies inside the implementation while exposing a small stable boundary externally.

Namespace discipline and ABI discipline should complement each other: meaningful namespaces improve maintainability, and explicit export policies prevent accidental binary coupling.

## 16. C++23 and C++26 do not erase lookup rules

C++23's explicit object parameters and related generic improvements introduce new ways to express callable interfaces. C++26 static reflection introduces compile-time examination of program entities. Neither changes namespaces into runtime packages, removes ADL, or makes overload resolution a global search.

Reflection-based code generation may depend on which entities are reachable and which names are exposed under module rules. A generated wrapper must still respect source lookup, access control, ODR, linkage, and ABI constraints.

In generic libraries, the most durable rules remain: use ordinary qualification when an operation is fixed; use ADL when customization is intentionally part of the contract; avoid broad using directives; and keep namespaces meaningful independently of physical source layout.

Toolchain support for C++26 reflection and related facilities must be feature-tested. Accepting a language flag does not imply that every library and compile-time reflection interface is implemented.

## 17. Diagnostics and compiler behavior

Most namespace errors surface as ordinary compile-time name lookup failures or ambiguities. A compiler reports that a name is undeclared, qualified lookup fails, or several overload candidates are viable. The challenge is understanding which declarations entered the candidate set.

Start with warnings such as GCC/Clang `-Wall -Wextra -Wpedantic -Wshadow`, and MSVC `/W4 /permissive-`. `-Wshadow` is useful for accidental hiding in nested scopes; no single warning flag can prove that every using directive or ADL customization is well designed.

When a call becomes ambiguous, read the full candidate list and distinguish declarations found by ordinary lookup from those added by ADL. Temporarily qualify the call to test whether ADL is involved. In template code, examine where dependent names are looked up and whether the relevant declaration is reachable at the instantiation point.

For a macro-related failure, inspect the preprocessed translation unit:

~~~sh
c++ -std=c++20 -E source.cpp > source.i
~~~

MSVC provides `/E` and `/P` for corresponding preprocessing inspection. This reveals whether a namespace-qualified name was transformed before lookup.

For ABI questions, examine compiled symbols with `nm -C`, `readelf`, `objdump`, `otool`, or `dumpbin`. A namespace alias typically has no new symbol identity; a renamed or versioned namespace declaration may change mangled symbols.

AddressSanitizer and UndefinedBehaviorSanitizer do not diagnose namespace lookup errors as such. They can reveal downstream runtime defects caused by incompatible ABI assumptions or invalid objects, but the root source-level naming contract still needs static analysis.

Compiler-specific switches and vendor extensions can affect module compilation, mangling, symbol visibility, and C++ conformance. Test with strict language modes such as `-std=c++20` and target-appropriate visibility flags. Do not confuse relaxing aliasing or overflow assumptions with repairing name lookup.

### 17.1 A useful review technique

When evaluating a public namespace interface, ask whether client code might accidentally depend on a name introduced indirectly by a header, whether generic customization is documented, whether an operator is discoverable only for the intended types, and whether a namespace refactor changes ABI symbols.

These questions catch defects that ordinary type checking cannot label as mistakes because the program is still technically well-formed.

## 18. The historical timeline

**C++98/03.** Named and unnamed namespaces, namespace aliases, using declarations/directives, ADL, and namespace-aware operator overload resolution formed the core model. The language already distinguished name organization from internal/external linkage and source-level declarations from compiled symbols.

**C++11.** Inline namespaces provided a useful mechanism for versioned lookup and ABI-aware library organization. Stronger language and library facilities increased the value of maintaining deliberate namespace boundaries in generic code.

**C++14.** Namespace lookup remained fundamentally the same. Generic lambdas and other language improvements made templated call sites more common, increasing the importance of ADL and dependent lookup.

**C++17.** Nested namespace definitions, such as `namespace vendor::detail { ... }`, reduced syntactic boilerplate. The feature did not create a module boundary or privacy mechanism.

**C++20.** Named modules introduced a distinct source-reachability model and module linkage. `using enum` provided a new, local way to import enumerator names, with corresponding collision risks.

**C++23.** New callable and template capabilities continued to use existing namespace and overload lookup rules. Explicit object parameters did not turn namespaces into runtime containers.

**C++26.** Static reflection may make some compile-time declaration discovery and code-generation tasks easier, but namespace lookup, ADL, and ABI obligations remain fundamental.

## 19. From Java, Python, and C# package intuition to C++

Java packages and C# namespaces primarily organize names but exist within ecosystems with strong runtime type systems and assembly/module loading rules. Python modules are runtime objects whose import process executes code and establishes module state. It is easy to transfer the word “namespace” between these systems and accidentally transfer the wrong behavior.

In C++, a namespace does not create an object when code enters it and does not run initialization when a name is used. It does not import a library, load a binary, or enforce a dependency. Compilation units and link targets remain separate.

A namespace can be reopened across different source files and headers. Macros ignore namespace boundaries. Generic calls may find functions in namespaces associated with argument types through ADL, even when a function was not mentioned by qualification. These are major differences from the usual mental model of package names as purely hierarchical paths.

The idiomatic translation is to use namespaces for stable domain naming, headers or modules for source reachability, and build-system targets for compiled dependencies. Use explicit ABI interfaces for runtime plugins or cross-language integration.

## 20. The working model

Namespaces let a program say where a declaration belongs and how its name should participate in lookup. They do not determine ownership, storage duration, external symbol export, module loading, or runtime lifetime.

A named namespace groups related APIs and may be extended across source files. An unnamed namespace provides translation-unit-local identity. A namespace alias redirects source lookup without moving a binary definition. A using declaration introduces selected names; a using directive broadens unqualified lookup. ADL discovers functions associated with argument types and enables generic customization. Inline namespaces make one version of an API available through an enclosing namespace without erasing version identities. Modules govern reachability and compilation dependencies through different mechanisms.

The central design question is not how many namespace levels a library can tolerate. It is **which names should participate in which lookup operations, and how stable should that relationship remain for clients?**

> A namespace is a contract about finding names. Treat it as part of your library's source API, and keep its consequences for overload resolution and ABI visible.

## Further reading

The language reference chapters on [namespaces](https://en.cppreference.com/w/cpp/language/namespace), [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup), [unqualified lookup](https://en.cppreference.com/w/cpp/language/unqualified_lookup), [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl), [overload resolution](https://en.cppreference.com/w/cpp/language/overload_resolution), and [modules](https://en.cppreference.com/w/cpp/language/modules) describe the detailed rules.

The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) include recommendations for namespace design, headers, ADL-friendly interfaces, and generic customization. [Compiler Explorer](https://godbolt.org/) is useful for comparing diagnostics and symbol names across toolchains. The eight [companion CMake targets](CMakeLists.txt) provide reproducible examples of how ordinary lookup, ADL, and versioned namespaces behave in real compiled programs.
