package com.brazedblue.intervox.image;

import com.brazedblue.intervox.data.*;
import com.brazedblue.intervox.display.*;
import com.brazedblue.intervox.geometry.*;
import com.brazedblue.intervox.util.Debug;
import com.brazedblue.intervox.util.NeuroSynchUtil;
import com.brazedblue.intervox.view3D.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

public class RegionSelectorFrame extends NSInternalFrame implements ActionListener {

  RegionedImageSlicesSet fSlicesSet;
  HistogramSlider fSlider;

  Image3DView fImage3DView;
  RegionAdjustmentListener fAdjustmentListener;

  RegionCrossSectionView fCrossSectionView;
  ImageSeriesView fImageSeriesView;
  ImageSpaceSelector fImageSpaceSelector;

  JSlider fTransparencySlider;
  JButton fSelectColorButton;

  ThreeDMeshResolutionControls f3DMeshResolution;

  RegionSelectorFrameRangeValuesHandler fRangeValuesHandler;
  RegionSelectorFrameRegionsChoicesHandler fRegionsChoicesHandler;

  private static final int kTitleJustification = TitledBorder.DEFAULT_JUSTIFICATION;
  private static final int kTitlePosition = TitledBorder.TOP;

  private static final BevelBorder kBevelBorder = new BevelBorder(BevelBorder.RAISED);

  private static final String kDefaultRegion = null;

  private static final String kClearSeedPtsString = "Clear Seed Points";
  private static final String kSliceClearSeedPtsCmdString = kClearSeedPtsString;
  private static final String kAllSlicesClearSeedPtsCmdString = "All " + kClearSeedPtsString;

  private static final String kClearEdgeLinesString = "Clear Edge Lines";
  private static final String kSliceClearEdgeLinesCmdString = kClearEdgeLinesString;
  private static final String kAllSlicesClearEdgeLinesCmdString = "All " + kClearEdgeLinesString;

  private static final String kShow3DString = "Generate 3D";
  private static final String kSetColorString = "Set Color";

  private static final String kAddRegionVersionString = "Add Region Version";

  private static final String kRegionPrefixString = "Region ";

  public static final boolean kTrace = false;

  public RegionSelectorFrame(PatientData data) {
    super("3D Segmentation", true, true);

    setOpaque(true);

    fSlicesSet = new RegionedImageSlicesSet();
    fRangeValuesHandler =
        new RegionSelectorFrameRangeValuesHandler(
            data.GetImageFilterAccessor(), fSlicesSet.GetHistogram());

    Container contentPane = getContentPane();

    contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.Y_AXIS));
    JPanel horizontalPanel = new JPanel();
    horizontalPanel.setLayout(new BoxLayout(horizontalPanel, BoxLayout.X_AXIS));
    contentPane.add(horizontalPanel);

    InstallSliceRegionSelector(horizontalPanel, data);

    JPanel horizontalPanel2 = new JPanel();
    horizontalPanel2.setLayout(new FlowLayout(FlowLayout.LEFT));
    contentPane.add(horizontalPanel2);
    Install3DControls(horizontalPanel2, data);
  }

  private void InstallSliceRegionSelector(Container parent, PatientData data) {
    JPanel currentRegionPanel = new JPanel();
    currentRegionPanel.setLayout(new BoxLayout(currentRegionPanel, BoxLayout.Y_AXIS));
    currentRegionPanel.setBorder(kBevelBorder);
    parent.add(currentRegionPanel);
    fImageSpaceSelector = new ImageSpaceSelector();
    fImageSpaceSelector.SetImageSpacesAccessor(data.GetImageSpacesAccessor());
    currentRegionPanel.add(fImageSpaceSelector.GetControl());

    Border border =
        new TitledBorder(kBevelBorder, "Selection Range", kTitleJustification, kTitlePosition);
    fSlider = new HistogramSlider(fSlicesSet.GetHistogram(), border);

    int preferredWidth = fSlider.getPreferredSize().width;

    currentRegionPanel.add(fSlider);
    fSlider.addActionListener(this);

    // current slice button panel
    CreateCurrentSlicePanel(currentRegionPanel, preferredWidth);

    ImageViewType viewType =
        new ImageViewType(OrthoImageSetController.kAxial, 1, true, false, false, false);
    fImageSeriesView = new ImageSeriesView(viewType, fSlicesSet);

    fImageSeriesView.SetShowsImageSpaceSelect(false);
    fImageSeriesView.SetShowsSettings(false);

    fImageSeriesView.SetData(data);
    fImageSeriesView.setMaximumSize(fImageSeriesView.getPreferredSize());

    fAdjustmentListener = new RegionAdjustmentListener();
    JScrollBar seederViewScrollBar = fImageSeriesView.GetScrollbar();
    seederViewScrollBar.addAdjustmentListener(fAdjustmentListener);

    JComponent imageComponent = fImageSeriesView.GetImageComponent();
    imageComponent.addMouseListener(fAdjustmentListener);
    imageComponent.addMouseMotionListener(fAdjustmentListener);
    RegionSelectCrossSectionListener listener =
        new RegionSelectCrossSectionListener(seederViewScrollBar);

    fCrossSectionView = new RegionCrossSectionView(fSlicesSet);
    parent.add(fImageSeriesView);
    parent.add(fCrossSectionView);
    fCrossSectionView.addMouseListener(listener);

    seederViewScrollBar.setValue(0);

    RegionSelectorFrameSpaceChanger spaceChanger =
        new RegionSelectorFrameSpaceChanger(viewType, fImageSpaceSelector);

    addInternalFrameListener(new RegionSelectorFrameInternalFrameListener());
  }

  private void Install3DControls(Container parent, PatientData data) {
    JPanel threeDControlsPanel = new JPanel();
    threeDControlsPanel.setLayout(new BoxLayout(threeDControlsPanel, BoxLayout.Y_AXIS));
    threeDControlsPanel.setBorder(kBevelBorder);
    parent.add(threeDControlsPanel);
    fImage3DView = new Image3DView(1.0f, false);
    int preferredWidth = fImage3DView.getPreferredSize().width;

    threeDControlsPanel.setMaximumSize(fImage3DView.getPreferredSize());

    Create3DSettingsPanel(threeDControlsPanel, preferredWidth);

    // 3d button panel
    CreateRegionGenerationPanel(threeDControlsPanel, preferredWidth);

    // add region selection
    fRegionsChoicesHandler = new RegionSelectorFrameRegionsChoicesHandler();
    fRegionsChoicesHandler.SetData(data);
    fRegionsChoicesHandler.SetupChoicesPanel(threeDControlsPanel, preferredWidth);
    SetRegion(kDefaultRegion);

    fImage3DView.SetShowsImageSpaceSelect(false);
    fImage3DView.SetShowsSettings(false);
    fImage3DView.SetData(data);
    fImage3DView.setMaximumSize(fImage3DView.getPreferredSize());

    parent.add(fImage3DView);
  }

  public void actionPerformed(ActionEvent e) {
    if (kTrace) {
      System.out.println("RegionSelectorFrame.actionPerformed. event- " + e.toString());
    }

    String cmdString = e.getActionCommand();
    if (e.getSource() == fSlider) {
      fRangeValuesHandler.UpdateFromSlider();

    } else if (cmdString.equals(kSliceClearSeedPtsCmdString)) {
      fSlicesSet.ClearSeedPoints();

    } else if (cmdString.equals(kSliceClearEdgeLinesCmdString)) {
      fSlicesSet.ClearEdgeLines();

    } else if (cmdString.equals(kAllSlicesClearSeedPtsCmdString)) {
      fSlicesSet.ClearAllSeedPoints();

    } else if (cmdString.equals(kAllSlicesClearEdgeLinesCmdString)) {
      fSlicesSet.ClearAllEdgeLines();

    } else if (cmdString.equals(kShow3DString)) {
      fSlicesSet.CreateSelected3DGeometry();

      fImage3DView.HideAllRegions();
      fImage3DView.DisplayRegion(fSlicesSet.GetSeletedRegionLabel());
      fImage3DView.UpdateRegionsBtns();
      fImage3DView.revalidate();
      fImage3DView.repaint();
      f3DMeshResolution.GeometryCreated();
    } else if (cmdString.equals(kSetColorString)) {
      UpdateRegionColor();
    }
  }

  private void CreateAllSlicesPanel(Container parent, int width) {
    // add button panel
    JPanel btnPanel = new JPanel();
    btnPanel.setBorder(
        new TitledBorder(kBevelBorder, "All Slices", kTitleJustification, kTitlePosition));
    parent.add(btnPanel);
    btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.X_AXIS));
    JButton clearButton = new JButton(kClearSeedPtsString);
    clearButton.setActionCommand(kAllSlicesClearSeedPtsCmdString);

    btnPanel.add(clearButton);

    clearButton.addActionListener(this);

    JButton clearEdgesBtn = new JButton(kClearEdgeLinesString);
    clearEdgesBtn.setActionCommand(kAllSlicesClearEdgeLinesCmdString);
    btnPanel.add(clearEdgesBtn);

    clearEdgesBtn.addActionListener(this);
  }

  private void CreateCurrentSlicePanel(Container parent, int width) {
    // add button panel
    JPanel btnPanel = new JPanel();
    btnPanel.setBorder(
        new TitledBorder(kBevelBorder, "Current Slice", kTitleJustification, kTitlePosition));
    parent.add(btnPanel);
    btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.X_AXIS));
    JButton clearButton = new JButton(kClearSeedPtsString);
    clearButton.setActionCommand(kSliceClearSeedPtsCmdString);

    btnPanel.add(clearButton);

    clearButton.addActionListener(this);

    JButton clearEdgesBtn = new JButton(kClearEdgeLinesString);
    clearEdgesBtn.setActionCommand(kSliceClearEdgeLinesCmdString);
    btnPanel.add(clearEdgesBtn);

    clearEdgesBtn.addActionListener(this);
  }

  private void Create3DSettingsPanel(Container parent, int width) {
    // add button panel
    JPanel btnPanel = new JPanel();
    btnPanel.setBorder(
        new TitledBorder(kBevelBorder, "Region Settings", kTitleJustification, kTitlePosition));
    parent.add(btnPanel);
    btnPanel.setLayout(new BoxLayout(btnPanel, BoxLayout.Y_AXIS));
    fTransparencySlider = new MyJSlider();
    fTransparencySlider.setSize(10, 10); // 	this is needed
    fTransparencySlider.setPaintLabels(true);
    fTransparencySlider.setPaintTicks(true);

    fTransparencySlider.putClientProperty("JSlider.isFilled", Boolean.TRUE);

    fTransparencySlider.setMajorTickSpacing(fTransparencySlider.getMaximum());

    Dictionary labelTable = fTransparencySlider.getLabelTable();
    labelTable.put(
        new Integer(fTransparencySlider.getMinimum()), new JLabel("Transparent", JLabel.LEFT));
    labelTable.put(
        new Integer(fTransparencySlider.getMaximum()), new JLabel("Opaque", JLabel.RIGHT));
    fTransparencySlider.setLabelTable(labelTable);

    btnPanel.add(fTransparencySlider);

    MouseAdapter mouseListener =
        new MouseAdapter() {

          public void mouseReleased(MouseEvent e) {
            if (e.getSource() == fTransparencySlider) {
              UpdateRegionTransparency();
            }
          }
        };

    fTransparencySlider.addMouseListener(mouseListener);

    fSelectColorButton = new JButton(kSetColorString);
    btnPanel.add(fSelectColorButton);

    fSelectColorButton.addActionListener(this);

    f3DMeshResolution = new ThreeDMeshResolutionControls(parent);
  }

  private void Close() {
    fImageSpaceSelector.Dispose();

    if (fImage3DView != null) {
      fImage3DView.Dispose();
    }
  }

  private class ThreeDMeshResolutionControls implements ItemListener {
    JComboBox fResolutionCombo;
    String fDefaultLabel = "Default";
    StringBuffer fDefaultStrBuffer = new StringBuffer(fDefaultLabel);
    int fDefaultIndex = 0;
    boolean fUpdating = false;

    ThreeDMeshResolutionControls(Container parent) {
      JPanel horizontalPanel = new JPanel();
      horizontalPanel.setLayout(new BoxLayout(horizontalPanel, BoxLayout.X_AXIS));
      horizontalPanel.setBorder(kBevelBorder);

      parent.add(horizontalPanel);

      horizontalPanel.add(new JLabel("Mesh resolution"));
      String[] resStrings = {"1", "2", "3", "4", "5", "6"};

      fResolutionCombo = new JComboBox(new DefaultComboBoxModel(resStrings));
      fResolutionCombo.insertItemAt(fDefaultStrBuffer, fDefaultIndex);
      fResolutionCombo.setSelectedItem(fDefaultStrBuffer);
      horizontalPanel.add(fResolutionCombo);

      fResolutionCombo.addItemListener(this);
    }

    private void GeometryCreated() {
      // append actual resolution if default;
      if (fResolutionCombo.getSelectedIndex() == fDefaultIndex) {
        UpdateDefaultItem();
      }
    }

    private void UpdateDefaultItem() {
      if (!fUpdating) {
        fUpdating = true; // avoids recursive call to itemStateChanged
        Region3DModel region3D = fSlicesSet.GetCurrentRegion();
        int resolution = region3D.GetMeshResolution();

        String newLabel = fDefaultLabel + (resolution > 0 ? " (" + resolution + ")" : "");
        fDefaultStrBuffer.replace(0, fDefaultStrBuffer.length(), newLabel);
        fUpdating = false;
      }
    }

    public void itemStateChanged(ItemEvent e) {
      Region3DModel region3D = fSlicesSet.GetCurrentRegion();
      if (e.getStateChange() == ItemEvent.SELECTED) {
        if (fResolutionCombo.getSelectedIndex() == fDefaultIndex) {
          region3D.SetMeshResolution(0);
          UpdateDefaultItem();
        } else {
          String resString = (String) fResolutionCombo.getSelectedItem();
          int resolution = Integer.parseInt(resString);
          region3D.SetMeshResolution(resolution);
        }
      }
    }
  }

  private void CreateRegionGenerationPanel(Container parent, int width) {
    JPanel btnPanel = new JPanel();
    parent.add(btnPanel);

    JButton show3DButton = new JButton(kShow3DString);
    btnPanel.add(show3DButton);

    show3DButton.addActionListener(this);

    if (Debug.fgDebugging) {
      JButton debugOpenGL = new JButton("Debug OpenGL");
      btnPanel.add(debugOpenGL);
      ActionListener glListener =
          new ActionListener() {
            public void actionPerformed(ActionEvent e) {
              DebugTestOpenGL dlog = new DebugTestOpenGL(fImage3DView);
              dlog.setVisible(true);
            }
          };

      debugOpenGL.addActionListener(glListener);
    }
  }

  private void UpdateRegionColor() {
    Color oldColor = fSlicesSet.GetCurrentGeometryColor();

    Color newColor = JColorChooser.showDialog(this, "Choose a Color", oldColor);

    if (newColor != null && !newColor.equals(oldColor)) {
      fSlicesSet.SetCurrentGeometryColor(newColor);
      fSelectColorButton.setBackground(newColor);
      if (fImage3DView != null) {
        fImage3DView.UpdateImageFrom3DView();
      }
    }
  }

  private void UpdateRegionTransparency() {
    float transparency =
        (float) fTransparencySlider.getValue()
            / (float) (fTransparencySlider.getMaximum() - fTransparencySlider.getMinimum());

    if (transparency != fSlicesSet.GetCurrentGeometryTransparency()) {
      fSlicesSet.SetCurrentGeometryTransparency(transparency);
      if (fImage3DView != null) {
        fImage3DView.UpdateImageFrom3DView();
      }
    }
  }

  private void UpdateSliderHighLow() {
    fSlider.SetLowValue(fSlicesSet.GetCurrentRangeLow());
    fSlider.SetHighValue(fSlicesSet.GetCurrentRangeHigh());

    fSlider.repaint();
  }

  private void UpdateRegionSettingsButtons() {
    float transparency = fSlicesSet.GetCurrentGeometryTransparency();
    int sliderValue = Math.round(transparency * fTransparencySlider.getMaximum());
    fTransparencySlider.setValue(sliderValue);

    Color color = fSlicesSet.GetCurrentGeometryColor();
    fSelectColorButton.setBackground(color);
  }

  private void SetRegion(String region) {
    fSlicesSet.SetCurrentRegion(region);
    UpdateSliderHighLow();
    UpdateRegionSettingsButtons();
  }

  class RegionSelectCrossSectionListener extends MouseAdapter {
    JScrollBar fSeederViewScrollbar;

    RegionSelectCrossSectionListener(JScrollBar seederViewScrollBar) {
      fSeederViewScrollbar = seederViewScrollBar;
    }

    public void mouseClicked(MouseEvent e) {
      if (kTrace) {
        System.out.println("RegionSelectCrossSectionListener.mouseClicked. event- " + e.toString());
      }

      int slice = fCrossSectionView.GetClickToSlice(e.getX(), e.getY());
      fSeederViewScrollbar.setValue(slice);
    }
  }

  class RegionAdjustmentListener extends MouseAdapter
      implements AdjustmentListener, MouseMotionListener {
    boolean fSelectingBoundary = false;
    Point fMousePressedPoint = null;

    RegionAdjustmentListener() {}

    public void adjustmentValueChanged(AdjustmentEvent e) {
      if (kTrace) {
        System.out.println(
            "RegionAdjustmentListener.adjustmentValueChanged. event- " + e.toString());
      }

      fSlicesSet.SetToDepth(e.getValue());
      fCrossSectionView.SetCurrentSlice(e.getValue());
      if (fSlider != null) {
        fSlider.repaint(); // 	update to new histogram
      }
    }

    public void mousePressed(MouseEvent e) {
      fMousePressedPoint = new Point(e.getX(), e.getY());
    }

    public void mouseClicked(MouseEvent e) {
      if (kTrace) {
        System.out.println("RegionAdjustmentListener.mouseClicked. event- " + e.toString());
      }

      //	need to make sure e is in the image
      fSlicesSet.SeedPoint(e.getX(), e.getY());
      fSelectingBoundary = false;
    }

    public void mouseDragged(MouseEvent e) {
      fSelectingBoundary = true;

      if (fMousePressedPoint != null) // mouse dragged doesn'g pass the first point
      {
        fSlicesSet.AddBoundaryPoint(fMousePressedPoint.x, fMousePressedPoint.y);
        fMousePressedPoint = null;
      }

      fSlicesSet.AddBoundaryPoint(e.getX(), e.getY());
    }

    public void mouseMoved(MouseEvent e) {}

    public void mouseReleased(MouseEvent e) {
      if (fSelectingBoundary) {
        fSlicesSet.BoundaryPointsAdded();
      }

      fSelectingBoundary = false;
      fMousePressedPoint = null;
    }
  }

  private class RegionSelectorFrameSpaceChanger
      implements ImageSpaceSelector.ImageSpaceSelectListener {
    ImageViewType fImageViewType;

    private RegionSelectorFrameSpaceChanger(
        ImageViewType imageViewType, ImageSpaceSelector spaceSelector) {
      fImageViewType = imageViewType;
      spaceSelector.AddSpaceSelectListener(this);
    }

    public void SpaceSelected(ImageSpaceSelector.SelectEvent event) {
      if (event.GetSelected()) {
        SingleSpaceID spaceID = event.GetSpaceID();

        fRegionsChoicesHandler.SetSpaceID(spaceID);

        fCrossSectionView.FlushImage();
        fCrossSectionView.invalidate();
        fImageViewType.SetSpaceID(spaceID);
        fImage3DView.SetSpaceID(spaceID);
        fImageSeriesView.ImageViewTypeChanged();
        fRangeValuesHandler.SetSpaceID(spaceID);
        UpdateSliderHighLow();
        UpdateRegionSettingsButtons();
        pack();
      }
    }
  }

  private class RegionSelectorFrameRangeValuesHandler {
    ImageFilterAccessor fImageFilter;

    Histogram fHistogram;
    int fLow;
    int fHigh;

    private RegionSelectorFrameRangeValuesHandler(
        ImageFilterAccessor filterAccessor, Histogram histogram) {
      fImageFilter = filterAccessor;
      fHistogram = histogram;

      SetImageAdjustments(
          fImageFilter.GetImageAdjustments(SingleSpaceID.GetDefaultVirtualSpaceID()));
    }

    private void SetSpaceID(SingleSpaceID spaceID) {
      SetImageAdjustments(fImageFilter.GetImageAdjustments(spaceID));
    }

    private void SetImageAdjustments(ImageAdjustments imageAdjustments) {
      fLow = imageAdjustments.GetAdjustedMinValue();
      fHigh = imageAdjustments.GetAdjustedMaxValue();

      fHistogram.SetLowEntry(fLow);
      fHistogram.SetNumOfEntries(fHigh - fLow);
    }

    private void UpdateFromSlider() {
      int low = fSlider.GetLowValue();
      int high = fSlider.GetHighValue();

      //	include whole range at limits
      if (low <= fLow) {
        low = -1;
      }
      if (high >= fHigh) {
        high = Integer.MAX_VALUE;
      }

      fSlicesSet.SetGrayScaleRange(low, high, !fSlider.IsAdjusting());
    }
  }

  //	trying to free up references for garbage collection
  class RegionSelectorFrameInternalFrameListener extends InternalFrameAdapter {
    RegionSelectorFrameInternalFrameListener() {}

    public void internalFrameClosed(InternalFrameEvent e) {
      Close();
    }
  }

  private class RegionSelectorFrameRegionsChoicesHandler implements ActionListener {
    private JPanel fRegionsPanel;
    private RegionsOfInterest fRegions;
    private ButtonGroup fButtonGroup;
    private PatientData fData;
    private Vector fButtons = new Vector();

    private RegionSelectorFrameRegionsChoicesHandler() {}

    private void SetupChoicesPanel(Container parent, int width) {
      JPanel regionsChoicesPanel = new JPanel();
      regionsChoicesPanel.setLayout(new BoxLayout(regionsChoicesPanel, BoxLayout.Y_AXIS));
      regionsChoicesPanel.setBorder(
          new TitledBorder(kBevelBorder, "Selected Region", kTitleJustification, kTitlePosition));
      fRegionsPanel = new JPanel();
      fRegionsPanel.setLayout(new BoxLayout(fRegionsPanel, BoxLayout.X_AXIS));
      fRegionsPanel.setBorder(kBevelBorder);
      regionsChoicesPanel.add(fRegionsPanel);

      fButtonGroup = new ButtonGroup();

      UpdateChoicesButtons();

      JButton addVersionBtn = new JButton(kAddRegionVersionString);
      regionsChoicesPanel.add(addVersionBtn);

      addVersionBtn.addActionListener(this);

      parent.add(regionsChoicesPanel);
    }

    private void UpdateChoicesButtons() {
      Enumeration iter = fButtons.elements();

      while (iter.hasMoreElements()) {
        AbstractButton button = (AbstractButton) iter.nextElement();
        fRegionsPanel.remove(button);
        fButtonGroup.remove(button);
        button.removeActionListener(this);
      }

      fButtons.removeAllElements();

      String[] labels = fRegions.GetRegionsLabels();
      for (int i = 0; i < labels.length; i++) {
        String actionCmd = kRegionPrefixString.concat(labels[i]);
        // label.concat
        JRadioButton button = new JRadioButton(labels[i], i == 0);
        button.setActionCommand(actionCmd);

        fButtons.addElement(button);
        fButtonGroup.add(button);
        fRegionsPanel.add(button);
        button.addActionListener(this);
      }

      revalidate();
      //			repaint();
    }

    public void actionPerformed(ActionEvent e) {
      String cmdString = e.getActionCommand();
      if (NeuroSynchUtil.SubstringMatches(cmdString, kRegionPrefixString, 0)) {
        String region = cmdString.substring(kRegionPrefixString.length());

        SetRegion(region);
      } else if (cmdString.equals(kAddRegionVersionString)) {
        fSlicesSet.CreateRegionVersion();
        UpdateChoicesButtons();
        fImage3DView.UpdateRegionsBtns();
      }
    }

    private void SetData(PatientData data) {
      fData = data;
      fRegions = fData.GetRegionsOfInterest(SingleSpaceID.GetDefaultVirtualSpaceID());
    }

    private void SetSpaceID(SingleSpaceID spaceID) {
      fRegions = fData.GetRegionsOfInterest(spaceID);
      UpdateChoicesButtons();
    }
  }
}

//	this fixes a bug in which the sliderUI is not notified of the first component resizing
//	bug in Component.reshape
class MyJSlider extends JSlider {
  boolean resizeEventInvoked = false;

  MyJSlider() {}

  public void setBounds(int x, int y, int width, int height) {
    boolean resized = width != getWidth() && height != getHeight();

    super.setBounds(x, y, width, height);

    if (resized) {
      ComponentEvent e = new ComponentEvent(this, ComponentEvent.COMPONENT_RESIZED);
      updateUI();
      resizeEventInvoked = true;
    }
  }
}
