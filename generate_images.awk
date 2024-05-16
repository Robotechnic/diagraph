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
		print "![" name "](https://github.com/Robotechnic/diagraph/tree/main/images/" name ".svg)"
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
		print "#import \"@preview/diagraph:0.2.3\": *" > "images/" name ".typ"
        next
    }
}

/^````$/ {
    if (capturing) {
        capturing = 0
        in_example = 0
		system("typst compile -f svg \"images/" name ".typ\"")
		create_image_tag = 1
        next
    }
}

capturing {
    print >> "images/" name ".typ"
}
