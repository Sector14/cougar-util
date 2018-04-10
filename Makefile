
all:
	g++ main.cpp usbdevice.cpp cougardevice.cpp -o cougar-util `pkg-config --libs --cflags libusb-1.0` -std=c++14

clean:
	rm cougar-util
