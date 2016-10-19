
test:
	mkdir /tmp/zfsTest
	dd bs=1M count=512 if=/dev/zero of=/tmp/zfsTest/disk1
	dd bs=1M count=512 if=/dev/zero of=/tmp/zfsTest/disk2
	sudo zpool create testPoolWith /tmp/zfsTest/disk1
	sudo zpool create testPoolWithout /tmp/zfsTest/disk2

	sudo zfs list -r testPoolWithout
	sudo zfs list -r testPoolWith

	sudo zfs set dedup=on testPoolWith
	sudo zfs get -r dedup testPoolWith
	sudo zfs get -r dedup testPoolWithout

	python createFile.py

	sudo time cp ./foo.txt /testPoolWithout/foo0.txt	
	sudo time cp ./foo.txt /testPoolWith/foo0.txt
	sudo time cp ./foo.txt /testPoolWithout/foo1.txt	
	sudo time cp ./foo.txt /testPoolWith/foo1.txt

	sudo zpool get dedupratio testPoolWithout
	sudo zfs list -r testPoolWithout
	sudo zpool get dedupratio testPoolWith
	sudo zfs list -r testPoolWith

	sudo zdb -b testPoolWith

clean:
	sudo zpool destroy testPoolWith
	sudo zpool destroy testPoolWithout
	sudo rm -rf /tmp/zfsTest
	rm -f ./foo.txt

diskclean:
	sudo rm -rf /tmp/zfsTest
