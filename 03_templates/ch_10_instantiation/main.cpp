#include "by_request.h"


void show_forward_decl()
{

    // Structure where pointer to a forward declaration is defined
    TemplatePreuse p;
}

void show_by_request()
{
    // There are a lot of declared but undefined members in this template
    // And it is being compiles, because they are not called and instantiated
    InstantByRequest<int, 0> thats_ok;
}

int main()
{

    show_forward_decl();
    show_by_request();

    return 0;
}
