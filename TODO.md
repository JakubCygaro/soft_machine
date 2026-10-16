- change drawing code to be dependent upon style
- fix a bug with passthrough connection drawing being bugged on startup
- fix a bug with CPU changing size and not highlighting all instructions for some
 reason
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
