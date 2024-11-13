#import "@preview/mantys:0.1.4": *
#import "@preview/diagraph:0.4.0"

#show: mantys.with(
  ..toml("../typst.toml"),
  examples-scope: dictionary(diagraph),
)

= Drawing graphs

To draw a graph you have two options: #cmd[raw-render] and #cmd[render]. #cmd[raw-render] is a wrapper around the #cmd[render] and allow to use raw block as input.

#command(
  "render",
  arg("dot"),
  arg("labels", (:)),
  arg("xlabels", (:)),
  arg("edges", (:)),
  arg("clusters", (:)),
  arg("engine", "dot"),
  arg("width", auto),
  arg("height", auto),
  arg("clip", true),
  arg("debug", true),
  arg("background", none),
)[
  #argument("dot", types: ("string"))[
    The dot code to render.
  ]
  #argument("labels", types: ((:)))[
    Nodes labels to overwrite. The dictionary is indexed by the node name.
    #example(````
    	#raw-render(```
    		digraph G {
    			rankdir=LR
    			A -> B
    			B -> A
    		}
    	```,
    	labels: ("A": "Node A", "B": "Node B")
    	)
    ````)
  ]
  \
  \
  \
  \
  \
  #argument("xlabels", types: ((:)))[
    Nodes labels to overwrite. The dictionary is indexed by the node name.
    #example(````
    	#raw-render(```
    		digraph G {
    			rankdir=LR
    			A -> B
    			B -> A
    		}
    	```,
    	xlabels: ("A": "Node A", "B": "Node B")
    	)
    ````)
  ]
  #argument("edges", types: ((:)))[
    Edges labels to overwrite. The dictionary keys are source nodes. The values are dictionaries indexed by the target node. Each edge is one dictionary or multiple ones designating multiple edges.

    Valid keys are:
    - `label`: The label of the edge.
    - `xlabel`: The xlabel of the edge.
    - `taillabel`: The taillabel of the edge.
    - `headlabel`: The headlabel of the edge.

    Instead of a dictionary, you can only specify a content value. In this case, the content is used as the label of the edge.

    #example(````
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
    ````)
  ]
  #argument("clusters", types: ((:)))[
    Clusters labels to overwrite. The dictionary is indexed by the cluster name.
    #example(````
    	#raw-render(```
    		digraph G {
    			rankdir=LR
    			subgraph cluster_0 {
    				A -> B
    			}
    		}
    	```,
    	clusters: ("cluster_0": "Cluster 0")
    	)
    ````)
  ]
  #argument("engine", types: ("string"))[
    The engine to use to render the graph. The currently supported engines are:
    #list(..diagraph.engine-list().engines)
  ]
  #argument("width", types: (1em, 1%))[
    The width of the rendered image.
  ]
  #argument("height", types: (1em, 1%))[
    The height of the rendered image.
  ]
  \
  #argument("clip", types: (true))[
    Whether to hide part of the graphs that goes outside the bounding box given by graphviz.
  ]
  #argument("debug", types: (true))[
    Show debug boxes around the labels.
  ]
  #argument("background", types: (none, "color"))[
    The background color of the rendered image. `none` means transparent.
  ]
]

= Fonts, colors and sizes
If you don't overwrite the labels, the specified graphviz font, color and size will be used. However, the fonts are not necessarily the same as the graphviz fonts. If you want to use a font, it must be accessible by Typst. For colors, you can us any valid graphviz color you want as they are converted to Typst colors automatically. Font size works as usual. If you overwrite a label, the font, color and size will be the ones used by Typst where the graph is rendered.

#example(````
	#raw-render(```
		digraph G {
			rankdir=LR
			A1 -> B1
			A1[color=orange, fontname="Impact", fontsize=20, fontcolor=blue]
			B1[color=red, fontname="Fira Sans", fontsize=20, fontcolor=green]
		}
	```)
````)

The same goes for edges and clusters.

#example(````
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
````)

#example(````
	#raw-render(```
		digraph G {
			rankdir=LR
			edge [color=red, fontname="Impact", fontsize=20, fontcolor=blue]
			A -> B[label="one"]
			A -> C[label="two"]
			B -> C[label="three"]
		}
	```)
````)

= Automatic math mode detection
Diagraph tries to automatically detect simple math expressions. Single letters, numbers, and greek letters are automatically put in math mode. 

#example(````
	#raw-render(```
		digraph {
			a -> alpha
			phi -> rho
			rho -> a
			tau -> omega
			phi -> a_8
			a_8 -> alpha
			a_8 -> omega
			alpha_8 -> omega
		}
	```)		
````)

= Examples
Those examples are here to demonstrate the capabilities of diagraph. For more information about the Graphviz Dot language, you can check the #underline(link("https://graphviz.org/documentation/", "official documentation")). Some of the following examples are taken from the #underline(link("https://graphviz.org/gallery/", "graphviz gallery")).

#example(````
	#raw-render(
		```
		digraph {
			rankdir=LR
			node[shape=circle]
			Hmm -> a_0
			Hmm -> big
			a_0 -> "a'" -> big [style="dashed"]
			big -> sum
		}
		```,
		labels: (
			big: [_some_#text(2em)[ big ]*text*],
			sum: $ sum_(i=0)^n 1 / i $,
		),
		width: 100%,
	)````
)

#example(````
 #raw-render(```
digraph finite_state_machine {
   rankdir=LR
   size="8,5"

   node [shape=doublecircle]
   LR_0
   LR_3
   LR_4
   LR_8

   node [shape=circle]
   LR_0 -> LR_2 [label="SS(B)"]
   LR_0 -> LR_1 [label="SS(S)"]
   LR_1 -> LR_3 [label="S($end)"]
   LR_2 -> LR_6 [label="SS(b)"]
   LR_2 -> LR_5 [label="SS(a)"]
   LR_2 -> LR_4 [label="S(A)"]
   LR_5 -> LR_7 [label="S(b)"]
   LR_5 -> LR_5 [label="S(a)"]
   LR_6 -> LR_6 [label="S(b)"]
   LR_6 -> LR_5 [label="S(a)"]
   LR_7 -> LR_8 [label="S(b)"]
   LR_7 -> LR_5 [label="S(a)"]
   LR_8 -> LR_6 [label="S(b)"]
   LR_8 -> LR_5 [label="S(a)"]
 }
 ```,
 labels: (
   "LR_0": $"LR"_0$,
   "LR_1": $"LR"_1$,
   "LR_2": $"LR"_2$,
   "LR_3": $"LR"_3$,
   "LR_4": $"LR"_4$,
   "LR_5": $"LR"_5$,
   "LR_6": $"LR"_6$,
   "LR_7": $"LR"_7$,
   "LR_8": $"LR"_8$,
 ),
 edges:(
	"LR_0": ("LR_2": $S S(B)$, "LR_1": $S S(S)$),
	"LR_1": ("LR_3": $S(dollar"end")$),
	"LR_2": ("LR_6": $S S(b)$, "LR_5": $S S(a)$),
 ),
 width: 100%,
)
````)