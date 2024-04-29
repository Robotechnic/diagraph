#import "graphviz_interface/protocol.typ": *
#let plugin = plugin("diagraph.wasm")


/// Converts a string containing escape sequences to content.
#let parse-string(s) = {
  let result = []
  let row = ""
  let is-escaped = false

  for cluster in s {
    if is-escaped {
      is-escaped = false

      if cluster == "l" {
        result += align(left, row)
        row = ""
      } else if cluster == "n" {
        result += align(center, row)
        row = ""
      } else if cluster == "r" {
        result += align(right, row)
        row = ""
      } else {
        row += cluster
      }
    } else if cluster == "\\" {
      is-escaped = true
    } else {
      row += cluster
    }
  }

  set block(spacing: 0.65em)
  result + align(center, row)
}

/// Get an array of evaluated labels from a graph.
#let get-labels(manual-label-names, dot) = {
	let encoded-labels = plugin.get_labels(encode-overriddenLabels((
		"labels": manual-label-names,
		"dot": dot,
	)))
	let (labels, size) = decode-nativeLabels(encoded-labels)
  labels.at("nativeLabels").map(encoded-label => {
    if encoded-label.at("mathMode") {
      math.equation(eval(mode: "math", encoded-label.at("label")))
    } else {
      parse-string(encoded-label.at("label"))
    }
  })
}

/// Encodes the dimensions of labels into bytes.
#let encode-label-dimensions(labels, labels-name: ()) = {
	if labels-name == () {
		labels.map(label => {
			let dimensions = measure(label)
			(
				label: "",
				width: dimensions.width / 1pt,
				height: dimensions.height / 1pt,
			)
		})
	} else {
		labels.zip(labels-name).map((label) => {
			let dimensions = measure(label.at(0))
			(
				label: label.at(1),
				width: dimensions.width / 1pt,
				height: dimensions.height / 1pt,
			)
		})
	}
}

/// Converts any relative length to an absolute length.
#let relative-to-absolute(value, container-dimension) = {
  if type(value) == relative {
    let absolute-part = relative-to-absolute(value.length, container-dimension)
    let ratio-part = relative-to-absolute(value.ratio, container-dimension)
    return absolute-part + ratio-part
  }
  if type(value) == length {
    return value.to-absolute()
  }
  if type(value) == ratio {
    return value * container-dimension
  }
  panic("Expected relative length, found " + str(type(value)))
}

#let two-digits(n) = {
	if n.len() == 1 {
		"0" + n
	} else {
		n
	}
}

/// Renders a graph with Graphviz.
#let render(
  /// A string containing Dot code.
	dot,
  /// Nodes whose name appear in this dictionary will have their label
  /// overridden with the corresponding content. Defaults to an empty
  /// dictionary.
  labels: (:),
  /// The name of the engine to generate the graph with. Defaults to `"dot"`.
	engine: "dot",
  /// The width of the image to display. If set to `auto` (the default), will be
  /// the width of the generated SVG or, if the height is set to a value, it
  /// will be scaled to keep the aspect ratio.
	width: auto,
  /// The height of the image to display. If set to `auto` (the default), will
  /// be the height of the generated SVG or if the width is set to a value, it
  /// will be scaled to keep the aspect ratio.
	height: auto,
  /// Whether to hide parts of the graph that extend beyond its frame. Defaults
  /// to `true`.
  clip: true,
  /// A color or gradient to fill the background with. If set to `none` (the
  /// default), the background will be transparent.
  background: none,
) = {
  let manual-labels = labels.values()
  let manual-label-names = labels.keys()
  let manual-label-count = manual-labels.len()
  let native-labels = get-labels(manual-label-names, dot)
  let native-label-count = native-labels.len()
	
	layout(((width: container-width, height: container-height)) => context {
		// return (repr((
		// 	"fontSize": text.size.to-absolute(),
		// 	"dot": dot,
		// 	"nativeLabels": encode-label-dimensions(native-labels),
		// 	"manualLabels": encode-label-dimensions(manual-labels, labels-name: manual-label-names),
		// 	"engine": engine,
		// )))
		let output = plugin.render(encode-renderGraph((
			"fontSize": text.size.to-absolute(),
			"dot": dot,
			"nativeLabels": encode-label-dimensions(native-labels),
			"manualLabels": encode-label-dimensions(manual-labels, labels-name: manual-label-names),
			"engine": engine,
		)))

		if output.at(0) != 0 {
			return {
				show: highlight.with(fill: red)
				set text(white)
				raw(block: true, str(output))
			}
		}

		let output = decode-graphInfo(output).at(0)

		// return [#repr(output)]

    // Get SVG dimensions.
    let (width: svg-width, height: svg-height) = measure(image.decode(output.at("svg"), format: "svg"))

		let final-width = if width == auto {
      svg-width
    } else {
      relative-to-absolute(width, container-width)
    }
		let final-height = if height == auto {
      svg-height
    } else {
      relative-to-absolute(height, container-height)
    }

    if width == auto and height != auto {
      let ratio = final-height / svg-height
      final-width = svg-width * ratio
    } else if width != auto and height == auto {
      let ratio = final-width / svg-width
      final-height = svg-height * ratio
    }
		// Rescale the final image to the desired size.
		show: block.with(
      width: final-width,
      height: final-height,
      clip: clip,
      breakable: false,
    )

    set align(top + left)

		show: scale.with(
      origin: top + left,
      x: final-width / svg-width * 100%,
      y: final-height / svg-height * 100%,
    )

		// Construct the graph and its labels.
		show: block.with(width: svg-width, height: svg-height, fill: background)

    // Display SVG.
		image.decode(
			output.at("svg"),
			format: "svg",
			width: svg-width,
			height: svg-height,
		)

    // Place native labels.
		for (label, coordinates) in native-labels.zip(output.at("nativeLabels")) {
			let label-dimensions = measure(label)
			place(
				top + left,
				dx: coordinates.at("x") - label-dimensions.width / 2,
				dy: final-height - coordinates.at("y") - label-dimensions.height / 2 - (final-height - svg-height),
				label,
			)
		}

    // Place manual labels.
		for (label, coordinates) in manual-labels.zip(output.at("manualLabels")) {
			let label-dimensions = measure(label)
			place(
				top + left,
				dx: coordinates.at("x") - label-dimensions.width / 2,
				dy: final-height - coordinates.at("y") - label-dimensions.height / 2 - (final-height - svg-height),
				label,
			)
		}
	})
}
