## Chapters Content

* Complex template implementation details
* Instantiating of templates
* Instantiation on demand, delayed instantiation
* Two-phase retrieval
* Templates implementation details in compilers
* Explicit instantiation

## Templates Instantiation
* Basic instantiation strategies used in compilers:
  * Greedy instantiation - multiple instances of the same template can be instantiated
  * The assembly code may slightly differ
  * Pros: ease of compilation
  * Cons: speed of compilation, duplication of code
* Instantiation on demand. The compiler keeps a table of instantiations in a file, and searches for a pre-built implementation when a template needs to be instantiated
  * If there's already an implementation, but the code has changed since the last compilation, it is updated
  * If there's none, the implementation is added
  * Pros: compact implementation
  * Cons: difficulties in the linker implementation
* Iterative instantiation. Using this approach we compile the source code and try linking, then we analyze the linker errors and iteration is repeated
  * Pros: compactness of the generated code
  * Cons: linking time
