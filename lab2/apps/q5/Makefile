default:
	cd makeprocs; make
	cd inj-h2o; make
	cd inj-so4; make
	cd reaction_h2o; make
	cd reaction_so4; make
	cd reaction_h2so4; make

clean:
	cd makeprocs; make clean
	cd inj-h2o; make clean
	cd inj-so4; make clean
	cd reaction_h2o; make clean
	cd reaction_h2so4; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3 3; ee469_fixterminal
