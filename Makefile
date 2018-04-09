
all:
	g++ main.cpp usbdevice.cpp -o cougar-util `pkg-config --libs --cflags libusb-1.0`

clean:
	rm cougar-util
