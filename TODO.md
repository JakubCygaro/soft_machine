- add styling, Object would need to take in a Styling shared pointer that provides
 information on styling details such as the font size and stuff.
 there probably needs to be an external StylingManager or whatever that keeps track
 of what styling is dependent on what other styling and in case something changes
 it will notify all dependent components - basically a styling graph
- [facelift] file saving, selection and loading at runtime
- styling via LISP, possibly with topo-sort before styling execution
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
?- refactor xml graph building to be generic over the builder
