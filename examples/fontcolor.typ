#import "@preview/diagraph:0.2.3" : *

#raw-render(```
	digraph G {
 		{
			one [
				label = "First\nSecond",
				color = "#BF101D",
				fontcolor = "#BF101D",
			];
			two [
				label = "Third\nFourth",
				color = "#BF101D",
				fontcolor = "#BF101F"
			];
    }
    one -> two [
			color = "#BF101D",
			penwidth = 2,
		];

		three [
			label = "Fifth\nSixth",
			color = "#BF101D",
			fontcolor = "orange",
		];
		two -> three [
			color = "#BF101D",
			penwidth = 2,
		];
		four [
			label = "Seventh\nEighth",
			color = "#BF101D",
			fontcolor = "red",
		];
		three -> four [
			color = "#BF101D",
			penwidth = 2,
		];
		five [
			label = "Ninth\nTenth",
			color = "#BF101D",
			fontcolor = "blue",
		];
		four -> five [
			color = "#BF101D",
			penwidth = 2,
		];
		six [
			label = "Eleventh\nTwelfth",
			color = "#BF101D",
			fontcolor = "green",
		];
		five -> six [
			color = "#BF101D",
			penwidth = 2,
		];
		seven [
			label = "Thirteenth\nFourteenth",
			color = "#BF101D",
			fontcolor = "purple",
		];
		six -> seven [
			color = "#BF101D",
			penwidth = 2,
		];
		eight [
			label = "Fifteenth\nSixteenth",
			color = "#BF101D",
			fontcolor = "yellow",
		];
		seven -> eight [
			color = "#BF101D",
			penwidth = 2,
		];
		nine [
			label = "Seventeenth\nEighteenth",
			color = "#BF101D",
			fontcolor = "black",
		];
		eight -> nine [
			color = "#BF101D",
			penwidth = 2,
		];
		ten [
			label = "Nineteenth\nTwentieth",
			color = "#BF101D",
			fontcolor = "white",
		];
		nine -> ten [
			color = "#BF101D",
			penwidth = 2,
		];
		eleven [
			label = "Twenty-first\nTwenty-second",
			color = "#BF101D",
			fontcolor = "gray",
		];
		ten -> eleven [
			color = "#BF101D",
			penwidth = 2,
		];
		twelve [
			label = "Twenty-third\nTwenty-fourth",
			color = "#BF101D",
			fontcolor = "brown",
		];
		eleven -> twelve [
			color = "#BF101D",
			penwidth = 2,
		];
		twelve -> one
 }
```, engine: "neato")
