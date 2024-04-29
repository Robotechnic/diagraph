#import "@preview/diagraph:0.2.3": *


#set heading(numbering: (..nums) => [Graph #numbering("1", ..nums):])

#let render-example(dot, ..args) = style(styles => {
  let code = raw(dot.text, lang: "dot")
  let graph = render(dot.text, ..args)
  let side-by-side = measure(code, styles).width + measure(graph, styles).width < 20cm
  let columns = if side-by-side { (auto, auto) } else { (auto,) }
  grid(
    columns: columns,
    gutter: 1cm,
    raw(dot.text, lang: "dot"),
    graph,
  )
})


= Test

#render-example(
  ```
  digraph {
    rankdir=LR;
    f -> B
    B -> f
    C -> D
    D -> B
    E -> F
    f -> E
    B -> F
  }
  ```
)


= Eating

#render-example(
  ```
  digraph {
    orange -> fruit
    apple -> fruit
    fruit -> food
    carrot -> vegetable
    vegetable -> food
    food -> eat
    eat -> survive
  }
  ```
)


= FFT

Labels are overridden manually.

#render-example(
  ```
  digraph {
    node [shape=none]
    1
    2
    3
    r1
    r2
    r3
    1->2
    1->3
    2->r1 [color=red]
    3->r2 [color=red]
    r1->r3 [color=red]
    r2->r3 [color=red]
  }
  ```,
  labels: (
    "1": $(1,0,0,0), i$,
    "2": $(1,0), -1$,
    "3": $(0,0), -1$,
    r1: $(1,1)$,
    r2: $(0,0)$,
    r3: $(1,1,1,1)$,
  )
)

= State Machine

#render-example(
  ```
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
  ```, labels: (
		"LR_0": $"LR"_0$,
		"LR_1": $"LR"_1$,
		"LR_2": $"LR"_2$,
		"LR_3": $"LR"_3$,
		"LR_4": $"LR"_4$,
		"LR_5": $"LR"_5$,
		"LR_6": $"LR"_6$,
		"LR_7": $"LR"_7$,
		"LR_8": $"LR"_8$
	),
)

#pagebreak()

= Clustering

See http://www.graphviz.org/content/cluster.

#render-example(
  ```
  digraph G {

    subgraph cluster_0 {
      style=filled;
      color=lightgrey;
      node [style=filled,color=white];
      a0 -> a1 -> a2 -> a3;
      label = "process #1";
    }

    subgraph cluster_1 {
      node [style=filled];
      b0 -> b1 -> b2 -> b3;
      label = "process #2";
      color=blue
    }

    start -> a0;
    start -> b0;
    a1 -> b3;
    b2 -> a3;
    a3 -> a0;
    a3 -> end;
    b3 -> end;

    start [shape=Mdiamond];
    end [shape=Msquare];
  }
  ```
)


= HTML

#render-example(
  ```
  digraph structs {
      node [shape=plaintext]
      struct1 [label=<
  <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0">
    <TR><TD>left</TD><TD PORT="f1">mid dle</TD><TD PORT="f2">right</TD></TR>
  </TABLE>>];
      struct2 [label=<
  <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0">
    <TR><TD PORT="f0">one</TD><TD>two</TD></TR>
  </TABLE>>];
      struct3 [label=<
  <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
    <TR>
      <TD ROWSPAN="3">hello<BR/>world</TD>
      <TD COLSPAN="3">b</TD>
      <TD ROWSPAN="3">g</TD>
      <TD ROWSPAN="3">h</TD>
    </TR>
    <TR>
      <TD>c</TD><TD PORT="here">d</TD><TD>e</TD>
    </TR>
    <TR>
      <TD COLSPAN="3">f</TD>
    </TR>
  </TABLE>>];
      struct1:f1 -> struct2:f0;
      struct1:f2 -> struct3:here;
  }
  ```
)


= Overridden labels

Labels for nodes `big` and `sum` are overridden.

```dot
digraph {
	rankdir=LR
	node[shape=circle]
	Hmm -> a_0
	Hmm -> big
	a_0 -> "a'" -> big [style="dashed"]
	big -> sum
}
```

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
  labels: (:
    big: [_some_#text(2em)[ big ]*text*],
    sum: $ sum_(i=0)^n 1/i $,
  ),
)

#pagebreak()

= Automatic math labels

#render-example(
	```
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
	```
)

#raw-render(```
		digraph {
			rankdir=LR
			node[shape=none]
			{
				rank=same
				nx11[fontcolor=yellow]
				x42[fontcolor=yellow]
				nx63[fontcolor=yellow]
				nx84[fontcolor=yellow]
			}
			{
				rank=same
				x21
				nx52
				nx94
			}
			{
				rank=same
				nx31
				x73
			}
			nx11->x21[label=<&alpha;>]
			x21 -> nx31[label=<&beta;>]
			nx31 -> nx121[label=<&gamma;>]
			nx121 -> x151[label="X"]
			x42 -> nx52[label=<&delta;>]
			x21 -> nx52[label=<&delta;>]
			nx63 -> x73[label=<&epsilon;>]
			nx52 -> x73[label=<&epsilon;>]
			nx31 -> x73[label=<&epsilon;>]
			nx84 -> nx94[label=<&theta;>]
			x73 -> nx94[label=<&theta;>]
			nx94 -> x104[label=<&lambda;>]
			x73 -> x104[label=<&lambda;>]
			nx63 -> x104[label=<&lambda;>]
			x104 -> x114[label=<&mu;>]
			nx84 -> x114[label=<&mu;>]
			x73 -> x114[label=<&mu;>]
			x114 -> nx134[label=<&nu;>]
			x114 -> nx144[label=<&tau;>]
		}
	```, labels:(
		nx114: $not x_11 @4$,
		nx84: $not x_8 @4$,
		nx63: $not x_6 @3$,
		x21: $x_2 @1$,
		x42: $x_4 @2$,
		nx31: $not x_3 @1$,
		nx11: $not x_1 @1$,
		nx52: $not x_5 @2$,
		nx94: $not x_9 @4$,
		x151: $x_15 @1$,
		nx121: $not x_12 @1$,
		x73: $x_7 @3$,
		x104: $x_10 @4$,
		nx134: $not x_13 @4$,
		nx144: $not x_14 @4$,
		x114: $x_11 @4$
	), width: 100%)