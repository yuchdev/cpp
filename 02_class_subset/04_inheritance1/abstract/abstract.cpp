#include "abstract.h"
#include <iostream>

using std::cout;
using std::endl;

void circle::draw(){ cout << "circle::draw" << endl; }
void circle::rotate(point& p){ cout << "circle::rotate" << endl; }

void rectangle::draw(){ cout << "rectangle::draw" << endl; }
void rectangle::rotate(point& p){ cout << "rectangle::rotate" << endl; }

void sphere::draw(){ cout << "sphere::draw" << endl; }
void sphere::rotate(line& px, line& py, line& pz){ cout << "sphere::rotate" << endl; }

void cube::draw(){ cout << "cube::draw()" << endl; }
void cube::rotate(line& px, line& py, line& pz){ cout << "cube::rotate" << endl; }

