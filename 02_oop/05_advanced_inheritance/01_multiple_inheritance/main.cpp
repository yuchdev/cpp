#include "simple.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace
{
    template <typename F>
    std::string captured_output(F&& f)
    {
        std::ostringstream out;
        std::streambuf* old = std::cout.rdbuf(out.rdbuf());
        f();
        std::cout.rdbuf(old);
        return out.str();
    }
}

// window_draw's base-class construction order follows the order bases are
// listed in `: public button_t, public slider_t` -- button_t (and its own
// control_t) constructs first, then slider_t (and ITS OWN, separate,
// control_t). Captured and asserted, not just left for a human to read in
// the printed order.
void show_construction_order()
{
    // wd is constructed AND destroyed within this lambda's scope, so the
    // captured log includes both: construction in base-list order
    // (button_t, and its own control_t, first; then slider_t and ITS
    // own control_t), then destruction in exact reverse order. Neither
    // button_t nor slider_t has its own destructor, so each inherits
    // control_t's unchanged -- hence two separate "~control_t()" lines,
    // one per sub-object, not one shared call.
    std::string ctor_log = captured_output([] { window_draw wd; });
    assert(ctor_log ==
        "control_t()\nbutton_t()\ncontrol_t()\nslider_t()\nwindow_draw()\n"
        "~control_t()\n~control_t()\n");

    window_draw wd;
    assert(captured_output([&] { wd.push(); }) == "button_t::push\n");
    assert(captured_output([&] { wd.move(5); }) == "slider_t::move\n");
}

// window_draw contains two independent control_t sub-objects, so an
// unqualified wd.draw() cannot compile -- the call is genuinely ambiguous,
// not just a style complaint. Qualifying by the path removes the
// ambiguity; this is exactly the `using-directive`/explicit-qualification
// resolution the course material calls for.
void show_ambiguity_resolution()
{
    window_draw wd;

#if 0
    // Ambiguous: which control_t's draw() is this? Does not compile.
    wd.draw();
#endif

    // button_t overrides draw(); slider_t does not, so wd.slider_t::draw()
    // reaches control_t::draw() unchanged through slider_t's own path --
    // a DIFFERENT function from the one wd.button_t::draw() reaches, even
    // though both paths ultimately lead back to "a control_t".
    assert(captured_output([&] { wd.button_t::draw(); }) == "button_t::draw\n");
    assert(captured_output([&] { wd.slider_t::draw(); }) == "control_t::draw\n");
}

// Repeating a non-virtual base costs real memory: window_draw pays for
// control_t's data twice, once per inheritance path.
void show_sizes()
{
    std::cout << "sizeof(control_t)   = " << sizeof(control_t) << '\n';
    std::cout << "sizeof(button_t)    = " << sizeof(button_t) << '\n';
    std::cout << "sizeof(slider_t)    = " << sizeof(slider_t) << '\n';
    std::cout << "sizeof(window_draw) = " << sizeof(window_draw) << '\n';

    // button_t and slider_t each contain their OWN control_t, so neither
    // is smaller than control_t; window_draw contains both of them in
    // full, with no data of its own, so its size is exactly their sum --
    // the "two full copies" claim, proven rather than just illustrated.
    assert(sizeof(button_t) > sizeof(control_t));
    assert(sizeof(slider_t) > sizeof(control_t));
    assert(sizeof(window_draw) == sizeof(button_t) + sizeof(slider_t));
}

int main()
{
    show_construction_order();
    show_ambiguity_resolution();
    show_sizes();
    return 0;
}
