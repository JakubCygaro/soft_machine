- custom parameters editing
- full editability of LISP styling
- full editability for all components: MEM - CPU needs an instruction parser for that to work
- editor: graph pausing, stepping and cycle rate controls
- immediate mode drawing of components:
```cpp
draw::BeginBody(m_bounds);
// various draw calls
draw::EndBody();
```
- add check to Style if the dependency graph has to be rebuild
- change drawing code to be dependent upon style
- fix a bug with CPU changing size
- styling via LISP, possibly with topo-sort before styling execution
- add logic gates that treat integer messages as either ON (non-zero) and OFF (zero)
- some sort of testing framework?
