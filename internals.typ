#let plugin = plugin("diagraph.wasm")


#let double-precision = 1000

#let length-to-int(value) = {
  calc.round(value * double-precision / 1pt)
}

#let int-to-length(value) = {
  value / double-precision * 1pt
}


/// Encodes a 32-bytes integer into big-endian bytes.
#let encode-int(value) = {
  bytes((
    calc.rem(calc.quo(value, 0x1000000), 0x100),
    calc.rem(calc.quo(value, 0x10000), 0x100),
    calc.rem(calc.quo(value, 0x100), 0x100),
    calc.rem(calc.quo(value, 0x1), 0x100),
  ))
}

/// Decodes a big-endian integer from the given bytes.
#let decode-int(bytes) = {
  let result = 0
  for byte in array(bytes) {
    result = result * 256 + byte
  }
  return result
}

/// Encodes an array of integers into bytes.
#let encode-int-array(arr) = {
  bytes(
    arr
      .map(encode-int)
      .map(array)
      .flatten()
  )
}

/// Transforms bytes into an array whose elements are all `bytes` with the specified length.
#let group-bytes(buffer, group-len) = {
  assert(calc.rem(buffer.len(), group-len) == 0)
  array(buffer).fold((), (acc, x) => {
    if acc.len() != 0 and acc.last().len() < group-len {
      acc.last().push(x)
      acc
    } else {
      acc + ((x,),)
    }
  }).map(bytes)
}

/// Group elements of the array in pairs.
#let array-to-pairs(arr) = {
  assert(calc.even(arr.len()))
  arr.fold((), (acc, x) => {
    if acc.len() != 0 and acc.last().len() < 2 {
      acc.last().push(x)
      acc
    } else {
      acc + ((x,),)
    }
  })
}

/// Get an array of evaluated labels from a graph.
#let get-labels(dot) = {
  let encoded-labels = plugin.get_labels(bytes(dot))
  let encoded-label-array = array(encoded-labels).split(0).slice(0, -1)
  encoded-label-array.map(label => eval(mode: "markup", str(bytes(label))))
}

/// Encodes the dimensions of labels into bytes.
#let encode-label-dimensions(styles, labels) = {
  encode-int-array(
    labels
      .map(label => {
        let dimensions = measure(label, styles)
        (
          length-to-int(dimensions.width),
          length-to-int(dimensions.height),
        )
      })
      .flatten()
  )
}

#let render(
	dot,
	engine: "dot",
	width: auto,
	height: auto,
	fit: "contain",
  clip: false,
	background: "transparent",
) = {
  if type(background) == color {
    background = background.to-hex()
  }

  let labels = get-labels(dot)
  let label-count = labels.len()

	style(styles => {
		let font-size = measure(line(length: 1em), styles).width

		let output = plugin.render(
			encode-int(length-to-int(font-size)),
			bytes(dot),
      encode-label-dimensions(styles, labels),
			bytes(engine),
			bytes(background)
		)

		if output.at(0) != 0 {
			return {
				show: highlight.with(fill: red)
				set text(white)
				raw(block: true, str(output))
			}
		}

		let integer-size = output.at(1)
		output = output.slice(2)

		// Get label coordinates.
		let label-coordinates-size = 2 * label-count * integer-size
		let label-coordinates = array-to-pairs(
      group-bytes(output.slice(0, label-coordinates-size), integer-size)
      .map(decode-int)
      .map(int-to-length)
    )
		output = output.slice(label-coordinates-size)

    // Get SVG dimensions.
    let svg-width = int-to-length(decode-int(output.slice(0, integer-size)))
    let svg-height = int-to-length(decode-int(output.slice(integer-size + 1, integer-size * 2)))
		output = output.slice(integer-size * 2)

    // panic(str(output))

		let final-width = if width == auto { svg-width } else { width }
		let final-height = if height == auto { svg-height } else { height }

		// Rescale the final image to the desired size
		show: block.with(
      width: final-width,
      height: final-height,
      clip: clip,
    )
		show: scale.with(
      origin: top + left,
      x: final-width / svg-width * 100%,
      y: final-height / svg-height * 100%,
    )

		// Construct the graph with proper labels
		show: block.with(width: svg-width, height: svg-height)

    // Place SVG
		image.decode(
			output,
			format: "svg",
			width: svg-width,
			height: svg-height,
			fit: fit,
		)

    // Place labels
		for (label, coordinates) in labels.zip(label-coordinates) {
			let (x, y) = coordinates
			let label-dimensions = measure(label, styles)
			place(
				top + left,
				dx: x - label-dimensions.width / 2,
				dy: final-height - y - label-dimensions.height / 2,
				label,
			)
		}
	})
}
