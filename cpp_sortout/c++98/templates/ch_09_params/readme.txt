Chapter 9 Nicolai Josuttis

Templates naming and syntax analysis

* In order to use a pointer or reference to a template, as well as for a class, 
  it is not necessary to define it completely - forward declaration would be enough
* Only used class members are instantiated, except for virtual functions and unnamed joins.
  Therefore, if there any errors in them, we won't know until they are explicitly instantiated.
* Compilers implement different approaches to instantiation
* The instantiation point is the place in the binary code where the generated template body is embedded
  For function templates, this is the furthest place in the translation unit
  For class templates, the closest place in the scope of the call
* The name of the overridden template parameter is inherited from the base class
