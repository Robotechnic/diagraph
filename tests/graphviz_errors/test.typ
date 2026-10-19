#import "../../lib.typ": render
#import "../../graphviz_interface/protocol.typ": decode-GraphInfo, decode-graphInfo, encode-GetGraphInfo, encode-renderGraph

#let plugin = plugin("../../graphviz_interface/diagraph.wasm")

#let unlabeled-edge = (
  overwrite: false, width: 0pt, height: 0pt,
  xoverwrite: false, xwidth: 0pt, xheight: 0pt,
  headoverwrite: false, headwidth: 0pt, headheight: 0pt,
  tailoverwrite: false, tailwidth: 0pt, tailheight: 0pt,
)

// The warnings and errors Graphviz reports while it draws `dot`, which `render`
// turns into an error depending on `fail-on`.
#let graphviz-report(dot) = {
  let (graph, _) = decode-GraphInfo(plugin.get_labels(encode-GetGraphInfo((dot: dot))))
  let (output, _) = decode-graphInfo(plugin.render(encode-renderGraph((
    font_size: 10pt,
    dot: dot,
    labels: graph.labels.map(node => (
      overwrite: false, xoverwrite: false,
      width: 0pt, height: 0pt, xwidth: 0pt, xheight: 0pt,
      edges_size: (unlabeled-edge,) * node.edges_infos.len(),
    )),
    cluster_labels: graph.cluster_labels.map(_ => (width: 0pt, height: 0pt)),
    engine: "dot",
  ))))
  (messages: output.messages, messages_include_errors: output.messages_include_errors)
}

// The dot layout fails on this graph but returns success: Graphviz reports the
// error only through agerr.
#let layout-error-dot = "digraph { subgraph cluster_a { a; b; c } { rank=same; a; b } a -> b }"
#let (messages, messages_include_errors) = graphviz-report(layout-error-dot)
#assert(messages.contains("Error: install_in_rank"), message: messages)
#assert(messages.contains("Warning: a was already in a rankset"), message: messages)
#assert(messages_include_errors)

#let layout-warning-dot = "digraph { a [shape=no_such_shape] }"
#let (messages, messages_include_errors) = graphviz-report(layout-warning-dot)
#assert(messages.contains("Warning: using box for unknown shape no_such_shape"), message: messages)
#assert(not messages_include_errors)

// Graphviz reports this warning only while it draws the laid out graph.
#let drawing-warning-dot = "digraph { a [color=no_such_color] }"
#let (messages, messages_include_errors) = graphviz-report(drawing-warning-dot)
#assert(messages.contains("Warning: no_such_color is not a known color."), message: messages)
#assert(not messages_include_errors)

#assert.eq(graphviz-report("digraph { a -> b }"), (messages: "", messages_include_errors: false))

// The placeholders that reserve room for labels, including the empty label of
// a cluster, cause no warnings.
#render("digraph { a -> b [label=x]; subgraph cluster_c { c } }", fail-on: "warning")

// The graphs are drawn despite the messages that `fail-on` lets through.
#render(layout-error-dot)
#render(layout-warning-dot)
#render(drawing-warning-dot)
#render(layout-warning-dot, fail-on: "error")
#render(drawing-warning-dot, fail-on: "error")
