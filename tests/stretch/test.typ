/// [ppi:200]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#let graph = raw-render.with(```
	digraph {
		a -> b
	}
```)

#grid(
	columns: 4,
	column-gutter: 50pt,
  ..for width in (10, 20, 30, 40) {
    for height in (10, 20, 30, 40) {
      let width = width * 1pt
      let height = height * 1pt
      (
        {
          block[
            #width \* #height
            #graph(
              width: width,
              height: height,
            )
            #place(dy: -height, rect([], stroke: red, width: width, height: height))
          ]
          block[
            #graph(
              width: width,
              height: height,
              stretch: false,
            )
            #place(dy: -height, rect([], stroke: red, width: width, height: height))
          ]
          v(height / 2)
        },
      )
    }
  }
)
