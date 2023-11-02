#let plugin = plugin("diagraph.wasm")

#let big-endian-decode(bytes) = {
	let result = 0
	for byte in array(bytes) {
		result = result * 256
		result = result + byte
	}
	return result
}

#let render(text, engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent") = {
	let render = plugin.render(
		bytes(text), 
		bytes(engine), 
		bytes(background)
	)

	if render.at(0) == 1 {
		return raw(str(render))
	}

	if render.at(0) != 0 {
		panic("First byte of render result must be 0 or 1")
	}

	// let integer_size = render.at(1)
	// let point_width = big-endian-decode(render.slice(2, integer_size + 2))
	// let point_height = big-endian-decode(render.slice(integer_size + 3, integer_size * 2 + 2))
	
	return image.decode(
		render.slice(2 + integer_size * 2),
		format: "svg",
		height: height,
		width: width,
		fit: fit,
	)
}

#let raw-render(engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent", raw) = {
	if (not raw.has("text")) {
		panic("This function requires a `text` field")
	}
	let text = raw.text
	return render(text, engine: engine, width: width, height: height, fit: fit, background: background)
}
