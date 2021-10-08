package com.brazedblue.intervox.geometry;

import java.awt.*;
import java.util.*;

import com.brazedblue.intervox.image.*;
import com.brazedblue.intervox.util.*;


public class RegionCrossSectAccessor extends RegionSliceAccessor {
 	
	

  	public RegionCrossSectAccessor() 
   	{
    }
	
	public void SetRegions(RegionsOfInterest regions)
	{
		super.SetRegions(regions);
		
		SetUpSlice();
		UpdateMarks();
	}
	
	private void SetUpSlice()
	{
		int numOfSlices = fRegions.GetNumOfSlices();
		Dimension sliceSize = fRegions.GetSliceDimensions();
		
		fSlice = new RegionSlice(sliceSize.height, numOfSlices);
/*		
		for (int whichslice = 0; whichslice < numOfSlices; whichslice++)
		{
			RegionSlice slice = fRegions.GetRegionSlice(whichslice);
			
			if (slice.HasRegion())
			{
				int height = slice.GetHeight();
				int width = slice.GetWidth();
				
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int mark = slice.GetMark(x, y);
						
						int myMark = fSlice.GetMark(y, whichslice);
						fSlice.MarkPixel(y, whichslice, mark | myMark);
					}
				}
			}
		} */
	}
	
	public void UpdateMarks()
	{
		if (RegionSelectorFrame.kTrace)
		{
			System.out.println("RegionCrossSectAccessor.UpdateMarks");
		}

		int numOfSlices = fRegions.GetNumOfSlices();
		Dimension sliceSize = fRegions.GetSliceDimensions();
		int xValue = fSliceIndex; // sliceSize.width / 2;
		for (int whichslice = 0; whichslice < numOfSlices; whichslice++)
		{
			RegionSlice slice = fRegions.GetRegionSlice(whichslice);
			
			if (slice.HasRegion())
			{
				for (int y = 0; y < sliceSize.height; y++)
				{
					int mark = slice.GetMark(xValue, y);
					
					int myMark = fSlice.GetMark(y, whichslice);
					fSlice.MarkPixel(y, whichslice, mark | myMark);
				}
			}
		} 
	}
	
	public void ClearMarks(int whichSlice)
	{
		if (RegionSelectorFrame.kTrace)
		{
			System.out.println("RegionCrossSectAccessor.ClearMarks");
		}
		
	 	int width = fSlice.GetWidth();
		
		for (int x = 0; x < width; x++)
		{
			UnMarkPixel(x, whichSlice);		
		}
	}
	
	public void ClearAllMarks()
	{
		if (RegionSelectorFrame.kTrace)
		{
			System.out.println("RegionCrossSectAccessor.ClearAllMarks");
		}
		
		fSlice.UnMark(fRegionValue);
	}
	
	public void SetPixelData(PixelData pixels, int coord)
	{
		fPixels = pixels;
		fSliceIndex = coord;
	}


/*	
	public void UpdateRGBArray(int[] rgbArray)
	{
		int height = fPixels.GetHeight();
		int width = fPixels.GetWidth();
		
		if (Debug.fgDebugging && (height * width != rgbArray.length) )
		{
			NeuroSynchUtil.ErrorMessage("height and width don't match in RegionSliceAccessor.UpdateRGBArray(rgbArray)", 
					NeuroSynchUtil.kNoDlog);
			return;
		}
		
		int i = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++, i++)
			{
				boolean marked = false;
				if (fDisplayRegion)
				{
					int mark = fSlice.GetMark(x, y);
					if ((mark & fRegionValue) != 0)
					{
						rgbArray[i] = fRegionColor;
						marked = true;
					}
					
				}

				if (!marked)
				{
					int pixel = fPixels.GetPixelValue(x, y);
					
					if (pixel <= fHighValue && pixel >= fLowValue)
					{
						rgbArray[i] = fRangeColor;
					}
				}
			}
		
		}
	} */
}