#include "simple.h"

#include <iostream>

using std::cout;
using std::endl;

// window_draw's base-class construction order follows the order bases are
// listed in `: public button_t, public slider_t` -- button_t (and its own
// control_t) constructs first, then slider_t (and ITS OWN, separate,
// control_t). Watch the printed order to see control_t() run twice.
void show_construction_order()
{
    window_draw wd;
    wd.push();
    wd.move(5);
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

    wd.button_t::draw();
    wd.slider_t::draw();
}

// Repeating a non-virtual base costs real memory: window_draw pays for
// control_t's data twice, once per inheritance path.
void show_sizes()
{
    cout << "sizeof(control_t)   = " << sizeof(control_t) << '\n';
    cout << "sizeof(button_t)    = " << sizeof(button_t) << '\n';
    cout << "sizeof(slider_t)    = " << sizeof(slider_t) << '\n';
    cout << "sizeof(window_draw) = " << sizeof(window_draw) << '\n';
}

int main()
{
    show_construction_order();
    show_ambiguity_resolution();
    show_sizes();
    return 0;
}
