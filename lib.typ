#let plugin = plugin("graphviz.wasm")

#let render(text, width: auto, height: auto, fit: "cover") = {
	let render = str(plugin.render(bytes(text)))
	if render.slice(0, 6) == "error:" {
		return raw(render)
	} else {
		return image.decode(
			render,
			"svg"
		)
	}
}

#show raw.where(lang: "dotrender"): it => {
	let text = it.text()
	
}