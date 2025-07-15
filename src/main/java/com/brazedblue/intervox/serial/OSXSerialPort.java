package com.brazedblue.intervox.serial;

// gnu.io using rxtxcomm implementation, switch to gnu.io when implementation exists for mac OS X
import com.brazedblue.intervox.util.Debug;
import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.NoSuchPortException;
import gnu.io.RXTXPort; // change to SerialPort for gnu.io
import java.io.*;

public class OSXSerialPort extends SerialPort implements SerialDriver {

  private RXTXPort fCommPort;

  private InputStream fInputStream;
  private OutputStream fOutputStream;

  private String fWhichPort;
  private int fBaudRate;
  private int fParity;
  private int fStopBits;
  private int fDataBits;

  private static final boolean fgDebugPort = true;

  public OSXSerialPort(String whichPort, int baudRate, int parity, int stopBits, int dataBits) {
    fWhichPort = whichPort;
    fBaudRate = baudRate;
    fParity = parity;
    fStopBits = stopBits;
    fDataBits = dataBits;
  }

  public OSXSerialPort(String whichPort) {
    this(
        whichPort,
        9600,
        gnu.io.SerialPort.PARITY_NONE,
        gnu.io.SerialPort.STOPBITS_1,
        gnu.io.SerialPort.DATABITS_8);
  }

  public boolean IsOpen() {
    return fCommPort != null;
  }

  public void Open() throws IOException {
    try // debugging
    {
      if (fgDebugPort) {
        Debug.PrintStackTrace("Opening port " + fWhichPort);
      }
      if (fCommPort == null) {
        CommPortIdentifier portIdentifier = null;
        try {
          portIdentifier = CommPortIdentifier.getPortIdentifier(fWhichPort);
        } catch (NoSuchPortException e) {
          throw new IOException(e.getMessage());
        }

        try {
          CommPort commPort = portIdentifier.open("Intervox", 1000); // 2nd arg, timeout
          if (commPort instanceof RXTXPort) {
            fCommPort = (RXTXPort) commPort;
          } else {
            throw new IOException("\"" + fWhichPort + "\" is the wrong type of port");
          }
        } catch (gnu.io.PortInUseException exc) {
          fCommPort = null;
          if (fgDebugPort) {
            System.out.println("Available Ports");
            String[] portsList = GetPortsList();
            for (int i = 0; i < portsList.length; i++) {
              System.out.println("    " + portsList[i]);
            }
          }
          String message = exc.getMessage() + ". Port: \"" + fWhichPort + "\"";
          Debug.PrintStackTrace(message);
          throw new IOException(message);
        }

        try {
          if (fgDebugPort) {
            System.out.println("Setting serial port parameters");
          }
          fCommPort.setSerialPortParams(fBaudRate, fDataBits, fStopBits, fParity);
        } catch (gnu.io.UnsupportedCommOperationException exc2) {
          fCommPort = null;
          Debug.PrintStackTrace(exc2.getMessage());
          throw new IOException(exc2.getMessage());
        }

        if (fgDebugPort) {
          System.out.println("Getting input Stream");
        }
        fInputStream = fCommPort.getInputStream();
        if (fgDebugPort) {
          System.out.println("Getting output Stream");
        }
        fOutputStream = fCommPort.getOutputStream();
      } else {
        Debug.PrintStackTrace("fCommPort != null");
      }
    } catch (IOException e) {
      Debug.PrintStackTrace(e.getMessage());

      throw e;
    }
  }

  public void Close() {
    if (fgDebugPort) {
      Debug.PrintStackTrace("Closing port " + fWhichPort);
    }
    if (fCommPort != null) {
      fCommPort.close();
      fCommPort = null;
      fInputStream = null;
      fOutputStream = null;
    } else {
      Debug.PrintStackTrace("fCommPort == null");
    }
  }

  public InputStream GetInputStream() {
    return new SerialInStream(this);
  }

  public PrintWriter GetPrintWriter() {
    return new PrintWriter(new SerialOutStream(this));
  }

  public int GetAvailableBytes() throws IOException {
    int result = 0;

    try {
      if (fInputStream != null) {
        result = fInputStream.available();
      } else {
        System.out.println("fInputStream == null");
      }
    } catch (IOException e) {
      System.out.println(e.getMessage());
      throw e;
    }

    if (fgDebugPort && result > 0) {
      System.out.println("OSXSerialPort.GetAvailableBytes result: " + result);
    }

    return result;
  }

  public void Write(byte[] byteArray) throws IOException {
    if (fgDebugPort) {
      System.out.println("OSXSerialPort.Write byteArray: " + byteArray);
    }
    try // debugging try
    {
      if (fOutputStream != null) {
        fOutputStream.write(byteArray);
      } else {
        System.out.println("fOutputStream == null");
      }
    } catch (IOException e) {
      Debug.PrintStackTrace(e.getMessage());

      throw e;
    }
  }

  public int Read(byte[] byteArray) throws IOException {
    int result = 0;

    try // debugging
    {
      if (fInputStream != null) {
        result = fInputStream.read(byteArray);
      } else {
        System.out.println("fInputStream == null");
      }
    } catch (IOException e) {
      Debug.PrintStackTrace(e.getMessage());

      throw e;
    }

    if (fgDebugPort) {
      System.out.println("OSXSerialPort.Read result: " + result);
    }
    return result;
  }
}
