package com.brazedblue.intervox.display;

import com.brazedblue.intervox.device.*;
import com.brazedblue.intervox.geometry.*;
import com.brazedblue.intervox.tracker.*;
import java.awt.*;
import java.util.*;
import javax.swing.*;

/**
 * View Object to display a Point with labels for each coordinate. Dimensions of coordinate system
 * determing by labels.length in SetLabels
 *
 * @version 1 6/4/98
 * @author Steve Meyer
 */
public class CoordinatesDisplay extends JComponent implements TrackerListener {
  Vector fSensor1Coordinates = new Vector();
  Vector fSensor2Coordinates = new Vector();
  public static final String[] kXYZLabel = {"X", "Y", "Z"};
  JPanel fCoordinatesPanel;
  JPanel fLabelsPanel;

  private static final int kDecimalPlaces = 1;

  /** Constructor */
  public CoordinatesDisplay() {

    setLayout(new FlowLayout());
    fLabelsPanel = new JPanel();
    fLabelsPanel.setLayout(new GridLayout(3, 1));
    add(fLabelsPanel);
    JLabel label = new JLabel("Receiver"); // blank space keeper
    fLabelsPanel.add(label);
    label = new JLabel("1");
    fLabelsPanel.add(label);
    label = new JLabel("2");
    fLabelsPanel.add(label);

    fCoordinatesPanel = new JPanel();
    fCoordinatesPanel.setLayout(new GridLayout(2, 3));
    add(fCoordinatesPanel);
  }

  /**
   * View Object to display a Point with labels for each coordinate. Dimensions of coordinate system
   * determing by labels.length in SetLabels
   *
   * @param labels, array which determines the labels, i.e. "x,y,etc..." and the number of
   *     coordinates
   */
  public void SetLabels(String[] labels) {

    fCoordinatesPanel.removeAll();

    fCoordinatesPanel.setLayout(new GridLayout(3, labels.length));
    FontMetrics fMetrics = getFontMetrics(getFont());
    Dimension size =
        new Dimension(labels.length * fMetrics.stringWidth("1.111111"), 3 * fMetrics.getHeight());
    fCoordinatesPanel.setMinimumSize(size);
    fCoordinatesPanel.setPreferredSize(size);

    size = new Dimension(fMetrics.stringWidth("Receiver"), 3 * fMetrics.getHeight());
    fLabelsPanel.setMinimumSize(size);
    fLabelsPanel.setPreferredSize(size);

    for (int i = 0; i < labels.length; i++) {
      JLabel descript = new JLabel(labels[i]);
      fCoordinatesPanel.add(descript);
    }

    for (int i = 0; i < labels.length; i++) {
      JLabel coord = new JLabel("0");
      fCoordinatesPanel.add(coord);
      fSensor1Coordinates.addElement(coord);
    }

    for (int i = 0; i < labels.length; i++) {
      JLabel coord = new JLabel("0");
      fCoordinatesPanel.add(coord);
      fSensor2Coordinates.addElement(coord);
    }

    fCoordinatesPanel.revalidate();
    fLabelsPanel.revalidate();
  }

  public Component GetComponent() {
    return this;
  }

  /**
   * View Object to display a Point with labels for each coordinate. Dimensions of coordinate system
   * determing by labels.length in SetLabels
   *
   * @param labels, array which determines the labels, i.e. "x,y,etc..." and the number of
   *     coordinates
   */
  void SetSensor1Coordinates(FloatPoint point) {
    for (int i = 0; i < 3 && i < fSensor1Coordinates.size(); i++) {
      JLabel coord = (JLabel) fSensor1Coordinates.elementAt(i);
      coord.setText(point.GetString(i, kDecimalPlaces));
    }
  }

  void SetSensor2Coordinates(FloatPoint point) {
    for (int i = 0; i < 3 && i < fSensor2Coordinates.size(); i++) {
      JLabel coord = (JLabel) fSensor2Coordinates.elementAt(i);
      coord.setText(point.GetString(i, kDecimalPlaces));
    }
  }

  public void DataRetrieved(DeviceOutput output) {
    Object point = output.GetDataOfType(DeviceOutput.POINT_COORDINATE);
    if (point != null) {
      if (output.GetStation() == DeviceOutput.STATION1) {
        SetSensor1Coordinates((FloatPoint) point);
      } else if (output.GetStation() == DeviceOutput.STATION2) {
        SetSensor2Coordinates((FloatPoint) point);
      }
    }
  }
}
