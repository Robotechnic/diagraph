#import "@preview/mantys:1.0.1": *
#import "@preview/diagraph:0.3.5"

#show: mantys(
  ..toml("../typst.toml"),
  examples-scope: (
		scope: (
			diagraph: diagraph,
		),
		imports: (
			diagraph: "*",
		)
	)
)

#set page(margin: (bottom: auto))

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
  arg("debug", false),
  arg("background", none),
	arg("stretch", true),
	arg("math-mode", auto)
)[
  #argument("dot", types: ("string"))[
    The dot code to render.
  ]
  #argument("labels", types: ((:), () => {}))[
    Nodes labels to overwrite.
		
		If the provided argument is a dictionary, the keys are the node names and the values are the labels. If the provided argument is a function, the function is called with the node name as argument and must return the label. A none value means that the label is not overwritten.
		
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
	
  #argument("xlabels", types: ((:), () => {}))[
    Nodes xlabels to overwrite.

		If the provided argument is a dictionary, the keys are the node names and the values are the xlabels. If the provided argument is a function, the function is called with the node name as argument and must return the xlabel. A none value means that the xlabel is not overwritten.
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
  #argument("edges", types: ((:), () => {}))[
    Edges labels to overwrite. 
		
		If the parameter is a dictionary, the dictionary keys are source nodes. The values are dictionaries indexed by the target node. Each edge is one dictionary of valid keys or a list designating multiple edges between the same nodes.

		If the parameter is a function, the function is called with two parameters:
		- The source node
		- A list of the nodes with an edge from the source node to them
		It must return a dictionary with the target nodes as keys and a dictionary or list of dictionary as values.

    Valid keys for the edges dictionary are:
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
    Clusters labels to overwrite. 

		If the provided argument is a dictionary, the keys are the cluster names and the values are the labels. If the provided argument is a function, the function is called with the cluster name as argument and must return the label. A none value means that the label is not overwritten.
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
	\
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
  #argument("clip", types: (true))[
    Whether to hide part of the graphs that goes outside the bounding box given by graphviz.
  ]
  #argument("debug", types: (true))[
    Show debug boxes around the labels.
  ]
  #argument("background", types: (none, "color"))[
    The background color of the rendered image. `none` means transparent.
  ]
	#argument("stretch", types: (true,))[
		By default, if both with and height are set, the graph will be stretched to fit in the bounding box. If you want to keep the aspect ratio, you can set this parameter to false. It then behaves like the css property `object-fit: contain`.
	]
	#argument("math-mode", types: (auto, "math", "text"))[
		The math mode to use for the labels. Can be `auto`, `"math"` or `"text"`. If set to `auto`, the mode will be determined by label content. In `"text"` mode, the label content will be parsed as a string. In `"math"` mode, the label content will be parsed as a math expression. This parameter acts globally on the graph. You can also set the math mode more precisely for each node, edge and cluster using the appropriate dot attribute (See @math-mode)

		#example(````
			#raw-render(```
				digraph G {
					rankdir=LR
					alpha -> beta
					beta -> "sum_(i=0)^n 1 / i"
				}
			```,
			math-mode: auto
			)
		````)
		#example(````
			#raw-render(```
				digraph G {
					rankdir=LR
					alpha -> beta
					beta -> "sum_(i=0)^n 1 / i"
				}
			```,
			math-mode: "math"
			)
		````)
		#example(````
			#raw-render(```
				digraph G {
					rankdir=LR
					alpha -> beta
					beta -> "sum_(i=0)^n 1 / i"
				}
			```,
			math-mode: "text"
			)
		````)
	]
]

= Math mode<math-mode>

This module uses a custom graphviz parameter to control how the math mode is managed by the renderer. By default, a simple algorithm convert automatically all text to math mode. You can control this behavior for each node, edge and cluster by using a parameter to enable full math mode or disable it completely.

#table(
	columns: (1fr, 1fr, 1fr),
	stroke: none,
	table.hline(),
	table.header("Object", "Label", "Math mode"),
	table.hline(),
	table.cell(rowspan: 2, "Node"), "label", "math",
	"xlabel", "xmath",
	table.hline(),
	table.cell(rowspan: 4, "Edge"), "label", "lmath",
	"xlabel", "lxmath",
	"headlabel", "hmath",
	"taillabel", "tmath",
	table.hline(),
	table.cell(rowspan: 1, "Cluster"), "label", "math",
	table.hline()
)

== Default behavior

By default, the algorithm detect the patterns that are likely to be math expressions. The following patterns are detected:
- Single letters
- Numbers
- Greek letters
- All of the above separated by a non letter or number character: an underscore, a space, a parenthesis, etc.

#example(side-by-side: true, ````
	#raw-render(```
		graph {
			alpha
			a_1
			"a^3_5"
			"a + b"
			"f(5)"
			"5 beta_gamma'"
		}
	```, engine: "osage")
````)

#pagebreak()

== No math mode

This mode diable the math mode for the node, edge or cluster. The text is rendered as is.

#example(side-by-side: true, ````
	#raw-render(```
		digraph G {
			rankdir=LR
			node[math=false]
			alpha
			a_1
			"a^3_5"
			"a + b"
			"f(5)"
			"beta_gamma"
		}
	```, engine: "osage")
````)

== Full math mode
In this mode, all the text is considered as a typst math expression. So, any invalid math expression will cause an error.

#example(````
#raw-render(```
	digraph {
		node[math=true]
		edge[lmath=true]
		s[label="sum_(n=0)^3 n"]
		s -> s1
		s -> s2
		s -> s3
		s -> s4
		{rank=same
		s1[label="0"]
		s2[label="1"]
		s3[label="2"]
		s4[label="3"]
		s1 -> s2[label="0 + 1"]
		s2 -> s3[label="1 + 2"]
		s3 -> s4[label="3 + 3"]
		}	
		s4[xlabel="sum_(n=0)^3 n = 6"]
	}
```)
````)

= Adjacency list
If you don't want to write the graph in dot format, you can use the adjacency list format. This format allow you to pass a matrix of edges values that represent the graph. 

#command("adjacency",
	arg("matrix"),
	arg("directed", true),
	arg("vertex-labels", ()),
	arg("clusters", ()),
	arg("debug", false),
	arg("clip", true),
	sarg("dot arguments")
)[
	#argument("matrix", types: (array))[
		The matrix of edges. The matrix must be a list of lists. Each list represents a row of the matrix. The values of the matrix are the labels of the edges. If the value is none, there is no edge between the nodes.

		#example(````
			#adjacency(
				(
					(none, "B", "C"),
					("D", none, "F"),
					("G", "H", none),
				),
				rankdir: "LR"
			)
		````)

		If the value is a dictionary, then the keys are the edge labels and attributes. The labels are the usual graphviz labels (i.e `label`, `xlabel`, `headlabel`, `taillabel`).

		#example(```
			#adjacency(
				(
					(none, (headlabel: "B", color: "red", style: "dashed"), "C"),
					("D", none, (headlabel: "F", color: "blue", style: "dashed")),
					((headlabel: "G", color: "green", style: "dashed"), "H", none),
				),
				rankdir: "LR",
			)
		```)
	]
	#argument("directed", types: (true))[
		Whether the graph is directed or not.
		#example(````
			#adjacency(
				(
					(none, "A"),
					("B", none),
				),
				rankdir: "LR",
				directed: false
			)
		````)
	]
	#argument("vertex-labels", types: ((:),))[
		List of labels. If the value is a content, the content is used as the label of the vertex. If the value is a dictionary then the keys are the nodes attributes.
		
		#example(````
			#adjacency(
				(
					(none, "B", "C"),
					("D", none, "F"),
					("G", "H", none),
				),
				rankdir: "LR",
				vertex-labels: (
					"Node A", "Node B", 
					(label: "Node C", color: red, shape: "doublecircle"),
				)
			)
		````)
	]
	#argument("clusters", types: (list(),))[
		List of clusters. If the value is a list, it is used as the list of nodes in the cluster. If the value is a dictionary, the mandatory key is `nodes` which is the list of nodes in the cluster. The other keys are the cluster attributes.
	]
	#argument("debug", types: (true))[
		Show debug boxes around the labels.
	]
	#argument("clip", types: (true))[
		Whether to hide part of the graphs that goes outside the bounding box given by graphviz.
	]
	#argument("dot arguments")[
		Graph arguments. There are two types of values:
		- A key -- dot value pair: The key is the attribute name and the value is the attribute value.
		
		For example, `rankdir: "LR"` will be converted to `rankdir=LR` in dot.
		
		- A key -- dictionary pair: The key is the element name and the value is a dictionary of attributes. The attributes must be all key -- dot value pairs.

		For example, `node: (shape: "circle")` will be converted to `node[shape=circle]` in dot. 
	]

	#example(````
	#adjacency(
		vertex-labels: ($alpha$, $beta$, $gamma$),
		(
			([1], move(dx: 25pt, dy: -5pt)[lala], none),
			($4x$, none, none),
			(none, 1, 13 / 4),
		),
		layout: "neato",
		rankdir: "LR",
		edge: (
			arrowsize: 0.7,
		),
		node: (
			shape: "circle",
			width: 0.3,
			fixedsize: true,
		),
	)
	````)
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
 height: 10em
)
````)

#example(````
#import "@preview/diagraph:0.3.3": *

#raw-render(```
	graph {
		abc -- def
		def -- ghi
		jkl -- mno
		abc -- jkl
		subgraph cluster_0 {
			ghi
			def -- pqr
		}
		abc -- vwx
		abc -- stu
		subgraph cluster_1 {
			vwx -- yz
		}
	}
```, 
height: 15em,
labels: (name) => {
	text(fill: red, name.rev())
},
xlabels: (name) => {
	text(fill: blue, size: 2em, font: "FreeMono", name)
},
clusters: (name) => {
		text(fill: orange, size: 3em, name.at(-1))
},
edges: (name, edges) => {
	let labels = (:)
	for edge in edges {
		labels.insert(edge, (
			label: text(fill: green, [#name -- #edge]),
			headlabel: name.at(0),
			taillabel: edge.at(0)
		))
	}
	labels
},
)
````)
