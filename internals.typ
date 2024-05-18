#import "graphviz_interface/protocol.typ": *
#let plugin = plugin("graphviz_interface/diagraph.wasm")


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

/// Convert a number to a string with a fixed number of digits.
/// The number is padded with zeros on the left if necessary.
#let int-to-string(n, digits, base: 10) = {
	let n-str = str(n, base: base)
	let n-len = n-str.len()
	let zeros = "0" * (digits - n-len)
	zeros + n-str
}

/// Get an array of evaluated labels from a graph.
#let get-labels(manual-label-names, manual-xlabel-names, dot) = {
	let labels = manual-label-names.map(label => {
		(
			label: label,
			content: true,
			xlabel: manual-xlabel-names.contains(label),
		)
	});
	let labels = labels + manual-xlabel-names.filter(label => not manual-label-names.contains(label)).map(label => {
		(
			label: label,
			content: false,
			xlabel: true,
		)
	});
	let encoded-labels = plugin.get_labels(encode-overriddenLabels((
		"labels": labels,
		"dot": dot,
	)))
	let (labels, _) = decode-LabelsInfos(encoded-labels)
  labels.at("labels").map(encoded-label => {
		let label = if encoded-label.at("native") {
			if encoded-label.at("math_mode") {
				math.equation(eval(mode: "math", encoded-label.at("label")))
			} else {
				parse-string(encoded-label.at("label"))
			}
		} else {
			encoded-label.at("label")
		}
		let xlabel = if encoded-label.at("xlabel") != "" {
			if encoded-label.at("xlabel_math_mode") {
				math.equation(eval(mode: "math", encoded-label.at("xlabel")))
			} else {
				parse-string(encoded-label.at("xlabel"))
			}
		} else {
			""
		}
		let font_size = text.size
		if encoded-label.at("font_size").pt() != 0 {
			font_size = encoded-label.at("font_size")
		}
		(
			..encoded-label,
			label: label,
			font_size: font_size,
			xlabel: xlabel,
		)
  })
	
}

/// Return a formatted label based on its color, font and content.
#let label-format(color, font, fontsize, label) = [
	#set text(fill: rgb(int-to-string(color, 8, base: 16)), bottom-edge: "bounds")
	#set text(size: fontsize) if fontsize.pt() != 0
	#set text(font: font) if font != ""
	#text(label)
]

#let label-dimensions(color, font, fontsize, label) = {
	let label = label-format(color, font, fontsize, label)
	measure(label)
}

/// Encodes the dimensions of labels into bytes.
#let encode-label-dimensions(labels, overridden-labels, overridden-xlabels) = {
	labels.map(label => {
		if label.at("html") {
			(
				override: false,
				xoverride: false,
				width: 0pt,
				height: 0pt,
				xwidth: 0pt,
				xheight: 0pt,
			)
		} else {
			let dimensions = if label.at("native") {
				label-dimensions(label.at("color"), label.at("font_name"), label.at("font_size"), label.at("label"))
			} else {
				measure(overridden-labels.at(label.at("name")))
			}
			let xdimensions = if label.at("override_xlabel") {
				measure(overridden-xlabels.at(label.at("name")))
			} else if label.at("xlabel") != "" {
				label-dimensions(label.at("color"), label.at("font_name"), label.at("font_size"), label.at("xlabel"))
			} else {
				(width: 0pt, height: 0pt)
			}
			(
				override: true,
				xoverride: label.at("xlabel") != "" or label.at("override_xlabel"),
				width: dimensions.width,
				height: dimensions.height,
				xwidth: xdimensions.width,
				xheight: xdimensions.height,
			)
		}
	})
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

#let debug-rectangle(x, y, width, height) = {
	place(
		top + left,
		dx: x,
		dy: y,
		rect(height: height, width: width, fill: none, stroke:red)
	)
}

/// Renders a graph with Graphviz.
#let render(
  /// A string containing Dot code.
	dot,
  /// Nodes whose name appear in this dictionary will have their label
  /// overridden with the corresponding content. Defaults to an empty
  /// dictionary.
  labels: (:),
	/// Nodes whose name appear in this dictionary will have their xlabel
	/// overridden with the corresponding content. Defaults to an empty
	/// dictionary.
	xlabels: (:),
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
  set math.equation(numbering: none)
  let manual-label-names = labels.keys()
	let manual-xlabel-names = xlabels.keys()
	
	layout(((width: container-width, height: container-height)) => context {
		let labels-infos = get-labels(manual-label-names, manual-xlabel-names, dot)
  	let labels-info-count = labels-infos.len()
		// return [#repr(labels-infos)]

		let encoded-data = (
			"font_size": text.size.to-absolute(),
			"dot": dot,
			"labels": encode-label-dimensions(labels-infos, labels, xlabels),
			"engine": engine,
		)
		// return [#repr(encoded-data)]
		// return [#((array(encode-renderGraph(encoded-data)).map(x => "0x" + int-to-string(x, 2, base: 16))).join(", "))]

		let output = plugin.render(encode-renderGraph(encoded-data))

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

    // Place labels.
		for (label-infos, label-coordinates) in labels-infos.zip(output.at("labels")) {
			if label-infos.at("html") {
				continue
			}
			let label = if label-infos.at("native") {
				label-infos.at("label")
			} else {
				labels.at(label-infos.at("name"))
			}
			let label = label-format(label-infos.at("color"), label-infos.at("font_name"), label-infos.at("font_size"), label)
			let label-dimensions = measure(label)
			place(
				top + left,
				dx: label-coordinates.at("x") - label-dimensions.width / 2,
				dy: final-height - label-coordinates.at("y") - label-dimensions.height / 2 - (final-height - svg-height),
				label
			)

			// debug-rectangle(
			// 	label-coordinates.at("x") - label-dimensions.width / 2,
			// 	final-height - label-coordinates.at("y") - label-dimensions.height / 2 - (final-height - svg-height),
			// 	label-dimensions.width,
			// 	label-dimensions.height
			// )

			let xlabel = if label-infos.at("override_xlabel") {
				xlabels.at(label-infos.at("name"))
			} else if label-infos.at("xlabel") != "" {
				label-infos.at("xlabel")
			} else {
				continue
			}

			let xlabel = label-format(label-infos.at("color"), label-infos.at("font_name"), label-infos.at("font_size"), xlabel)
			let xlabel-dimensions = measure(xlabel)
			place(
				top + left,
				dx: label-coordinates.at("xx") - xlabel-dimensions.width / 2,
				dy: final-height - label-coordinates.at("xy") - xlabel-dimensions.height / 2 - (final-height - svg-height),
				xlabel
			)
			// debug-rectangle(
			// 	label-coordinates.at("xx") - xlabel-dimensions.width / 2,
			// 	final-height - label-coordinates.at("xy") - xlabel-dimensions.height / 2 - (final-height - svg-height),
			// 	xlabel-dimensions.width,
			// 	xlabel-dimensions.height
			// )
		}
	})
}
