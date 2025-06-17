/// [ppi:100]

#import "../../lib.typ": render
#set page(width: auto, height: auto, margin: 1cm)

#let colors = (
  "black",
  "silver",
  "white",
  "navy",
  "blue",
  "aqua",
  "teal",
  "eastern",
  "purple",
  "fuchsia",
  "maroon",
  "red",
  "orange",
  "yellow",
  "olive",
  "green",
  "lime",
)

#for color in colors {
	render("digraph {A[fontcolor="+color+"]}")
}