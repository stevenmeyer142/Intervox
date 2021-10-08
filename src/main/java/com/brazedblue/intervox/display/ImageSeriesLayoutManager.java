package com.brazedblue.intervox.display;

import javax.swing.JComponent;
import java.util.Vector;
import com.brazedblue.intervox.image.NSImageView;


public class ImageSeriesLayoutManager implements LayoutListener
{
	protected NSImageView[]				fImageViews = null;
	protected LayoutSelectionsModel		fLayoutSelections;
	protected LayoutModel				fCurrentLayout;
	protected JComponent					fContainer;
	
	public ImageSeriesLayoutManager(JComponent container)
	{
		fContainer = container;
	}
	
	public void SetLayoutSelections(LayoutSelectionsModel layoutSelections)
	{
		if (fLayoutSelections != null)
		{
			fLayoutSelections.RemoveLayoutListener(this);
		}
		
		fLayoutSelections = layoutSelections;
		fLayoutSelections.AddLayoutListener(this);
		
		UpdateCurrentLayout();
	}
	
	private void UpdateCurrentLayout()
	{
		LayoutModel newLayout = fLayoutSelections.GetSelectedLayout();
		
		if (newLayout != fCurrentLayout)
		{
			if (fCurrentLayout != null)
			{
				fCurrentLayout.RemoveLayoutListener(this);
				fCurrentLayout.DisposeViews();
			}
			
			fCurrentLayout = newLayout;
			
			if (fCurrentLayout != null)
			{
				fCurrentLayout.AddLayoutListener(this);
			}
		}
		
	}
	
	public void ProcessLayoutEvent(LayoutEvent event)
	{
		UpdateCurrentLayout();
		SetupImageViews();
	}
	
	public void Dispose()
	{
		if (fLayoutSelections != null)
		{
			fLayoutSelections.RemoveLayoutListener(this);
			fLayoutSelections = null;
		}
		
		if (fCurrentLayout != null)
		{
			fCurrentLayout.RemoveLayoutListener(this);
			fCurrentLayout = null;
		}
	}
	
	public void SetupImageViews()
	{
		fContainer.repaint();	// force erase of current area
		for (int i = 0; fImageViews != null && i < fImageViews.length; i++)
		{
			if (fImageViews[i] != null)
			{
				fContainer.remove(fImageViews[i].GetComponent()) ;
				fImageViews[i].Dispose();
			}
		}
		
		fImageViews = null;
		LayoutModel layout = GetLayout();
		if (layout != null)
		{
			Vector viewList = new Vector();
			layout.CreateViews(fContainer, viewList);
			fImageViews = new NSImageView[viewList.size()];
			
			for (int i = 0; i < viewList.size(); i++)
			{
				NSImageView view = (NSImageView)viewList.elementAt(i);
				fImageViews[i] = view;
				PostProcessView(view);
//				fImageViews[i].SetData(fData);
//				f3DImageController.AddImageView(view);
				//				fImageCopier.AddImageCopier(view.GetImageCopier());
			}
		}
		
		
//		f3DImageController.UpdateSeriesViews();
//		fContainer.invalidate();
		fContainer.revalidate();
	}

	protected void PostProcessView(NSImageView view)
	{
		ImageControlSettings controlSettings = view.GetImageControlSettings();
		if (fCurrentLayout != null && controlSettings != null)
		{
			controlSettings.SetLayout(fCurrentLayout);
		}
	}
	
	
	public LayoutModel GetLayout()
	{
		return fLayoutSelections != null ? fLayoutSelections.GetSelectedLayout() : null;
	}
}
