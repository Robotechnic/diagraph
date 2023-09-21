#let plugin = plugin("graphviz.wasm")

#let render(text, engine: "dot", width: auto, height: auto, fit: "cover") = {
	if text.len() != bytes(text).len() {
		return raw("error: text must be utf-8 encoded")
	}

	// add null terminator
	let encodedText = bytes(text) + bytes((0,))
	let encodedEngine = bytes(engine) + bytes((0,))

	let render = str(
		plugin.render(encodedText, encodedEngine)
	)

	if render.slice(0, 6) == "error:" {
		return raw(render)
	} else {
		return image.decode(
			render,
			format: "svg",
			height: height,
			width: width,
			fit: fit,
		)
	}
}

#show raw.where(lang: "dotrender"): it => {
	let text = it.text()
	return render(text)
}

