/// [ppi:100]

#import "../../lib.typ": adjacency
#set page(width: auto, height: auto, margin: 1cm)

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