Chapter 7 Nicolai Josuttis

Template organization models
Inclusion model, explicit instantiation model

*  In general, it's impossible to put the template definition in *.cpp file, 
   as you don't know yet what kind of template type you want to instantiate until the compilation stage
* "Inclusion model", which is defining a template class and methods in the header, the most common practice
  This creates the problem of increasing the headers size 
  (because all the standard headers also have to be included in the header with the template)
* The explicit instantiation model is to explicitly instantiate all the types of templates you might need in a *.cpp file.
  The disadvantage of this method is that you have to explicitly monitor all types being instantiated 
  (which is not recommended in large projects)
