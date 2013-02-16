import geomerative.*;
import org.apache.batik.svggen.font.table.*;
import org.apache.batik.svggen.font.*;

/**
  Polargraph controller
  Copyright Sandy Noble 2012.

  This file is part of Polargraph Controller.

  Polargraph Controller is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Polargraph Controller is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
    
  Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
  Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
  
  This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.

  sandy.noble@gmail.com
  http://www.polargraph.co.uk/
  http://code.google.com/p/polargraph/
*/
import javax.swing.*;
import processing.serial.*;
import controlP5.*;
import java.awt.event.*;

ControlP5 cp5;

boolean drawbotReady = false;
boolean drawbotConnected = false;

int leftEdgeOfQueue = 800;
int rightEdgeOfQueue = 1100;
int topEdgeOfQueue = 0;
int bottomEdgeOfQueue = 0;
int queueRowHeight = 15;


Serial myPort;                       // The serial port
int[] serialInArray = new int[1];    // Where we'll put what we receive
int serialCount = 0;                 // A count of how many bytes we receive

final JFileChooser chooser = new JFileChooser();

SimpleDateFormat sdf = new SimpleDateFormat("dd/MM/yy hh:mm:ss");

String commandStatus = "Waiting for a click.";

List<String> commandQueue = new ArrayList<String>();
List<String> commandHistory = new ArrayList<String>();

String lastCommand = "";
String lastDrawingCommand = "";
Boolean commandQueueRunning = false;
static final String MODE_EXPORT_QUEUE = "button_mode_exportQueue";
static final String MODE_IMPORT_QUEUE = "button_mode_importQueue";
static final String MODE_CLEAR_QUEUE = "button_mode_clearQueue";
static final String MODE_CHANGE_SERIAL_PORT = "button_mode_serialPortDialog";
boolean useSerialPortConnection = false;

static final char BITMAP_BACKGROUND_COLOUR = 0x0F;

PVector homePointCartesian = null;


// used in the preview page
public color pageColour = color(220);
public color frameColour = color(200,0,0);
public color machineColour = color(150);
public color guideColour = color(255);
public color backgroundColour = color(100);
public color densityPreviewColour = color(0);


public boolean showingSummaryOverlay = true;
public boolean showingDialogBox = false;

public Integer windowWidth = 650;
public Integer windowHeight = 400;

public static Integer serialPortNumber = -1;


Properties props = null;
public static String propertiesFilename = "default.properties.txt";
public static String newPropertiesFilename = null;

public static final String TAB_NAME_QUEUE= "default";
public static final String TAB_LABEL_QUEUE = "Queue";

// Page states
public String currentTab = TAB_NAME_QUEUE;


public static final String PANEL_NAME_QUEUE = "panel_queue";

public final PVector DEFAULT_CONTROL_SIZE = new PVector(100.0, 20.0);
public final PVector CONTROL_SPACING = new PVector(2.0, 2.0);
public PVector mainPanelPosition = new PVector(10.0, 85.0);

public final Integer PANEL_MIN_HEIGHT = 400;

public Set<String> panelNames = null;
public List<String> tabNames = null;
public Set<String> controlNames = null;
public Map<String, List<Controller>> controlsForPanels = null;

public Map<String, Controller> allControls = null;
public Map<String, String> controlLabels = null;
public Set<String> controlsToLockIfBoxNotSpecified = null;
public Set<String> controlsToLockIfImageNotLoaded = null;

public Map<String, Set<Panel>> panelsForTabs = null;
public Map<String, Panel> panels = null;

void setup()
{
  println("Running polargraph controller");
  frame.setResizable(true);

  try 
  { 
    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName()); 
  } 
  catch (Exception e) 
  { 
    e.printStackTrace();   
  }   
  this.cp5 = new ControlP5(this);
  initTabs();

  String[] serialPorts = Serial.list();
  println("Serial ports available on your machine:");
  println(serialPorts);

  useSerialPortConnection = false;

  size(windowWidth, windowHeight);
  changeTab(TAB_NAME_QUEUE, TAB_NAME_QUEUE);
  frameRate(4);
}

void draw()
{
  drawCommandQueuePage();

  if (drawbotReady)
  {
    dispatchCommandQueue();
  }
  
}

String getCurrentTab()
{
  return this.currentTab;
}

Panel getPanel(String panelName)
{
  return getPanels().get(panelName);
}

void drawCommandQueuePage()
{
  cursor(ARROW);
  background(100);

  // machine outline
  fill(100);
  showingSummaryOverlay = false;
  

  
  int right = 0;
  for (Panel panel : getPanelsForTab(TAB_NAME_QUEUE))
  {
    panel.draw();
    float r = panel.getOutline().getRight();
    if (r > right)
      right = (int) r;
  }
  showCommandQueue(right, (int)mainPanelPosition.y);
  
  drawStatusText(245, 12);
  
}

void controlEvent(ControlEvent controlEvent) 
{
  if (controlEvent.isTab()) 
  {
    if (controlEvent.tab().name() == getCurrentTab())
    {
      // already here.
      println("Already here.");
    }
    else
    {
      changeTab(currentTab, controlEvent.tab().name());
    }
  }
  else if(controlEvent.isGroup()) 
  {
    print("got an event from "+controlEvent.group().name()+"\t");

    // checkbox uses arrayValue to store the state of 
    // individual checkbox-items. usage:
    for (int i=0; i<controlEvent.group().arrayValue().length; i++) 
    {
      int n = (int)controlEvent.group().arrayValue()[i];
    }
    println();
  }
  
}

void changeTab(String from, String to)
{
  
  // hide old panels
  currentTab = to;
  for (Panel panel : getPanelsForTab(currentTab))
  {
    for (Controller c : panel.getControls())
    {
      c.moveTo(currentTab);
      c.show();
    }
  }
  
}



boolean mouseOverQueue()
{
  boolean result = true;
  if (mouseX < leftEdgeOfQueue
    || mouseX > rightEdgeOfQueue
    || mouseY < topEdgeOfQueue
    || mouseY > bottomEdgeOfQueue)
    result = false;
  return result;
}

void mouseClicked()
{
  if (mouseOverQueue())
  {
    // stopping or starting 
    println("queue clicked.");
    queueClicked();
  }
}

void exportQueueToFile()
{
  if (!commandQueue.isEmpty())
  {
    String savePath = selectOutput();  // Opens file chooser
    if (savePath == null) 
    {
      // If a file was not selected
      println("No output file was selected...");
    } 
    else 
    {
      // If a file was selected, print path to folder
      println("Output file: " + savePath);
      String[] list = (String[]) commandQueue.toArray(new String[0]);
      saveStrings(savePath, list);
      println("Completed queue export, " + list.length + " commands exported.");
    }  
  }
}
void importQueueFromFile()
{
  commandQueue.clear();
  String loadPath = selectInput();
  if (loadPath == null)
  {
    // nothing selected
    println("No input file was selected.");
  }
  else
  {
    println("Input file: " + loadPath);
    String commands[] = loadStrings(loadPath);
//    List<String> list = Arrays
    commandQueue.addAll(Arrays.asList(commands));
    println("Completed queue import, " + commandQueue.size() + " commands found.");
  }
}


void queueClicked()
{
  int relativeCoord = (mouseY-topEdgeOfQueue);
  int rowClicked = relativeCoord / queueRowHeight;
  int totalCommands = commandQueue.size();
  
  if (rowClicked < 1) // its the header - start or stop queue
  {
    if (commandQueueRunning)
      commandQueueRunning = false;
    else
      commandQueueRunning = true;
  }
  else if (rowClicked > 2 && rowClicked < totalCommands+3) // it's a command from the queue
  {
    int cmdNumber = rowClicked-2;
    if (commandQueueRunning)
    {
      // if its running, then clicking on a command will mark it as a pause point
    }
    else
    {
      // if it's not running, then clicking on a command row will remove it
      commandQueue.remove(cmdNumber-1);
    }
  }
}
void resetQueue()
{
  commandQueue.clear();
  commandHistory.clear();
}
void drawStatusText(int x, int y)
{
  String drawbotStatus = null;
  
  if (useSerialPortConnection)
  {
    if (isDrawbotConnected())
    {
      if (drawbotReady)
      {
        fill(0, 200, 0);
        drawbotStatus = "Polargraph READY!";
      }
      else
      {
        fill(200, 200, 0);
        String busyDoing = lastCommand;
        if ("".equals(busyDoing))
          busyDoing = commandHistory.get(commandHistory.size()-1);
        drawbotStatus = "BUSY: " + busyDoing;
      }  
    }
    else
    {
      fill(255, 0, 0);
      drawbotStatus = "Polargraph is not connected.";
    }  
  }
  else
  {
    fill(255, 0, 0);
    drawbotStatus = "No serial connection.";
  }
  
  text(drawbotStatus, x, y);
  fill(255);
}

void setCommandQueueFont()
{
  textSize(12);
  fill(255);
}  
void showCommandQueue(int xPos, int yPos)
{
  setCommandQueueFont();
  int tRow = 15;
  int textPositionX = xPos;
  int textPositionY = yPos;
  int tRowNo = 1;

  int commandQueuePos = textPositionY+(tRow*tRowNo++);

  topEdgeOfQueue = commandQueuePos-queueRowHeight;
  leftEdgeOfQueue = textPositionX;
  rightEdgeOfQueue = textPositionX+300;
  bottomEdgeOfQueue = height;
  
  drawCommandQueueStatus(textPositionX, commandQueuePos, 14);
  commandQueuePos+=queueRowHeight;
  text("Last command: " + ((commandHistory.isEmpty()) ? "-" : commandHistory.get(commandHistory.size()-1)), textPositionX, commandQueuePos);
  commandQueuePos+=queueRowHeight;
  text("Current command: " + lastCommand, textPositionX, commandQueuePos);
  commandQueuePos+=queueRowHeight;
  
  fill(128,255,255);
  int queueNumber = commandQueue.size();
  fill(255);
  for (String s : commandQueue)
  {
    text((queueNumber--)+". "+ s, textPositionX, commandQueuePos);
    commandQueuePos+=queueRowHeight;
  }
}

void drawCommandQueueStatus(int x, int y, int tSize)
{
  String queueStatus = null;
  textSize(tSize);
  if (commandQueueRunning)
  {
    queueStatus = "QUEUE RUNNING - click to pause";
    fill(0, 200, 0);
  }
  else
  {
    queueStatus = "QUEUE PAUSED - click to start";
    fill(255, 0, 0);
  }

  text("CommandQueue: " + queueStatus, x, y);
  setCommandQueueFont();
}

void serialEvent(Serial myPort) 
{ 
  // read the serial buffer:
  String incoming = myPort.readStringUntil('\n');
  myPort.clear();
  // if you got any bytes other than the linefeed:
  incoming = trim(incoming);
  println("incoming: " + incoming);
  
  if (incoming.startsWith("READY"))
    drawbotReady = true;
  else if (incoming.startsWith("ACK"))
    respondToAckCommand(incoming);
  else if ("RESEND".equals(incoming))
    resendLastCommand();
  else
    drawbotReady = false;

  if (drawbotReady)
    drawbotConnected = true;
}


void respondToAckCommand(String ack)
{
  String commandOnly = ack.substring(4);
  if (lastCommand.equals(commandOnly))
  {
    // that means the bot got the message!! huspag!!
    // signal the EXECUTION
    commandHistory.add(lastCommand);
    String command = "EXEC";
    lastCommand = "";
    println("Dispatching confirmation command: " + command);
    myPort.write(command);
  }
  else
  {
    // oh dear, the message got mangled!
    // try again!!!!
    if (lastCommand == null || lastCommand.equals(""))
    {
      println("Apparently the last command has been badly acknowledged, but there isn't one!!");
    }
    else
    {
      resendLastCommand();
    }
  }
}

void resendLastCommand()
{
  println("Re-sending command: " + lastCommand);
  myPort.write(lastCommand);
  drawbotReady = false;
}

void dispatchCommandQueue()
{
  if (isDrawbotReady() 
    && (!commandQueue.isEmpty())
    && commandQueueRunning)
  {

    String command = commandQueue.get(0);
    lastCommand = command;
    commandQueue.remove(0);
    println("Dispatching command: " + command);

    myPort.write(lastCommand);
    drawbotReady = false;
  }
}

boolean isDrawbotReady()
{
  return drawbotReady;
}
boolean isDrawbotConnected()
{
  return drawbotConnected;
}


/**
  Polargraph controller
  Copyright Sandy Noble 2012.

  This file is part of Polargraph Controller.

  Polargraph Controller is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Polargraph Controller is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
    
  Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
  Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
  
  This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.

  sandy.noble@gmail.com
  http://www.polargraph.co.uk/
  http://code.google.com/p/polargraph/
*/
class Panel
{
  private Rectangle outline = null;
  private String name = null;
  private List<Controller> controls = null;
  private Map<String, PVector> controlPositions = null;
  private Map<String, PVector> controlSizes = null;
  private boolean resizable = true;
  private float minimumHeight = DEFAULT_CONTROL_SIZE.y+4;
  private color outlineColour = color(255);
  
  public final color CONTROL_COL_BG_DEFAULT = color(0,54,82);
  public final color CONTROL_COL_BG_DISABLED = color(20,44,62);
  public final color CONTROL_COL_LABEL_DEFAULT = color(255);
  public final color CONTROL_COL_LABEL_DISABLED = color(200);

  public Panel(String name, Rectangle outline)
  {
    this.name = name;
    this.outline = outline;
  }
  
  public Rectangle getOutline()
  {
    return this.outline;
  }
  public void setOutline(Rectangle r)
  {
    this.outline = r;
  }
  
  public String getName()
  {
    return this.name;
  }
  public void setName(String name)
  {
    this.name = name;
  }
  
  public List<Controller> getControls()
  {
    if (this.controls == null)
      this.controls = new ArrayList<Controller>(0);
    return this.controls;
  }
  public void setControls(List<Controller> c)
  {
    this.controls = c;
  }
  
  public Map<String, PVector> getControlPositions()
  {
    return this.controlPositions;
  }
  public void setControlPositions(Map<String, PVector> cp)
  {
    this.controlPositions = cp;
  }
  
  public Map<String, PVector> getControlSizes()
  {
    return this.controlSizes;
  }
  public void setControlSizes(Map<String, PVector> cs)
  {
    this.controlSizes = cs;
  }
  
  void setOutlineColour(color c)
  {
    this.outlineColour = c;
  }
  
  void setResizable(boolean r)
  {
    this.resizable = r;
  }
  boolean isResizable()
  {
    return this.resizable;
  }
  
  void setMinimumHeight(float h)
  {
    this.minimumHeight = h;
  }
  float getMinimumHeight()
  {
    return this.minimumHeight;
  }
  
  public void draw()
  {
//    stroke(outlineColour);
//    strokeWeight(2);
//    rect(getOutline().getLeft(), getOutline().getTop(), getOutline().getWidth(), getOutline().getHeight());

    drawControls();
  }
  
  public void drawControls()
  {
    for (Controller c : this.getControls())
    {
      PVector pos = getControlPositions().get(c.name());
      float x = pos.x+getOutline().getLeft();
      float y = pos.y+getOutline().getTop();
      c.setPosition(x, y);

      PVector cSize = getControlSizes().get(c.name());
      c.setSize((int)cSize.x, (int)cSize.y);

      boolean locked = false;
      
      int col = c.getColor().getBackground();      
      setLock(c, locked);
    }
  }
  
  void setLock(Controller c, boolean locked) 
  {
    c.setLock(locked);
    if (locked) 
    {
      c.setColorBackground(CONTROL_COL_BG_DISABLED);
      c.setColorLabel(CONTROL_COL_LABEL_DISABLED);
    } 
    else 
    {
      c.setColorBackground(CONTROL_COL_BG_DEFAULT);
      c.setColorLabel(CONTROL_COL_LABEL_DEFAULT);
    }
  }  
  
  void setHeight(float h)
  {
    if (this.isResizable())
    {
      if (h <= getMinimumHeight())
        this.getOutline().setHeight(getMinimumHeight());
      else
        this.getOutline().setHeight(h);
      setControlPositions(buildControlPositionsForPanel(this));
      
      float left = 0.0;
      String controlName = "";
      for (String key : getControlPositions().keySet())
      {
        PVector pos = getControlPositions().get(key);
        if (pos.x >= left)
        {
          left = pos.x;
          controlName = key;
        }
      }
      
      Map<String, PVector> map = getControlSizes();
      
//      PVector size = getControlSizes().get(controlName);
//      println("size: " + size);
      float right = left + DEFAULT_CONTROL_SIZE.x;
      
      this.getOutline().setWidth(right);
    }
  }
  
  
}

/**
  Polargraph controller
  Copyright Sandy Noble 2012.

  This file is part of Polargraph Controller.

  Polargraph Controller is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Polargraph Controller is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
    
  Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
  Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
  
  This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.

  sandy.noble@gmail.com
  http://www.polargraph.co.uk/
  http://code.google.com/p/polargraph/
*/
class Rectangle
{
  public PVector position = null;
  public PVector size = null;
  
  public Rectangle(float px, float py, float width, float height)
  {
    this.position = new PVector(px, py);
    this.size = new PVector(width, height);
  }
  public Rectangle(PVector position, PVector size)
  {
    this.position = position;
    this.size = size;
  }
  public Rectangle(Rectangle r)
  {
    this.position = new PVector(r.getPosition().x, r.getPosition().y);
    this.size = new PVector(r.getSize().x, r.getSize().y);
  }
  
  public float getWidth()
  {
    return this.size.x;
  }
  public void setWidth(float w)
  {
    this.size.x = w;
  }
  public float getHeight()
  {
    return this.size.y;
  }
  public void setHeight(float h)
  {
    this.size.y = h;
  }
  public PVector getPosition()
  {
    return this.position;
  }
  public PVector getSize()
  {
    return this.size;
  }
  public PVector getTopLeft()
  {
    return getPosition();
  }
  public PVector getBotRight()
  {
    return PVector.add(this.position, this.size);
  }
  public float getLeft()
  {
    return getPosition().x;
  }
  public float getRight()
  {
    return getPosition().x + getSize().x;
  }
  public float getTop()
  {
    return getPosition().y;
  }
  public float getBottom()
  {
    return getPosition().y + getSize().y;
  }
  
  public void setPosition(float x, float y)
  {
    if (this.position == null)
      this.position = new PVector(x, y);
    else
    {
      this.position.x = x;
      this.position.y = y;
    }
  }
  
  public Boolean surrounds(PVector p)
  {
    if (p.x >= this.getLeft()
    && p.x < this.getRight()
    && p.y >= this.getTop()
    && p.y < this.getBottom())
      return true;
    else
      return false;
  }
  
}

/**
  Polargraph controller
  Copyright Sandy Noble 2012.

  This file is part of Polargraph Controller.

  Polargraph Controller is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Polargraph Controller is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
    
  Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
  Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
  
  This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.

  sandy.noble@gmail.com
  http://www.polargraph.co.uk/
  http://code.google.com/p/polargraph/
*/
void button_mode_pauseQueue()
{
}
void button_mode_runQueue()
{
}
void button_mode_clearQueue()
{
  resetQueue();
}
void button_mode_exportQueue()
{
  exportQueueToFile();
}
void button_mode_importQueue()
{
  importQueueFromFile();
}

void button_mode_serialPortDialog()
{
  ControlWindow serialPortWindow = cp5.addControlWindow("changeSerialPortWindow",100,100,150,150);
  serialPortWindow.hideCoordinates();
  
  serialPortWindow.setBackground(backgroundColour);
  Radio r = cp5.addRadio("radio_serialPort",10,10);

  if (serialPortNumber >= 0)
    r.setValue(serialPortNumber);
    
  r.add("setup", -2);
  r.add("No serial connection", -1);
  
  String[] ports = Serial.list();
  for (int i = 0; i < ports.length; i++)
  {
    r.add(ports[i], i);
  }
  
  int portNo = serialPortNumber;
  if (portNo > -1)
    r.activate(ports[portNo]);
  else
    r.activate("No serial connection");
    
  r.removeItem("setup");
  r.setWindow(serialPortWindow);
}

void radio_serialPort(int newSerialPort) 
{
  if (newSerialPort == -2)
  {
  }
  else if (newSerialPort == -1)
  {
    println("Disconnecting serial port.");
    useSerialPortConnection = false;
    if (myPort != null)
    {
      myPort.stop();
      myPort = null;
    }
    drawbotReady = false;
    drawbotConnected = false;
    serialPortNumber = newSerialPort;
  }
  else if (newSerialPort != serialPortNumber)
  {
    println("About to connect to serial port in slot " + newSerialPort);
    // Print a list of the serial ports, for debugging purposes:
    if (newSerialPort < Serial.list().length)
    {
      try 
      {
        drawbotReady = false;
        drawbotConnected = false;
        if (myPort != null)
        {
          myPort.stop();
          myPort = null;
        }
        if (serialPortNumber >= 0)
          println("closing " + Serial.list()[serialPortNumber]);
        
        serialPortNumber = newSerialPort;
        String portName = Serial.list()[serialPortNumber];
  
        myPort = new Serial(this, portName, 57600);
        //read bytes into a buffer until you get a linefeed (ASCII 10):
        myPort.bufferUntil('\n');
        useSerialPortConnection = true;
        println("Successfully connected to port " + portName);
      }
      catch (Exception e)
      {
        println("Attempting to connect to serial port in slot " + serialPortNumber 
        + " caused an exception: " + e.getMessage());
      }
    }
    else
    {
      println("No serial ports found.");
      useSerialPortConnection = false;
    }
  }
  else
  {
    println("no serial port change.");
  }
}

/**
 Polargraph controller
 Copyright Sandy Noble 2012.
 
 This file is part of Polargraph Controller.
 
 Polargraph Controller is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Polargraph Controller is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
 
 Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
 Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
 
 This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.
 
 sandy.noble@gmail.com
 http://www.polargraph.co.uk/
 http://code.google.com/p/polargraph/
 */
Set<String> getPanelNames()
{
  if (this.panelNames == null)
    this.panelNames = buildPanelNames();
  return this.panelNames;
}
List<String> getTabNames()
{
  if (this.tabNames == null)
    this.tabNames = buildTabNames();
  return this.tabNames;
}
Set<String> getControlNames()
{
  if (this.controlNames == null)
    this.controlNames = buildControlNames();
  return this.controlNames;
}
Map<String, List<Controller>> getControlsForPanels()
{
  if (this.controlsForPanels == null)
    this.controlsForPanels = buildControlsForPanels();
  return this.controlsForPanels;
}
Map<String, Controller> getAllControls()
{
  if (this.allControls == null)
    this.allControls = buildAllControls();
  return this.allControls;
}
Map<String, String> getControlLabels()
{
  if (this.controlLabels == null)
    this.controlLabels = buildControlLabels();
  return this.controlLabels;
}
Map<String, Set<Panel>> getPanelsForTabs()
{
  if (this.panelsForTabs == null)
    this.panelsForTabs = buildPanelsForTabs();
  return this.panelsForTabs;
}
Map<String, Panel> getPanels()
{
  if (this.panels == null)
    this.panels = buildPanels();
  return this.panels;
}

void hideAllControls()
{
  for (String key : allControls.keySet())
  {
    Controller c = allControls.get(key);
    c.hide();
  }
}

Map<String, Panel> buildPanels()
{
  Map<String, Panel> panels = new HashMap<String, Panel>();

  float panelHeight = frame.getHeight() - getMainPanelPosition().y - (DEFAULT_CONTROL_SIZE.y*3);
  Rectangle panelOutline = new Rectangle(getMainPanelPosition(), 
  new PVector((DEFAULT_CONTROL_SIZE.x+CONTROL_SPACING.x), 400));
  Panel queuePanel = new Panel(PANEL_NAME_QUEUE, panelOutline);
  queuePanel.setOutlineColour(color(200, 200, 200));
  // get controls
  queuePanel.setResizable(true);
  queuePanel.setControls(getControlsForPanels().get(PANEL_NAME_QUEUE));
  // get control positions
  queuePanel.setControlPositions(buildControlPositionsForPanel(queuePanel));
  queuePanel.setControlSizes(buildControlSizesForPanel(queuePanel));
  panels.put(PANEL_NAME_QUEUE, queuePanel);

  return panels;
}

PVector getMainPanelPosition()
{
  return this.mainPanelPosition;
}

Map<String, Controller> buildAllControls()
{

  Map<String, Controller> map = new HashMap<String, Controller>();

  for (String controlName : getControlNames())
  {
    if (controlName.startsWith("button_"))
    {
      Button b = cp5.addButton(controlName, 0, 100, 100, 100, 100);
      b.setLabel(getControlLabels().get(controlName));
      b.hide();
      map.put(controlName, b);
      //      println("Added button " + controlName);
    }
    else if (controlName.startsWith("toggle_"))
    {
      Toggle t = cp5.addToggle(controlName, false, 100, 100, 100, 100);
      t.setLabel(getControlLabels().get(controlName));
      t.hide();
      controlP5.Label l = t.captionLabel();
      l.style().marginTop = -17; //move upwards (relative to button size)
      l.style().marginLeft = 4; //move to the right
      map.put(controlName, t);
      //      println("Added toggle " + controlName);
    }
    else if (controlName.startsWith("minitoggle_"))
    {
      Toggle t = cp5.addToggle(controlName, false, 100, 100, 100, 100);
      t.setLabel(getControlLabels().get(controlName));
      t.hide();
      controlP5.Label l = t.captionLabel();
      l.style().marginTop = -17; //move upwards (relative to button size)
      l.style().marginLeft = 4; //move to the right
      map.put(controlName, t);
      //      println("Added minitoggle " + controlName);
    }
    else if (controlName.startsWith("numberbox_"))
    {
      Numberbox n = cp5.addNumberbox(controlName, 100, 100, 100, 100, 20);
      n.setLabel(getControlLabels().get(controlName));
      n.hide();
      n.setDecimalPrecision(0);
      controlP5.Label l = n.captionLabel();
      l.style().marginTop = -17; //move upwards (relative to button size)
      l.style().marginLeft = 40; //move to the right
      // change the control direction to left/right
      n.setDirection(Controller.VERTICAL);
      map.put(controlName, n);
      //      println("Added numberbox " + controlName);
    }
  }

  return map;
}




String getControlLabel(String butName)
{
  if (controlLabels.containsKey(butName))
    return controlLabels.get(butName);
  else
    return "";
}

Map<String, PVector> buildControlPositionsForPanel(Panel panel)
{
  Map<String, PVector> map = new HashMap<String, PVector>();
  String panelName = panel.getName();
  int col = 0;
  int row = 0;
  for (Controller controller : panel.getControls())
  {
    if (controller.name().startsWith("minitoggle_"))
    {
      PVector p = new PVector(col*(DEFAULT_CONTROL_SIZE.x+CONTROL_SPACING.x), row*(DEFAULT_CONTROL_SIZE.y+CONTROL_SPACING.y));
      map.put(controller.name(), p);
      row++;
      if (p.y + (DEFAULT_CONTROL_SIZE.y*2) >= panel.getOutline().getHeight())
      {
        row = 0;
        col++;
      }
    }
    else
    {
      PVector p = new PVector(col*(DEFAULT_CONTROL_SIZE.x+CONTROL_SPACING.x), row*(DEFAULT_CONTROL_SIZE.y+CONTROL_SPACING.y));
      map.put(controller.name(), p);
      row++;
      if (p.y + (DEFAULT_CONTROL_SIZE.y*2) >= panel.getOutline().getHeight())
      {
        row = 0;
        col++;
      }
    }
  }

  return map;
}
Map<String, PVector> buildControlSizesForPanel(Panel panel)
{
  Map<String, PVector> map = new HashMap<String, PVector>();
  String panelName = panel.getName();
  int col = 0;
  int row = 0;
  for (Controller controller : panel.getControls())
  {
    if (controller.name().startsWith("minitoggle_"))
    {
      PVector s = new PVector(DEFAULT_CONTROL_SIZE.y, DEFAULT_CONTROL_SIZE.y);
      map.put(controller.name(), s);
    }
    else
    {
      PVector s = new PVector(DEFAULT_CONTROL_SIZE.x, DEFAULT_CONTROL_SIZE.y);
      map.put(controller.name(), s);
    }
  }

  return map;
}


Map<String, List<Controller>> buildControlsForPanels()
{
//  println("build controls for panels.");
  Map<String, List<Controller>> map = new HashMap<String, List<Controller>>();
  map.put(PANEL_NAME_QUEUE, getControllersForControllerNames(getControlNamesForQueuePanel()));
  return map;
}

List<Controller> getControllersForControllerNames(List<String> names)
{
  List<Controller> list = new ArrayList<Controller>();
  for (String name : names)
  {
    Controller c = getAllControls().get(name);
    if (c != null)
      list.add(c);
  }
  return list;
}

List<String> getControlNamesForQueuePanel()
{
  List<String> controlNames = new ArrayList<String>();
  controlNames.add(MODE_CHANGE_SERIAL_PORT);
  controlNames.add(MODE_CLEAR_QUEUE);
  controlNames.add(MODE_EXPORT_QUEUE);
  controlNames.add(MODE_IMPORT_QUEUE);

  return controlNames;
}



Map<String, String> buildControlLabels()
{
  Map<String, String> result = new HashMap<String, String>();
  result.put(MODE_CLEAR_QUEUE, "Clear queue");
  result.put(MODE_EXPORT_QUEUE, "Export queue");
  result.put(MODE_IMPORT_QUEUE, "Import queue");
  result.put(MODE_CHANGE_SERIAL_PORT, "Serial port...");

  return result;
}

Set<String> buildControlNames()
{
  Set<String> result = new HashSet<String>();
  result.add(MODE_CLEAR_QUEUE);
  result.add(MODE_EXPORT_QUEUE);
  result.add(MODE_IMPORT_QUEUE);
  result.add(MODE_CHANGE_SERIAL_PORT);
  return result;
}

/**
  Polargraph controller
  Copyright Sandy Noble 2012.

  This file is part of Polargraph Controller.

  Polargraph Controller is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Polargraph Controller is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Polargraph Controller.  If not, see <http://www.gnu.org/licenses/>.
    
  Requires the excellent ControlP5 GUI library available from http://www.sojamo.de/libraries/controlP5/.
  Requires the excellent Geomerative library available from http://www.ricardmarxer.com/geomerative/.
  
  This is an application for controlling a polargraph machine, communicating using ASCII command language over a serial link.

  sandy.noble@gmail.com
  http://www.polargraph.co.uk/
  http://code.google.com/p/polargraph/
*/

Set<Panel> getPanelsForTab(String tabName)
{
  if (getPanelsForTabs().containsKey(tabName))
  {
    return getPanelsForTabs().get(tabName);
  }
  else
    return new HashSet<Panel>(0);
}

Map<String, Set<Panel>> buildPanelsForTabs()
{
  Map<String, Set<Panel>> map = new HashMap<String, Set<Panel>>();

  Set<Panel> queuePanels = new HashSet<Panel>(2);
  queuePanels.add(getPanel(PANEL_NAME_QUEUE));
  
  map.put(TAB_NAME_QUEUE, queuePanels);
  
  return map;
}

List<String> buildTabNames()
{
  List<String> list = new ArrayList<String>(4);
  list.add(TAB_NAME_QUEUE);
  return list;
}

void initTabs()
{
  cp5.tab(TAB_NAME_QUEUE).setLabel(TAB_LABEL_QUEUE);
  cp5.tab(TAB_NAME_QUEUE).activateEvent(true);
  cp5.tab(TAB_NAME_QUEUE).setId(1);

}

public Set<String> buildPanelNames()
{
  Set<String> set = new HashSet<String>(5);
  set.add(PANEL_NAME_QUEUE);
  return set;
}


