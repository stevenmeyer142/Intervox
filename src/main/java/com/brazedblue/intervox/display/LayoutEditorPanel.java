package com.brazedblue.intervox.display;

import com.brazedblue.intervox.image.NSImageView;
import java.awt.Dimension;
import javax.swing.*;

class LayoutEditorPanel extends JPanel {
  private ImageSeriesLayoutManager fLayoutManager;

  LayoutEditorPanel() {
    setLayout(null);
    fLayoutManager = new MyLayoutManager();
    setPreferredSize(new Dimension(20, 20)); // initialize preferred size
  }

  void SetLayoutSelections(LayoutSelectionsModel layoutSelections) {
    if (layoutSelections != null) {
      fLayoutManager.SetLayoutSelections(layoutSelections);
      fLayoutManager.SetupImageViews();
    }
  }

  public Dimension getPreferredSize() {
    Dimension result = super.getPreferredSize();
    LayoutModel layout = fLayoutManager.GetLayout();

    if (layout != null) {
      Dimension layoutSize = layout.GetLayoutSize();
      result.setSize(layoutSize);
    }

    return result;
  }

  public void doLayout() {
    Dimension imageViewsExtent = new Dimension(0, 0);

    LayoutModel layout = fLayoutManager.GetLayout();

    if (layout != null) {
      layout.Layout(this, imageViewsExtent);
    }
  }

  private class MyLayoutManager extends ImageSeriesLayoutManager {
    private MyLayoutManager() {
      super(LayoutEditorPanel.this);
    }

    protected void PostProcessView(NSImageView view) {
      super.PostProcessView(view);
      view.SetData(null);
    }
  }
}
