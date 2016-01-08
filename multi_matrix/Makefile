
check: all
	./multi.py < input/qwe.txt | sort > poutput
	sort < ./output/part-r-00000 > houtput
	diff poutput houtput
	rm -f poutput houtput
	@echo CHECK COMPLETED.
	@echo No errors found

all: Run.jar
	rm -rf output
	hadoop jar ./Run.jar Run input output

input: gen.py
	rm -f input
	mkdir input
	./gen.py > ./input/qwe.txt

Run.jar: Run.java
	rm -rf classes
	mkdir classes
	javac -cp `hadoop classpath` Run.java -d classes
	jar cvf Run.jar classes/*
	rm -rf classes

clean:
	rm -rf classes
	rm -f Run.jar
	rm -rf input
	rm -rf output
	rm -rf __pycache__

.PHONY: clean
