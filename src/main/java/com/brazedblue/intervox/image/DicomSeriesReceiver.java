package com.brazedblue.intervox.image;


		// encountering folders with multiple series of same study.  Convenient to save only the
		// largest
public interface DicomSeriesReceiver extends DicomImageReceiver {
	
	public int SeriesAddedCount();
	
	public void SaveLargestSeriesOnly();
}
