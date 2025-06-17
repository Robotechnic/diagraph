/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(
  ```
  	digraph G {
  		rankdir=LR
  		A -> B
  		B -> A
  	}
  ```,
  edges: (
    "A": ("B": "Edge A->B"),
    "B": ("A": "Edge B->A"),
  ),
)
