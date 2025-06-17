/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```
	digraph G {
		rankdir=LR
		A1 -> B1
		A1[color=orange, fontname="Libertinus Serif", fontsize=20, fontcolor=blue]
		B1[color=red, fontname="New Computer Modern Math", fontsize=20, fontcolor=green]
	}
```)

#raw-render(```
	digraph G {
		rankdir=LR
		subgraph cluster_0 {
			label="Custom label"
			fontname="DejaVu Sans Mono"
			fontcolor=purple
			fontsize=25
			A -> B
		}
	}
```)

#raw-render(```
	digraph G {
		rankdir=LR
		edge [color=red, fontname="New Computer Modern", fontsize=20, fontcolor=blue]
		A -> B[label="one"]
		A -> C[label="two"]
		B -> C[label="three"]
	}
```)
