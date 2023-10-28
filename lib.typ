#let plugin = plugin("diagraph.wasm")

#let render(text, engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent") = {
	let render = str(
		plugin.render(
			bytes(text),
			bytes(engine),
			bytes(background)
		)
	)

	/// Returns a `(width, height)` pair corresponding to the dimensions of an
	/// SVG, which is passed as its source code.
	///
	/// If the dimensions of the SVG cannot be inferred, returns `none`.
	let get-svg-dimensions(svg) = {
		// Matches an arbitrary amount of arbitrary XML attributes with a
		// double-quoted values.
		let xml-attributes-regex = `(?:(?:\w|-)+=".+?[^\\]"|\s*)*?`.text
		// Matches a "width" XML attribute and captures its value.
		let width-attribute-regex = `(?:width="(\d+(?:pt|cm))")`.text
		// Same for height.
		let height-attribute-regex = `(?:height="(\d+(?:pt|cm))")`.text
		// Matches a XML opening `svg` tag with "width" and "height" attributes
		// in this order, and captures the width and the height in pt.
		let svg-width-height-regex = regex(
			`<svg\s+`.text
			+ xml-attributes-regex
			+ width-attribute-regex
			+ xml-attributes-regex
			+ height-attribute-regex
			+ xml-attributes-regex
			+ ">"
		)
		// Same, but with "width" and "height" in reverse order.
		let svg-height-width-regex = regex(
			`<svg\s+`.text
			+ xml-attributes-regex
			+ width-attribute-regex
			+ xml-attributes-regex
			+ height-attribute-regex
			+ xml-attributes-regex
			+ ">"
		)

		let result
		let match = svg.match(svg-width-height-regex)
		if match != none {
			result = match.captures
		} else {
			let match = svg.match(svg-height-width-regex)
			if match != none {
				result = match.captures
			} else {
				return none
			}
		}
		result.map(eval)
	}

	if render.slice(0, 6) == "error:" {
		return raw(render)
	}

	if width != auto and height != auto {
		return image.decode(
			render,
			format: "svg",
			width: width,
			height: height,
			fit: fit,
		)
	}

	let initial-dimensions = get-svg-dimensions(render)
	let svg-text-size = 14pt // Default font size in Graphviz
	style(styles => {
		let document-text-size = measure(line(length: 1em), styles).width
		let (auto-width, auto-height) = initial-dimensions.map(dimension => {
			dimension / svg-text-size * document-text-size
		})

		let final-width = if width == auto { auto-width } else { width }
		let final-height = if height == auto { auto-height } else { height }

		return image.decode(
			render,
			format: "svg",
			width: final-width,
			height: final-height,
			fit: fit,
		)
	})
}

#let raw-render(engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent", raw) = {
	if (not raw.has("text")) {
		panic("This function requires a `text` field")
	}
	let text = raw.text
	return render(text, engine: engine, width: width, height: height, fit: fit, background: background)
}
