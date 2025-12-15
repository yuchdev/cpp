## Decrypting debug messages and tracers.

* Debug messages for templates are quite hard to read and require careful decoding
* To improve such situations, a "shallow instantiation" model is used - the top level checks the template parameter for some requirements, like pointer dereferencing
* You can use intelligent tracers to check the validity of template function parameters and their algorithms
