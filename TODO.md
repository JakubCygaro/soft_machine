- editor: paused overlay indicator or something that tells the user that the graph is paused
- highlight element name on hover
- immediate mode drawing of components:
```cpp
draw::BeginBody(m_bounds);
// various draw calls
draw::EndBody();
```
- change drawing code to be dependent upon style
- fix a bug with CPU changing size
- expand CPU instruction set and maybe rework the way its implemented (dynamic casts)
- add logic gates that treat integer messages as either ON (non-zero) and OFF (zero)
- some sort of testing framework?
