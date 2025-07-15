package com.brazedblue.intervox.serial;

import com.brazedblue.intervox.util.NeuroSynchUtil;
import gnu.io.*;
import java.io.*;
import java.util.*;

public abstract class SerialPort {
  // move these into MacSerialPort?
  protected static final String kMacPrinterPortString = "Printer";
  protected static final String kMacModemPortString = "Modem";
  protected static final String kDefaultMacPortString = kMacModemPortString;

  protected static final String[] kMacPortStrings = {kMacModemPortString, kMacPrinterPortString};

  protected SerialPort() {}

  protected void finalize() throws Throwable {
    if (IsOpen()) {
      Close();
    }
  }

  public static String[] GetPortsList() {
    //		String[] result = null;

    //		if (NeuroSynchUtil.IsMacintosh()) {
    //			result = kMacPortStrings;
    //		} else if (NeuroSynchUtil.IsOSX()) {
    //			try {
    //				Enumeration iter = CommPortIdentifier.getPortIdentifiers();
    //
    //				Vector strings = new Vector();
    //				while (iter.hasMoreElements()) {
    //					CommPortIdentifier identifier = (CommPortIdentifier) iter.nextElement();
    //					strings.add(identifier.getName());
    //				}
    //				result = new String[strings.size()];
    //				return (String[]) strings.toArray(result);
    //			} catch (UnsatisfiedLinkError e) {
    //				return new String[0];
    //			}
    //		} else {
    //			NeuroSynchUtil.ErrorMessage("SerialPort.GetPortsList not implemented for OS.",
    // NeuroSynchUtil.kNoDlog);
    //		}
    //
    //		return result;
    return new String[0];
  }

  public static String GetExistingPort(String port) {
    String result = port;

    if (NeuroSynchUtil.IsMacintosh()) {
      result = QueryIfPortNotFound(port, kMacPortStrings);
    } else if (NeuroSynchUtil.IsOSX()) {
      String[] ports = GetPortsList();

      if (ports.length > 0) {
        result = QueryIfPortNotFound(port, ports);
      } else {
        result = null;
      }
    } else {
      NeuroSynchUtil.ErrorMessage(
          "SerialPort.GetExistingPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
    }

    return result;
  }

  private static String QueryIfPortNotFound(String port, String[] choices) {
    String result = null;

    if (!NeuroSynchUtil.StrArrayContainsString(choices, port)) {
      result =
          NeuroSynchUtil.GetUserInput(
              "Available Serial Port",
              "Could not find port \"" + port + "\". Please choose a new port",
              choices,
              kDefaultMacPortString);

      if (result == null) {
        result = "No Serial Port Set";
      }
    }

    return result;
  }

  public static SerialPort NewSerialPort(String port) {
    SerialPort result = null;

    if (NeuroSynchUtil.IsMacintosh()) {
      result = new MacSerialPort(port);
    } else if (NeuroSynchUtil.IsOSX()) {
      return new OSXSerialPort(port);
    } else {
      NeuroSynchUtil.ErrorMessage(
          "SerialPort.NewSerialPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
    }

    return result;
  }

  public abstract boolean IsOpen();

  public abstract void Open() throws IOException;

  public abstract void Close() throws IOException;

  public abstract InputStream GetInputStream();

  public abstract PrintWriter GetPrintWriter();

  public static String GetDefaultPort() {
    String result = "";

    if (NeuroSynchUtil.IsMacintosh()) {
      result = kDefaultMacPortString;
    } else if (NeuroSynchUtil.IsOSX()) {
      String[] list = GetPortsList();
      if (list.length > 0) {
        result = list[0];
      }
    } else {
      NeuroSynchUtil.ErrorMessage(
          "SerialPort.GetDefaultPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
    }

    return result;
  }
}
