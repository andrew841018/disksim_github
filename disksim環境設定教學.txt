1. 環境:
	ubuntu18 64bits
	gcc-4.8
	g++-4.8
	make
	flex
	bison

2. 改 gcc 預設版本:
	sudo apt install gcc-4.8 gcc-4.8-multilib g++-4.8 g++-4.8-multilib

	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8

	使用 gcc -v 確認預設版本



3. 閱讀 disksim with ssdmodel.pdf


4. make完成後閱讀 disksim with ssdmodel 測試.PNG




Reference:
http://cighao.com/2016/03/23/disksim-with-ssdmodel-source-analysis-013-use-it-on-64bit/
https://blog.csdn.net/FontThrone/article/details/104279224
https://www.luoow.com/dc_tw/102894220



Arthur Chang 張丞嘉