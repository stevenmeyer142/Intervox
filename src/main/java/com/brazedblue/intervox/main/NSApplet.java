package com.brazedblue.intervox.main;

import javax.swing.*;

import com.brazedblue.intervox.util.NeuroSynchUtil;

import java.awt.*;
import java.awt.event.*;

/*
	FramelessApplet java
	Main applet file for Frameless Visualizer
	Started 6/3/98 Using jdk 1.1.5
	Developed by Steve Meyer for Dr. Werner Doyle
	Copyright 1998 Werner Doyle
	All rights reserved
*/

public class NSApplet extends JApplet 
{
	
	Frame GetFrame()
	{
		return NeuroSynchUtil.GetFrame(this);
	}
	

}