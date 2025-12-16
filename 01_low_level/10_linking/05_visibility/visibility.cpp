#include "visibility.h"
int exported_add(int a, int b) { return a + b; }
int hidden_add(int a, int b) { return a + b + 1000; }
