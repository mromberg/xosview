
all: default.xrc pink.xrc book.xrc black-nw.xrc white-nb.xrc\
     space.xrc clover.xrc spring.xrc holes.xrc sky.xrc classic.xrc

clean:
	rm -f *~ *.xrc

%.xrc: %.m4 Xdefaults.m4
	m4 $< Xdefaults.m4 > $@
