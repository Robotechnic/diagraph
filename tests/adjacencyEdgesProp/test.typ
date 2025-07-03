/// [ppi:100]

#import "../../lib.typ": adjacency
#set page(width: auto, height: auto, margin: 1cm)

#adjacency(
  (
    (none, (headlabel: "B", color: "red", style: "dashed"), "C"),
    ("D", none, (headlabel: "F", color: "blue", style: "dashed")),
    ((headlabel: "G", color: "green", style: "dashed"), "H", none),
  ),
  rankdir: "LR",
)
