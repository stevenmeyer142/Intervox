package com.brazedblue.intervox.display;

import java.beans.PropertyChangeSupport;
import java.awt.Color;

import com.brazedblue.intervox.image.*;
import com.brazedblue.intervox.data.*;
import com.brazedblue.intervox.util.Debug;

public class ImageViewType implements java.io.Serializable, java.lang.Cloneable {
  
  	private int							fAxis;
	private float						fMagnification;
	private boolean 					fHasScrollbar;
	private boolean 					fHasPopup;
	private boolean 					fDisplaysFiducials;
	private boolean 					fTracks;
	private int							fFeatures;
	private SingleSpaceID				fWhichSpace = SingleSpaceID.GetDefaultVirtualSpaceID();
	private PropertyChangeSupport		fImageViewTypeListeners ;
	
	
	public static final int 		kNoFeatures = 0;
	public static final int 		kTracksInterceptFeature = 1;
	public static final String		kSpaceChanged = "ImageViewType_SpaceChanged";
	public static final	String 		kMagnificationProperty = "ImageViewType_Magnification";
	public static final	String 		kAxisProperty = "ImageViewType_Axis";
	
	public static final String		kSagitalPerspective = "Sagital";
	public static final String		kCoronalPerspective = "Coronal";
	public static final String		kAxialPerspective = "Axial";
	public static final String		kProbeTrajectoryPerspective = "Probe Trajectory";
	public static final String		k3DPerspective = "3D";
	public static final String[] 	kPerspectiveChoices = {kSagitalPerspective,
											kCoronalPerspective, kAxialPerspective, kProbeTrajectoryPerspective,
											k3DPerspective};
	private static final Color		kSagitalColor = new Color(67, 155, 67);
	private static final Color		kAxialColor = new Color(109, 109, 213);
	private static final Color		kCoronalColor = new Color(237, 186, 151);
	private static final Color		k3DColor = new Color(189, 112, 84);
	private static final Color		kTrajectoryColor = new Color(155, 92, 151);
	
	private static final String		k50PercentMagnification = "50%";
	private static final String		k100PercentMagnification = "100%";
	private static final String		k150PercentMagnification = "150%";
	private static final String 	k200PercentMagnification = "200%";
	public static final String[] 	kMagnificationChoices = {k50PercentMagnification, 
					k100PercentMagnification, k150PercentMagnification, k200PercentMagnification};

	static final long			serialVersionUID = -4783516988435056267L;

    public ImageViewType(int axis, float magnification, boolean hasScrollbar, boolean hasPopup,
							boolean displaysFiducials, boolean tracks) 
	{
		this(axis, magnification, hasScrollbar, hasPopup, displaysFiducials, tracks, kNoFeatures);
	}
	
   	public ImageViewType(int axis, float magnification, boolean hasScrollbar, boolean hasPopup,
							boolean displaysFiducials, boolean tracks, int features) 
	{
		fAxis = axis;
		fMagnification = magnification;
		fHasScrollbar = hasScrollbar;
		fDisplaysFiducials = displaysFiducials;
		fTracks = tracks;
		fFeatures = features;
		fHasPopup = hasPopup;
		fImageViewTypeListeners = new PropertyChangeSupport(this);
    }
	
	public String GetPerspectiveString()
	{
		String result = "";
		switch (fAxis)
		{
			case OrthoImageSetController.kSagittal:
				result = kSagitalPerspective;
				break;
			case OrthoImageSetController.kCoronal:
				result = kCoronalPerspective;
				break;
			case OrthoImageSetController.kAxial:
				result = kAxialPerspective;
				break;
			case OrthoImageSetController.kTrajectory:
				result = kProbeTrajectoryPerspective;
				break;
			case OrthoImageSetController.k3D:
				result = k3DPerspective;
				break;
		}
		return result;
	}
	
	public boolean GetAutoScrolls()
	{
		return !HasFeature(kTracksInterceptFeature);
	}
	
	public void SetTracks(boolean tracks)
	{
		fTracks = tracks;
	
	}
	
	public String GetIDText()
	{
		String result = OrthoImageSetController.GetAxisIDString(GetAxis());
			
		if (HasFeature(ImageViewType.kTracksInterceptFeature))
		{
			result += "(I)";
		}
		
		return result;
	}
	
	public void SetAutoScrolls(boolean autoScrolls)
	{
		if (!autoScrolls && (fAxis == OrthoImageSetController.kAxial
				|| fAxis == OrthoImageSetController.kSagittal || fAxis == OrthoImageSetController.kCoronal ))
		{
			fFeatures |= kTracksInterceptFeature;
		}
		else
		{
			if ((fFeatures & kTracksInterceptFeature) != 0)
			{
				fFeatures -= kTracksInterceptFeature;
			}
		}
	}
	
	public Color GetColor()
	{
		Color result = Color.lightGray;
		switch (fAxis)
		{
			case OrthoImageSetController.kCoronal :
				result = kCoronalColor;
				break;
			
			case OrthoImageSetController.kSagittal :
				result = kSagitalColor;
				break;
			
			case OrthoImageSetController.kAxial :
				result = kAxialColor;
				break;
			
			case OrthoImageSetController.kTrajectory :
				result = kTrajectoryColor;
				break;
			
			case OrthoImageSetController.k3D :
				result = k3DColor;
				break;
		}
		
		return result;
	}
	
	public static ImageViewType NewDefaultViewtype(String perspective)
	{
		ImageViewType result = null;
		if (perspective.equals(k3DPerspective))
		{
			result = new Image3DViewType(1.0f);
		}
		else
		{
			int axis = OrthoImageSetController.kAxial;
			float magnification = 1.0f; 
			boolean hasScrollbar = true; 
			boolean hasPopup = true;  
			boolean displaysFiducials = true; 
			boolean tracks = true; 
			
			if (perspective.equals(kProbeTrajectoryPerspective))
			{
				displaysFiducials = false;
				hasScrollbar = false;
				displaysFiducials = false;
			}
			
			result = new ImageViewType(axis, magnification, hasScrollbar, hasPopup, displaysFiducials, tracks);
			result.SetPerspectiveString(perspective);
		}
		
		return result;
	}
	
	public synchronized Object clone() 
	{
		if (Debug.fgDebugging && false)
		{
			Debug.PrintStackTrace("ImageViewType.clone called");
		}
		
		try 
		{ 
		    Object result = super.clone();
			
			fImageViewTypeListeners = new PropertyChangeSupport(this);

			
		    return result;
		} 
		catch (CloneNotSupportedException e) 
		{ 
		    // this shouldn't happen, since we are Cloneable
		    throw new InternalError();
		}
    }

	
	public int GetAxis()
	{
		return fAxis;
	}
	
	public boolean HasFeature(int feature)
	{
		return (fFeatures & feature) != 0;
	}
	
	private void FireChange(String propertyName,  Object oldValue, Object newValue)
	{
		fImageViewTypeListeners.firePropertyChange(propertyName, oldValue, newValue);
	}
	
	public void AddImageViewTypeListener(ImageViewTypeListener listener)
	{
		fImageViewTypeListeners.addPropertyChangeListener(listener);
	}
	
	public void RemoveImageViewTypeListener(ImageViewTypeListener listener)
	{
		fImageViewTypeListeners.removePropertyChangeListener(listener);
	}
	
	public float GetMagnification()
	{
		return fMagnification;
	}
	
	public String GetMagnificationString()
	{
		String result = null;
		if (fMagnification == 0.5f)
		{
			result = k50PercentMagnification;
		}
		else if (fMagnification == 1.0f)
		{
			result = k100PercentMagnification;
		}
		else if (fMagnification == 1.5f)
		{
			result = k150PercentMagnification;
		}
		else if (fMagnification == 2.0f)
		{
			result = k200PercentMagnification;
		}
		return result;
	}
	
	public boolean DisplaysFiducials()
	{
		return fDisplaysFiducials;
	}
	
	public boolean HasPopup()
	{
		return fHasPopup;
	}
	
		public SingleSpaceID GetSingleSpaceID()
	{
		return fWhichSpace;
	}
	

	
	public boolean HasScrollbar()
	{
		return fHasScrollbar;
	}
	
	public boolean Tracks()
	{
		return fTracks;
	}
	
	public NSImageView CreateView()
	{
/*		if (HasFeature(kTracksInterceptFeature))
		{
			return new ImageInterceptView(this);
		}
		else
		{
			return new ImageSeriesView(this);
		} */

		return new ImageSeriesView(this);
	}
	
	public void SetPerspectiveString(String perspective)
	{
		int newAxis = fAxis;
		if (kAxialPerspective.equals(perspective))
		{
			newAxis = OrthoImageSetController.kAxial;
		}
		else if (k3DPerspective.equals(perspective))
		{
			newAxis = OrthoImageSetController.k3D;
		}
		else if (kCoronalPerspective.equals(perspective))
		{
			newAxis = OrthoImageSetController.kCoronal;
		}
		else if (kProbeTrajectoryPerspective.equals(perspective))
		{
			newAxis = OrthoImageSetController.kTrajectory;
		}

		else if (kSagitalPerspective.equals(perspective))
		{
			newAxis = OrthoImageSetController.kSagittal;
		}
		
		if (fAxis != newAxis)
		{
	//		Int oldAxis = new Int(fAxis);
	//		Int newAxisObj = new Int(newAxis);
			fAxis = newAxis;
	//		FireChange(kAxisProperty, oldAxis, newAxisObj);
		}
		
	}

	public void SetMagnificationString(String magnificationStr)
	{
		float newMagnification = fMagnification;
		if (k50PercentMagnification.equals(magnificationStr))
		{
			newMagnification = .5f;
		}
		else if (k100PercentMagnification.equals(magnificationStr))
		{
			newMagnification = 1.0f;
		}
		else if (k150PercentMagnification.equals(magnificationStr))
		{
			newMagnification = 1.5f;
		}
		else if (k200PercentMagnification.equals(magnificationStr))
		{
			newMagnification = 2.0f;
		}
		
		if (newMagnification != fMagnification)
		{
			fMagnification = newMagnification;
		}
	}
	
	public static ImageViewType NewImageViewType(String axisString, boolean scrolls, boolean tracks, 
				String magnificationString, ImageViewType oldViewType)
	{
		ImageViewType result = NewDefaultViewtype(axisString);

		result.SetMagnificationString(magnificationString);
		result.SetSpaceID(oldViewType.GetSingleSpaceID());

		if (!axisString.equals(k3DPerspective))
		{
			result.SetAutoScrolls(scrolls);
			result.SetTracks(tracks);
		}
		
		return result;
	}
	
	public void SetSpaceID(SingleSpaceID spaceID)
	{
		if (!fWhichSpace.equals(spaceID))
		{
			SingleSpaceID oldSpace = fWhichSpace;
			fWhichSpace = spaceID;
			
			FireChange(kSpaceChanged, oldSpace, fWhichSpace);
		}
	}
  }

