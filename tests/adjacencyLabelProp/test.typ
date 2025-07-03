/// [ppi:100]

#import "../../lib.typ": adjacency
#set page(width: auto, height: auto, margin: 1cm)

#adjacency(
  (
    (none, "B", "C"),
    ("D", none, "F"),
    ("G", "H", none),
  ),
  rankdir: "LR",
  vertex-labels: (
    "Node A",
    "Node B",
    (label: "Node C", color: red, shape: "doublecircle"),
  ),
)
