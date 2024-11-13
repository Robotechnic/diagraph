#import "@preview/diagraph:0.4.0" : *


#raw-render(```
	digraph G {
 		redFiraSans [
			label = "Red\nFiraSans\n20pt",
			color = red,
			fontcolor=red,
			fontsize=20
			fontname="Fira Sans",
		]
		blueDejaVuSans [
			label = "Blue\nDejaVuSans\n10pt",
			color = blue,
			fontcolor=blue,
			fontsize=10
			fontname="DejaVu Sans",
		]
		redFiraSans -> blueDejaVuSans[color = red]
		purpleTimesNewRoman [
			label = "Purple\nTimesNewRoman\n8pt",
			color = purple,
			fontsize=8
			fontcolor=purple,
			fontname="Times New Roman",
		]
		blueDejaVuSans -> purpleTimesNewRoman[color = blue]
		orangeredCourier [
			label = "Orangered\nCourier\n32pt",
			color = orangered,
			fontsize=32
			fontcolor=orangered,
			fontname="Courier",
		]
		purpleTimesNewRoman -> orangeredCourier[color = purple]
		limeComicSans [
			label = "Lime\nComicSans\n15pt",
			color = lime,
			fontsize=15
			fontcolor=lime,
			fontname="Comic Sans",
		]
		orangeredCourier -> limeComicSans[color = orangered]
		efefefArial [
			label = "efefef\nArial\n20pt",
			color = efefef,
			fontsize=20
			fontcolor=efefef,
			fontname="Arial",
		]
		limeComicSans -> efefefArial[color = lime]
		yellowVerdana [
			label = "Yellow\nVerdana\n10pt",
			color = yellow,
			fontsize=10
			fontcolor=yellow,
			fontname="Verdana",
		]
		efefefArial -> yellowVerdana[color = efefef]
		cyanGeorgia [
			label = "Cyan\nGeorgia\n15pt",
			color = cyan,
			fontsize=15
			fontcolor=cyan,
			fontname="Georgia",
		]
		yellowVerdana -> cyanGeorgia[color = yellow]
		blueImpact [
			label = "Blue\nImpact\n20pt",
			color = blue,
			fontsize=20
			fontcolor=blue,
			fontname="Impact",
		]
		cyanGeorgia -> blueImpact[color = cyan]
		redFiraSans -> blueImpact[color = red]
 }
```, engine: "neato")
