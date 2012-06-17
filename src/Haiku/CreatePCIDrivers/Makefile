all: ../../EthernetDriverStub.i ../../VideoDriverStub.i

clean:
	-rm *.o hexconv Ethernet Video

../../EthernetDriverStub.i: Ethernet hexconv
	hexconv $< $@

../../VideoDriverStub.i: Video hexconv
	hexconv $< $@

hexconv: hexconv.cpp
	$(CC) -o hexconv hexconv.cpp

Ethernet.o: Ethernet.cpp
	$(CC) -I.. -I../../include -o $@ -c $<

Video.o: Video.cpp
	$(CC) -I.. -I../../include -o $@ -c $<

Ethernet: Ethernet.o
	mwldppc -xms -export pragma -nostdentry -nostdlib -o $@ $<

Video: Video.o
	mwldppc -xms -export pragma -nostdentry -nostdlib -o $@ $<
