/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```
digraph {
	orientation=landscape
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)

#raw-render(```
digraph {
	orientation=Landscape
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)

#raw-render(```
digraph {
	rotate = 90
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)

#raw-render(```
digraph {
	landscape = true
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)

#raw-render(```
digraph {
	orientation=landscape
  rotate = 1
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)


#raw-render(```
digraph {
	orientation=landscape
  rotate = 0
  landscape = true
  node [shape = box]
  A -> B [label="30"]
  D -> B [label="45"]
  A -> D [label="30"]
  B -> E [label="40"]
  C -> E [label="90"]
  A -> F [label="30"]
  E -> G [label="45"]
  F -> G [label="120"]
  D -> H [label="45"]
  F -> H [label="120"]
  H -> I [label="20"]
  H -> J [label="20"]
  G -> K [label="40"]
  I -> K [label="40"]
  J -> L [label="120"]
  K -> L [label="100"]
}
```)
