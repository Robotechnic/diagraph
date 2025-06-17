/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```
	graph {
		abc -- def
		def -- ghi
		jkl -- mno
		subgraph cluster_0 {
			ghi -- jkl
			def -- pqr
		}
		abc -- vwx
		pqr -- stu
		subgraph cluster_1 {
			vwx -- yz
		}
	}
```, 
labels: (name) => {
	text(fill: red, name.rev())
},
xlabels: (name) => {
	if name != "abc" {
		text(fill: blue, size: 2em, font: "FreeMono", name)
	}
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
}
)