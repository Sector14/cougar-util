
all:
	g++ src/main.cpp src/usbdevice.cpp src/cougardevice.cpp -o cougar-util `pkg-config --libs --cflags libusb-1.0 libcrypto++` -std=c++14

clean:
	rm cougar-util
