created after commit 25a0d1d

Path to Success:
- refactor the parsing to first make a tree of all the enclosures then the AST
- implement structures in InterLang:
    - add way to create structures in InterLang (may need to rename `DEF` to `DEFFUNC`)
    - add ways to interact with those structures
    - implem,ent in `interLang.c`
- support "if","while":
    - add branching in InterLang
- add non linear asm generation in `InterLangCompiler.py`
- add sections in InterLang
- add better asm register allocation in `InterLangCompiler.py`
- support modifier syntax
- (possibly) add operator overloading
- change all the `InterLangCompiler.py` infrastructure to be able to genereate a object file 
- change all the `cCompiler.c` infrastructure to raise errors instead of assuming perfect code


