#let render(
	dot,
	node-labels: (:),
	engine: "dot",
	width: auto,
	height: auto,
	fit: "contain",
	background: "transparent"
) = {
	import "utils.typ": *

	let plugin = plugin("diagraph.wasm")


	style(styles => {
		let font-size = measure(line(length: 1em), styles).width

		let node-label-infos = node-labels.pairs().map(((id, label)) => {
			let dimensions = measure(label, styles)
			(id, dimensions.width, dimensions.height)
		})

		let background-string = if type(background) == color {
			background.to-hex()
		} else {
			background
		}

		let output = plugin.render(
			big-endian-encode(calc.round(font-size / 0.01pt)),
			bytes(dot),
			encode-node-label-info-array(node-label-infos),
			bytes(engine),
			bytes(background-string)
		)

		if output.at(0) == 1 {
			return {
				show: highlight.with(fill: red)
				set text(white)
				raw(block: true, str(output.slice(1)))
			}
		}

		if output.at(0) != 0 {
			panic("First byte of output must be 0 or 1")
		}

		let integer_size = output.at(1)
		output = output.slice(2)

		// Get the coordinates of all nodes with overridden labels
		let node-coordinates-size = 2 * node-labels.len() * integer_size
		let node-coordinates = array-to-pairs(group-bytes(output.slice(0, node-coordinates-size), integer_size).map(big-endian-decode))
		output = output.slice(node-coordinates-size)

		/// Returns a `(width, height)` pair corresponding to the dimensions of the SVG stored in the bytes.
		let get-svg-dimensions(svg) = {
			let point_width = big-endian-decode(output.slice(0, integer_size)) * 1pt
			let point_height = big-endian-decode(output.slice(integer_size + 1, integer_size * 2)) * 1pt
			return (point_width, point_height)
		}

		let (auto-width, auto-height) = get-svg-dimensions(output)
		output = output.slice(integer_size * 2)

		let final-width = if width == auto { auto-width } else { width }
		let final-height = if height == auto { auto-height } else { height }

		// Rescale the final image to the desired size
		show: block.with(width: final-width, height: final-height)
		show: scale.with(x: final-width / auto-width * 100%, y: final-height / auto-height * 100%)

		// Construct the graph with proper labels
		show: block.with(width: auto-width, height: auto-height)

		image.decode(
			output,
			format: "svg",
			width: auto-width,
			height: auto-height,
			fit: fit,
		)

		for (label, coordinates) in array.zip(node-labels.values(), node-coordinates) {
			let (x, y) = coordinates.map(w => w * 1pt)
			let label-dimensions = measure(label, styles)
			place(
				top + left,
				dx: x - label-dimensions.width / 2,
				dy: final-height - y - label-dimensions.height / 2,
				boxed-label,
			)
		}
	})
}

#let raw-render(node-labels: (:), engine: "dot", width: auto, height: auto, fit: "contain", background: "transparent", raw) = {
	if (not raw.has("text")) {
		panic("This function requires a `text` field")
	}
	let dot = raw.text
	return render(dot, node-labels: node-labels, engine: engine, width: width, height: height, fit: fit, background: background)
}
