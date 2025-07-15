package com.brazedblue.intervox.data;

public class BroadcasterDataModel extends DataModel implements DataModelListener {

  public BroadcasterDataModel() {}

  public void DataModelChanged(DataModel model, DataChangedEvent event) {
    Changed(event);
  }
}
