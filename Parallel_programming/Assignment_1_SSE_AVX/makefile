cpps=$(filter-out seq,$(subst .cpp,,$(wildcard *.cpp)))
odir=/tmp/$(shell echo $$PWD|md5sum|cut -c-10)
n=1600000
d=3

seq:
	@mkdir -p -m 700 ${odir} && rm -rf ${odir}/*
	g++ -DD=$(d) -march=native -O3 -fno-tree-vectorize seq.cpp -o ${odir}/seq && ${odir}/seq $(n)

$(cpps):
	@mkdir -p -m 700 ${odir} && rm -rf ${odir}/*
	g++ -DD=$(d) -march=native -O3 $@.cpp -o ${odir}/$@ && ${odir}/$@ $(n)

edit:
	@which code >/dev/null && code -n *.cpp makefile || echo "visual studio code not installed :("

zip:
	@rm -f ../lab1.zip && zip ../lab1.zip seq.cpp makefile