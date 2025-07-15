package com.brazedblue.intervox.file;

public class FileLoaderThread extends Thread {

	public FileLoaderThread() {
	}

	public void MyYield() {
		Thread.yield();
		try {
			sleep(50); // I need this because sometimes the yield doesn't work
		} catch (InterruptedException e) {
		}
	}

}
