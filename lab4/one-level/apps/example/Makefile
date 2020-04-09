default:
	cd makeprocs; make
	cd hello_world; make
	cd Step2.1; make
	cd Step2.2; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd Step2.1; make clean
	cd Step2.2; make clean

run0:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 0; ee469_fixterminal

run1:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1; ee469_fixterminal

run2:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 1; ee469_fixterminal