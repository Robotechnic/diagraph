#import "@preview/diagraph:0.2.3" : *

#raw-render(```
	digraph G {
 		{
			one [
				label = "First\nSecond",
				color = "#BF101D",
				fontcolor = "#BF101D"
			];
			two [
				label = "Third\nFourth",
				color = "#BF101D",
				fontcolor = "#BF101D"
			];
    }
    one -> two [
			color = "#BF101D",
			penwidth = 2,
		];
 }
```)