//
//  LayoutOptionsPane.java
//  neurosynch
//
//  Created by Steven Meyer on Tue Jun 15 2004.
//  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
//
package com.brazedblue.intervox.display;

import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.event.*;

import com.brazedblue.intervox.util.NeuroSynchUtil;

class LayoutOptionsPane extends JPanel {
  	private LayoutSelectionsModel	fLayoutSelections;
	
	private LayoutEditorPanel		fEditorPanel;
	private SelectLayoutDisplay		fSelectLayoutDisplay;
	
	LayoutOptionsPane(LayoutSelectionsModel layoutSelections)
	{
		SetLayoutSelections(layoutSelections);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	}
	
	void SetLayoutSelections(LayoutSelectionsModel layoutSelections)
	{
		fLayoutSelections = layoutSelections;
	}
	
	
	void SetupControls()
	{
		if (fEditorPanel == null)
		{
			fEditorPanel = new LayoutEditorPanel();
			JScrollPane scrollPane = new JScrollPane(fEditorPanel);
			add(scrollPane);
			fEditorPanel.SetLayoutSelections(fLayoutSelections);
			
			add(Box.createVerticalGlue());
			Box controlsbox = Box.createVerticalBox();
			add(controlsbox, BorderLayout.SOUTH);
			
			fSelectLayoutDisplay = new SelectLayoutDisplay();
			fSelectLayoutDisplay.SetLayoutSelections(fLayoutSelections);
			controlsbox.add(fSelectLayoutDisplay);
			fSelectLayoutDisplay.UpdateControls();
			
			Box bottomBtns = Box.createHorizontalBox();
			controlsbox.add(bottomBtns);
			
			JButton deleteBtn = new JButton("Delete");
			bottomBtns.add(deleteBtn);
			
			ActionListener l = new ActionListener()
			{
				public void actionPerformed(ActionEvent evt)
				{
					LayoutModel selectedLayout = fLayoutSelections.GetSelectedLayout();
					if (selectedLayout != null)
					{
						fLayoutSelections.DeleteLayout(selectedLayout);
					}
					else
					{
						NeuroSynchUtil.ErrorMessage("No Layout selected to Delete", NeuroSynchUtil.kShowDlog);
					}
				}
			};
			deleteBtn.addActionListener(l);
			
			JButton duplicateBtn = new JButton("Duplicate");
			bottomBtns.add(duplicateBtn);
			l = new ActionListener()
			{
				public void actionPerformed(ActionEvent evt)
				{
					LayoutModel selectedLayout = fLayoutSelections.GetSelectedLayout();
					if (selectedLayout != null)
					{
						LayoutModel newLayout = (LayoutModel)selectedLayout.clone();
						fLayoutSelections.AddLayout(newLayout);
						fLayoutSelections.SetSelectedLayout(newLayout);
					}
					else
					{
						NeuroSynchUtil.ErrorMessage("No Layout selected to Duplicate", NeuroSynchUtil.kShowDlog);
					}
				}
			};
			
			duplicateBtn.addActionListener(l);
		}
		else
		{
			fEditorPanel.SetLayoutSelections(fLayoutSelections);
			fSelectLayoutDisplay.SetLayoutSelections(fLayoutSelections);
			fSelectLayoutDisplay.UpdateControls();
		}
		
		
	}
	
	void Dispose()
	{
		if (fSelectLayoutDisplay != null)
		{
			fSelectLayoutDisplay.Dispose();
			fSelectLayoutDisplay = null;
		}
	}
	
}