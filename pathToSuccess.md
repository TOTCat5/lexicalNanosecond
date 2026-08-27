created after commit 25a0d1d

Path to Success:
- ~~refactor the parsing to first make a tree of all the enclosures then the AST~~
- ~~support function calling~~
- implement structures in InterLang:
    - add better support to non linear asm generation in `InterLangCompiler.py`
    - add way to create structures in InterLang (may need to rename `DEF` to `DEFFUNC`)
    - add ways to interact with those structures
    - implement in `interLang.c`
- support "if","while":
    - add branching in InterLang
- add floating point arithmetic
- add sections in InterLang
- add better asm register allocation in `InterLangCompiler.py`
- support modifier syntax
- add builtins in InterLang (ex:getAddr)
- support calling pointer math or bullshit of that sort
- support multifile code
- change all the `InterLangCompiler.py` infrastructure to be able to generate a object file 
- (possibly) add operator overloading
- change all the `cCompiler.c` infrastructure to raise errors instead of assuming perfect code


