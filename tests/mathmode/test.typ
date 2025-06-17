/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(
  ```
  	digraph G {
  		rankdir=LR
  		alpha -> beta
  		beta -> "sum_(i=0)^n 1 / i"
  	}
  ```,
  math-mode: auto,
)
#raw-render(
  ```
  	digraph G {
  		rankdir=LR
  		alpha -> beta
  		beta -> "sum_(i=0)^n 1 / i"
  	}
  ```,
  math-mode: "math",
)
#raw-render(
  ```
  	digraph G {
  		rankdir=LR
  		alpha -> beta
  		beta -> "sum_(i=0)^n 1 / i"
  	}
  ```,
  math-mode: "text",
)
