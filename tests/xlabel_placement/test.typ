/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```
	digraph {
		node[math=true, xmath=true]
		edge[lmath=true]
		s[label="sum_(n=0)^3 n"]
		s -> s1
		s -> s2
		s -> s3
		s -> s4
		{rank=same
		s1[label="0"]
		s2[label="1"]
		s3[label="2"]
		s4[label="3"]
		s1 -> s2[label="0 + 1"]
		s2 -> s3[label="1 + 2"]
		s3 -> s4[label="3 + 3"]
		}	
		s4[xlabel="sum_(n=0)^3 n = 6"]

	}
```)