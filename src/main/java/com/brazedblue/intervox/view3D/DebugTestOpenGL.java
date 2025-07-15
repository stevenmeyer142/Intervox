//
//  DebugTestOpenGL.java
//  neurosynch
//
//  Created by Steven Meyer on Wed Feb 26 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

package com.brazedblue.intervox.view3D;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;

public class DebugTestOpenGL extends JDialog {
  String[] rowNames = {
    "Light Position",
    "Light Specular",
    "Light ambient",
    "Light Diffuse",
    "Material Specular",
    "Material ambient",
    "Material diffuse",
    "Material shininess"
  };
  Object[][] values = new Object[rowNames.length][5];
  View3D fView3D;
  Image3DView fImageView3D;

  public DebugTestOpenGL(Image3DView image3DView) {
    fImageView3D = image3DView;
    fView3D = fImageView3D.DebugGetView3D();
    float glLighting[][] = new float[values.length][values[0].length];

    fView3D.DebugGetOpenGLLighting(glLighting);
    for (int i = 0; i < values.length; i++) {
      values[i][0] = rowNames[i];
      {
        for (int j = 1; j < values[i].length; j++) {
          if ((i > 0 && j < 2) || i == 0) {
            values[i][j] = new Float(glLighting[i][j - 1]);
          } else {
            values[i][j] = new Float(0); // 	 unused
          }
        }
      }
    }
    Container contentPane = getContentPane();
    JPanel contentPanel = new JPanel();
    contentPanel.setLayout(new BoxLayout(contentPanel, BoxLayout.Y_AXIS));
    contentPane.add(contentPanel);
    // Create a model of the data.
    TableModel dataModel =
        new AbstractTableModel() {
          public int getColumnCount() {
            return values[0].length;
          }

          public int getRowCount() {
            return values.length;
          }

          public Object getValueAt(int row, int col) {
            return values[row][col];
          }

          //         public String getColumnName(int column) {return rowNames[column];}
          public Class getColumnClass(int c) {
            return values[0][c].getClass();
          }

          public boolean isCellEditable(int row, int col) {
            return col != 0;
          }

          public void setValueAt(Object aValue, int row, int column) {
            values[row][column] = (Float) aValue;
          }
        };

    JTable table = new JTable(dataModel);
    contentPanel.add(table);

    JButton applyButton = new JButton("Apply");

    ActionListener l =
        new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            float glLighting[][] = new float[values.length][values[0].length];

            for (int i = 0; i < values.length; i++) {
              for (int j = 1; j < values[i].length; j++) {
                glLighting[i][j - 1] = ((Float) values[i][j]).floatValue();
              }
            }

            fView3D.DebugSetOpenGLLighting(glLighting);
            fImageView3D.UpdateImageFrom3DView();
          }
        };
    applyButton.addActionListener(l);
    contentPanel.add(applyButton);
  }
}
