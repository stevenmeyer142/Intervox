package com.brazedblue.intervox.serial;

import java.io.*;
import gnu.io.*;
import java.util.*;

import com.brazedblue.intervox.util.NeuroSynchUtil;

public abstract class SerialPort {
	// move these into MacSerialPort?
	static protected final String kMacPrinterPortString = "Printer";
	static protected final String kMacModemPortString = "Modem";
	static protected final String kDefaultMacPortString = kMacModemPortString;

	static protected final String[] kMacPortStrings = { kMacModemPortString, kMacPrinterPortString };

	protected SerialPort() {
	}

	protected void finalize() throws Throwable {
		if (IsOpen()) {
			Close();
		}

	}

	static public String[] GetPortsList() {
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
//			NeuroSynchUtil.ErrorMessage("SerialPort.GetPortsList not implemented for OS.", NeuroSynchUtil.kNoDlog);
//		}
//
//		return result;
		return new String[0];
	}

	static public String GetExistingPort(String port) {
		String result = port;

		if (NeuroSynchUtil.IsMacintosh()) {
			result = QueryIfPortNotFound(port, kMacPortStrings);
		} else if (NeuroSynchUtil.IsOSX()) {
			String[] ports = GetPortsList();
			
			if (ports.length > 0)
			{
				result = QueryIfPortNotFound(port, ports);
			}
			else
			{
				result = null;
			}
		} else {
			NeuroSynchUtil.ErrorMessage("SerialPort.GetExistingPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
		}

		return result;
	}

	static private String QueryIfPortNotFound(String port, String[] choices) {
		String result = null;

		if (!NeuroSynchUtil.StrArrayContainsString(choices, port)) {
			result = NeuroSynchUtil.GetUserInput("Available Serial Port",
					"Could not find port \"" + port + "\". Please choose a new port", choices, kDefaultMacPortString);

			if (result == null) {
				result = "No Serial Port Set";
			}
		}

		return result;
	}

	static public SerialPort NewSerialPort(String port) {
		SerialPort result = null;

		if (NeuroSynchUtil.IsMacintosh()) {
			result = new MacSerialPort(port);
		} else if (NeuroSynchUtil.IsOSX()) {
			return new OSXSerialPort(port);
		} else {
			NeuroSynchUtil.ErrorMessage("SerialPort.NewSerialPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
		}

		return result;
	}

	public abstract boolean IsOpen();

	public abstract void Open() throws IOException;

	public abstract void Close() throws IOException;

	public abstract InputStream GetInputStream();

	public abstract PrintWriter GetPrintWriter();

	static public String GetDefaultPort() {
		String result = "";

		if (NeuroSynchUtil.IsMacintosh()) {
			result = kDefaultMacPortString;
		} else if (NeuroSynchUtil.IsOSX()) {
			String[] list = GetPortsList();
			if (list.length > 0) {
				result = list[0];
			}
		} else {
			NeuroSynchUtil.ErrorMessage("SerialPort.GetDefaultPort not implemented for OS.", NeuroSynchUtil.kNoDlog);
		}

		return result;
	}

}
