- LISP: set!-font-size, get-font-size, set!-font, set!-param, get-param, set!-body-color, get-body-color
    set!-param and get-param would do a lookup in the custom parameter list of the Style object
- improve LISP error sink (longer history)
- full editability of LISP styling
- full editability for all components - CPU needs an instruction parser for that to work
- immediate mode drawing of components:
- editor: graph pausing, stepping and cycle rate controls
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
```xml
<element name="element">
    <style>
        <lisp>
            (set!-position-rel "element2" 'right 100)
            (set!-font "default")
            (set!-font-size 24)
            (set!-param "body-color" 'GREEN)
        </lisp>
    </style>
</element>
```
