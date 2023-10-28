#let plugin = plugin("diagraph.wasm")

#let render(text, engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent") = {
	let render = str(
		plugin.render(
			bytes(text),
			bytes(engine),
			bytes(background)
		)
	)

	if render.slice(0, 6) == "error:" {
		return raw(render)
	}

	let default = image.decode(
		render,
		format: "svg",
		width: width,
		height: height,
		fit: fit,
	)

	if width != auto and height != auto {
		return default
	}

	/// Returns a `(width, height)` pair corresponding to the dimensions of an
	/// SVG, which is passed as its source code.
	///
	/// If the dimensions of the SVG cannot be inferred, returns `none`.
	let get-svg-dimensions(svg) = {
		// Matches an arbitrary amount of arbitrary XML attributes with a
		// double-quoted values.
		let xml-attributes-regex = `(?:[\w-]+=".+?[^\\]"|\s*)*?`.text
		// Matches a "width" XML attribute and captures its value.
		let width-attribute-regex = `(?:width="([\d.]+)pt")`.text
		// Same for height.
		let height-attribute-regex = `(?:height="([\d.]+)pt")`.text
		// Matches a XML opening `svg` tag with a "width" attribute, and its
		// value in pt.
		let svg-width-regex = regex(
			`<svg\s`.text
			+ xml-attributes-regex
			+ width-attribute-regex
			+ xml-attributes-regex
			+ ">"
		)
		// Matches a XML opening `svg` tag with a "height" attribute, and its
		// value in pt.
		let svg-height-regex = regex(
			`<svg\s`.text
			+ xml-attributes-regex
			+ height-attribute-regex
			+ xml-attributes-regex
			+ ">"
		)

		let width-match = svg.match(svg-width-regex)
		if width-match == none {
			return none
		}
		let width = float(width-match.captures.first()) * 1pt

		let height-match = svg.match(svg-height-regex)
		if height-match == none {
			return none
		}
		let height = float(height-match.captures.first()) * 1pt

		(width, height)
	}

	let initial-dimensions = get-svg-dimensions(render)

	if initial-dimensions == none {
		return default
	}

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
