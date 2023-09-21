# typst-graphviz

A simple graphviz binding for typst using the new webassembly plugin system

## Usage

This plugin is quite simple to use, you just need to import it:

```typ
#import "@preview/typst-graphviz:0.1.0": *
```

For now, the only function available is `render`, which takes a graphviz dot string and returns a svg string.

```typ
#render("digraph { a -> b }")
```

For a simpler usage, you can setup a simple show rule:

```typ
#show raw.where(lang: "dotrender"): it => {
    let text = it.text
    return render(text)
}
```

This allow you to draw a graphviz graph like this:

```typ
```dotrender
digraph {
    a -> b
}
`` `
```

You can see an exemple of this in the [example](./example) folder.

For more information about the graphviz dot language, you can check the [official documentation](https://graphviz.org/documentation/).

## Functions definitions

### render

```typ
render(
    dot: string,
    engine: string,
    width: auto relative,
    height: auto relative,
    fit: string
)
```

| Parameter | Description | Default value |
| :-------: | :---------: | :-----------: |
| dot | The dot string to render | Required |
| engine | The graphviz engine to use | dot |
| width | The width of the svg | auto |
| height | The height of the svg | auto |
| fit | The fit mode of the svg | cover |

All the optional parameters are the same as the one of images, you can check the [documentation](https://typst.app/docs/reference/visualize/image/) for more details.

## Future work

I plan to add more utilities to this plugin:

- A way to render an automata with graphviz
- A way to render a syntax tree

## Build

This project was built with emscripten `3.1.45`. Apart from that, you just need to run `make wasm` to build the wasm file. All libraries are downloaded and built automatically to get the right version that works.

There are also some other make commands:

- `make clean`: Clean the build folder
- `make link`: Link the project to the typst plugin folder
- `make compile_database`: Generate the compile_commands.json file

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
