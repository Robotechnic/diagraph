/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```dot
  graph {
    node[shape=point];
    a -- b -- c -- d;
    b -- d -- a -- a;
    d[shape=point]
  }
 ```)
