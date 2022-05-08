Questions:

New C++14 features:
* inline namespaces (14.4.6)

Advices:
* The inline specifies that the nested namespace is the default meaning of sub-namespace
* this could be useful for the namespace versionig
* 

Book advices: 
* Use namespaces to express logical structure; 14.3.1
* Place every nonlocal name, except main(), in some namespace; 14.3.1
* Design a namespace so that you can conveniently use it without accidentally gaining access to unrelated namespaces; 14.3.3
* Avoid very short names for namespaces; 14.4.2
* If necessary, use namespace aliases to abbreviate long namespace names; 14.4.2
* Avoid placing heavy notational burdens on users of your namespaces; 14.2.2, 14.2.3
* Use separate namespaces for interfaces and implementations; 14.3.3
* Use the Namespace::member notation when defining namespace members; 14.4
* Use inline namespaces to support versioning; 14.4.6
* Use using-directives for transition, for foundational libraries (such as std), or within a local scope; 14.4.9
* Don't put a using-directive in a header file; 14.2.3