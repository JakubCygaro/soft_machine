- fix broadcast
- refactor xml graph building to be generic over the builder
- add styling, Object would need to take in a Styling shared pointer that provides
 information on styling details such as the font size and stuff
- proper incident edge removal on element removal, m_incident needs to be updated
- clock component
- [facelift] file saving, selection and loading at runtime
- styling via LISP, possibly with topo-sort before styling execution
<element name="element">
    <style>
        (set!-position-rel "element2" 'right 100)
    </style>
</element>
