default:
	cd makeprocs; make
	cd inj-co; make
	cd inj-s2; make
	cd reaction_co; make
	cd reaction_s2; make
	cd reaction_so4; make

clean:
	cd makeprocs; make clean
	cd inj-co; make clean
	cd inj-s2; make clean
	cd reaction_co; make clean
	cd reaction_s2; make clean
	cd reaction_so4; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1 4; ee469_fixterminal
