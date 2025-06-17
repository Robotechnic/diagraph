/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(
  ```
  	digraph G {
  		rankdir=LR
  		subgraph cluster_0 {
  			A -> B
  		}
  	}
  ```,
  clusters: ("cluster_0": "Cluster 0"),
)
