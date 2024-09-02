# diagraph

A simple Graphviz binding for Typst using the WebAssembly plugin system.

## Usage

### Basic usage

<!--EXCLUDE-->
This plugin is quite simple to use, you just need to import it:

```typ
#import "@preview/diagraph:0.3.0": *
```
<!--END-->

You can render a Graphviz Dot string to a SVG image using the `render` function:

```typ
#render("digraph { a -> b }")
```

Alternatively, you can use `raw-render` to pass a `raw` instead of a string:

<!--EXAMPLE(raw-render)-->
````typ
#raw-render(
  ```dot
  digraph {
    a -> b
  }
  ```
)
````
![raw-render](https://raw.githubusercontent.com/Robotechnic/diagraph/main/images/raw-render1.png)

For more information about the Graphviz Dot language, you can check the [official documentation](https://graphviz.org/documentation/).

### Advanced usage

Check the [manual](https://raw.githubusercontent.com/Robotechnic/diagraph/main/doc/manual.pdf) for more information about the plugin.

<!--EXCLUDE-->
## Build

This project was built with emscripten `3.1.46`. Apart from that, you just need to run `make wasm` to build the wasm file. All libraries are downloaded and built automatically to get the right version that works.

There are also some other make commands:

- `make link`: Link the project to the typst plugin folder
- `make clean`: Clean the build folder and the link
- `make clean-link`: Only clean the link
- `make compile_database`: Generate the compile_commands.json file
- `make module`: It copy the files needed to run the plugin in a folder called `graphviz` in the current directory
- `make manual`: Generate the manual pdf
- `make wasi-stub`: Build the wasi stub executable, it require a rust toolchain properly configured

### Wasi stub

Somme functions need to be stubbed to work with the webassembly plugin system. The `wasi-stub` executable is a spetial one fitting the needs of the typst plugin system. You can find the source code [here](https://github.com/astrale-sharp/wasm-minimal-protocol/tree/master). It is important to use this one as the default subbed functions are not the same and the makefile is suited for this one.

### Protocol generator

This package also uses a protocol generator to generate the wasm interface. You can find the source code [here](https://github.com/Robotechnic/WebAssembly-protocol-generator). It is a required dependency to build this project.

<!--END-->

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Changelog

### 0.3.0

- Added support for edge labels
- Added a manual generated with Typst
- Updated graphviz version
- Fix an error in math mode detection

### 0.2.5

- If the shape is point, the label isn't displayed
- Now a minimum size is not enforced if the node label is empty
- Added support for font alternatives

### 0.2.4

- Added support for xlabels which are now rendered by Typst
- Added support for cluster labels which are now rendered by Typst
- Fix a margin problem with the clusters

### 0.2.3

- Updated to typst 0.11.0
- Added support for `fontcolor`, `fontsize` and `fontname` nodes attributes
- Diagraph now uses a protocol generator to generate the wasm interface

### 0.2.2

- Fix an alignment issue
- Added a better mathematic formula recognition for node labels

### 0.2.1

- Added support for relative lenghts in the `width` and `height` arguments
- Fix various bugs

### 0.2.0

- Node labels are now handled by Typst

### 0.1.2

- Graphs are now scaled to make the graph text size match the document text size

### 0.1.1

- Remove the `raw-render-rule` show rule because it doesn't allow use of custom font and the `render` / `raw-render` functions are more flexible
- Add the `background` parameter to the `render` and `raw-render` typst functions and default it to `transparent` instead of `white`
- Add center attribute to draw graph in the center of the svg in the `render` c function

### 0.1.0

Initial working version
