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
