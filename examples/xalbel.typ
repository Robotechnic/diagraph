#import "@preview/diagraph:0.2.4": *

#raw-render(```
	graph {
		hello -- everyone
		hello -- all
		hello -- test
		hello[xlabel=world]
		everyone[xlabel=alpha]
		test[xlabel="yeyeyeyeyeyeyeyeyeyeyey\neifefijepfijefpijepfijerreheheheheeh"]
		all[xlabel="eeeeeeeeeeeeeeeee", fontname="Arial", fontcolot="green"]
		alpha
	}
```, xlabels: (
	alpha: "test"
))