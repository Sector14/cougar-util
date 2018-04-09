
all:
	g++ main.cpp -o cougar-util `pkg-config --libs --cflags libusb-1.0`

clean:
	rm cougar-util
