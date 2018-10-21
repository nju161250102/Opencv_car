source = code-v2.cpp
header = GPIOlib.h
lib = libGPIO.a
prom = main

clean:
	rm -rf $(prom)

$(prom): $(source), $(header), $(lib)
	g++ $(source) -o $(prom) `pkg-config --cflags --libs opencv` -L. -lwiringPi -lGPIO -lpthread

run: $(prom)
	sudo chmod 777 $(prom)
	./$(prom)

