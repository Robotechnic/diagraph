#import "@preview/diagraph:0.2.5": *

#let labels = (
	"a" :$1$,
	"b" :$2$,
	"c" :$3$,
	"d" :$4$,
	"e" :$5$,
	"f" :$9$,
	"g" :$10$,
	"h" :$11$,
	"i" :$12$,
	"j" :$13$,
	"k" :$14$,
	"l" :$15$,
	"m" :$19$,
	"n": $20$,
	"p" : $50$
)

#let graph = (nodes) => {
	let result = "digraph {"
	for (i,n) in nodes.keys().enumerate() {
		for m in nodes.keys().slice(i) {
			result += n.at(0) + " -> " + m.at(0) + ";"
		}
	}
	render(result + "}", labels: nodes, width: 100%)
}

#for i in range(2, labels.len()) {
	for j in range(0, labels.len() - i) {
		graph(labels.keys().slice(j, j + i).fold(("z":"2"), (acc, e) => {
			acc.insert(e, labels.at(e));
			acc
		}))
	}
}
