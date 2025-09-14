/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```
  digraph{
    e
    edge [arrowsize=0.6]
    splines=false
    nodesep=0.5
    node [shape=circle,fixedsize=true]
    edge [penwidth=0.6]
    graph [
      style=dashed
      penwidth=0.2
      margin=9
      labeljust=l ]
      
    {
      rank=same
      i1 [label= "", shape=none,height=.0,width=.0]
      i2 [label= "", shape=none,height=.0,width=.0]
    }
    {
      rank=same
      i3 [label= "", shape=none,height=.0,width=.0]
      i4 [label= "", shape=none,height=.0,width=.0]
    }

    i1 -> a1 [color=red,penwidth=1.5]
    i2 -> a3
    
    subgraph cluster_1{ 
      label=V_1
      {
      rank=same
      a1->a2->a3 [color=red,penwidth=1.5]
      a1->a2->a3 [dir=back]
    }
  }

    subgraph cluster_2{ 
      label=V_2
      {
        rank=same
        b1->b2->b3 [color=red,penwidth=1.5]
        b1->b2->b3 [dir=back]
      }
    }
    
    a1 -> b1
    a1:se -> b3:nw
    a3:sw -> b1:ne [color=red,penwidth=1.5]
    a3 -> b3

    b1 -> c1
    b1:se -> c3:nw
    b3:sw -> c1:ne
    b3 -> c3 [color=red,penwidth=1.5]

    subgraph cluster_3{ 
      label=V_3   
      {
      rank=same
      c1->c2->c3
      c1->c2->c3 [dir=back] [color=red,penwidth=1.5]
    }}
    
    c1 ->d1 [color=red,penwidth=1.5]
    c1:se ->d3:nw
    c3:sw->d1:ne
    c3->d3

    subgraph cluster_4{ 
      label=V_4
      {
      rank=same
      d1->d2->d3 [color=red,penwidth=1.5]
      d1->d2->d3 [dir=back]
    }}
    d1 ->e1
    d1:se ->e3:nw
    d3:sw->e1:ne
    d3->e3 [color=red,penwidth=1.5]

    subgraph cluster_5{ 
      label=V_5
      {
      rank=same
      e1->e2->e3
      e1->e2->e3 [dir=back] [color=red,penwidth=1.5]
    }}
    
    e1->i3 [color=red,penwidth=1.5]
    e3->i4
  }
```)
