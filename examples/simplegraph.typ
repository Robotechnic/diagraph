#import "@preview/typst-graphviz:0.1.0": *


```dotrender
digraph {
	A -> B
	B -> C
	C -> D
}
```

#render("digraph{A->B\nB->C\nC->D}")