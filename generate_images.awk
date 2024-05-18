#!/usr/bin/awk -f

BEGIN {
    in_example = 0
	create_image_tag = 0
	capturing = 0
}

/!\[.*\]\(.*\)/ {
	create_image_tag = 0
}

{
	if (create_image_tag) {
		print "![" name "](https://raw.githubusercontent.com/Robotechnic/diagraph/main/images/" name "1.png)"
		create_image_tag = 0
	}
	print
}

match($0,/^<!--EXAMPLE\((.*)\)-->$/,group) {
    in_example = 1
	name = group[1]
    next
}

/^````typ$/ {
    if (in_example) {
        capturing = 1
		print "#import \"@preview/diagraph:0.2.4\": *" > "images/" name ".typ"
		print "#context{" >> "images/" name ".typ"
		print "let render = [" >> "images/" name ".typ"
        next
    }
}

/^````$/ {
    if (capturing) {
        capturing = 0
        in_example = 0
		print "]" >> "images/" name ".typ"
		print "let dimensions = measure(render)" >> "images/" name ".typ"
		print "set page(width: dimensions.width, height: dimensions.height, margin: 0cm, fill: white)" >> "images/" name ".typ"
		print "render" >> "images/" name ".typ"
		print "}" >> "images/" name ".typ"
		system("typst compile -f png \"images/" name ".typ\" \"images/" name "{n}.png\"")
		create_image_tag = 1
        next
    }
}

capturing {
    print >> "images/" name ".typ"
}
