#import "internals.typ": render

#let raw-render(engine: "dot", width: auto, height: auto, fit: "contain", clip: true, background: "transparent", raw) = {
	if (not raw.has("text")) {
		panic("This function requires a `text` field")
	}
	let dot = raw.text
	return render(dot, engine: engine, width: width, height: height, fit: fit, clip: clip, background: background)
}
