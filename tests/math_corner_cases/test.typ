/// [ppi:100]

#import "../../lib.typ": raw-render
#set page(width: auto, height: auto, margin: 1cm)

#raw-render(```dot
  digraph G {
    1_2
    "   _   7"
    "  ^   8"
    "   a_   7"
    "   b^    8                 o"
    "a   7"
    " a   7"
    "b - ^+_ - 9"
  }
```)