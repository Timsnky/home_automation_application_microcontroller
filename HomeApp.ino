String mainCommand ="";        //Receives string commands from BT or GSM;

//Credentials to login to website
String email = "homeapplicationgsm@gmail.com"; 
String password = "1234";

//Contacts for people incase of alarm
String phoneContacts = "";
unsigned long time;

//Pin Mappings depending on TIVA launchpad 
int pinMappings[33][2] ={{15,2},{10,3},{11,4},{44,5},{45,6},{14,7},{5,8},{6,9},
                          {7,10},{2,11},{3,12},{4,13},{16,14},{17,15},{40,18},{12,19},
                          {30,23},{31,24},{32,25},{33,26},{41,27},{42,28},{43,29},{37,32},
                          {36,33},{27,34},{26,35},{25,36},{24,37},{13,38},{52,40},{53,39},{51,30}};
                          
//Analog level parameter for brightness
int brightness = 255; 
float temperature;
int motionSensor = 0;

//Structure to store the devices initialised
struct Component
{
  int category;
  int pin;
  int state;
  int level;
};
struct Component device;
struct Component *devicePointer = & device;
int deviceIndex = 0;

//Error data to correct invalid block in structure
void errorDeviceData()
{
    (devicePointer + deviceIndex)->pin = 1;
    (devicePointer + deviceIndex)->category = 0;
    (devicePointer + deviceIndex)->state = 0;
    (devicePointer + deviceIndex)->level = 255;
    deviceIndex ++;
    deviceIndex ++;
}

//Initialising the Bluetooth Serial3 port
void BluetoothSetup()
{
  Serial3.begin(115200);
  delay(1000);  
}

//Main setup function for the Microcontroller
void setup()
{
  pinMode(38, INPUT);
  Serial.begin(2400);
  BluetoothSetup(); 
  errorDeviceData();
  gsmSetup();  
  deleteMessages();
  gprsSetup();
  receiveMessage();
  sendMessage("720023598","Gsm Testing");
}

//Main Loop function for the Microcontroller
void loop()
{  
  mainControl();
  time = millis();
  if (time % 1800000 == 0)
  {
    Serial.println("Its Time");
    prepareUpload();
  }
}

//Prepare the message with the status of the devices for sending to http get command
void prepareUpload()
{
  Serial.println("Prepare Upload");
  int index = 2;
  String uploadMessage = "d";
  while(index < deviceIndex)
  {
    uploadMessage += String((devicePointer + index)->pin) + ",";
    uploadMessage += String((devicePointer + index)->state); 
    index ++;
    if(index != deviceIndex)
    {
      uploadMessage += ":";
    }      
  }
  uploadData(uploadMessage);
}

//Send Alert incase of Alarm
void sendEmailAlert()
{
  Serial.println("Send Email Alert");
  uploadData("e");  
}

//Upload device statuses to the website using http get commands
void uploadData(String message)
{
  Serial.println("Upload Data");
  int upload = 1;
  int retries = 0;
  char inChar;
  Serial.println("Uploading");
  Serial.println(retries);
  String result = "";
  Serial1.println("AT+HTTPINIT");    
  delay(2000);
  Serial1.println("AT+HTTPPARA=\"URL\",\"http://homeapplication.3eeweb.com/GSMPage.php?data=" + message + "&email=" + email + "&password=" + password + "\"");
  delay(2000);
  Serial1.println("AT+HTTPACTION=0");
  delay(2000);
  result = checkData();
  Serial1.println("AT+HTTPREAD"); 
  delay(2000);
  if(checkData() == "Done")
  {
    upload = 0;
  }
  Serial1.println("AT+HTTPTERM");
  delay(2000);
}

//check if sim is available to send data
void toSerial()
{
  while(Serial1.available()!=0)
  {
    Serial.write(Serial1.read());
  }
}

//Removes the unnecessary http get command replies to get the desired message
String checkData()
{
  Serial.println("Check Data");
  char inChar;
  String result = "";
  while(Serial1.available()!=0)
  {
    inChar = Serial1.read();
    if(inChar == '#')
    {
      inChar = Serial1.read();
      delay(10);
      while(inChar != '#')
      {
        result = result + inChar;
        inChar = Serial1.read();      
      }
    }    
  }
  return result;
}

//Maps the pin numbers to the repestive Port and Pin Combination
int mapPin(int pin)
{
  Serial.println("Map Pin");
  int index = 0;
  while(index < 33)
  {
    if(pin == pinMappings[index][0])
    {
      return pinMappings[index][1];
    }
    index ++;
  }
  return 0;
}

//Function to handle Input from Serial Port 3
void serialEvent3(){
  mainCommand = "";
  if(Serial3.available() > 0)
  {
    char inChar = Serial3.read();
    if(inChar == '#')
    {
      inChar = Serial3.read();
      delay(100);
      while(inChar != '#')
      {
        mainCommand = mainCommand + inChar;
        inChar = Serial3.read();   
      }
    }
  }
}

//Function to handle Input from Serial Port 1
void serialEvent1(){
  mainCommand = "";
  if(Serial1.available() > 0)
  {
    char inChar = Serial1.read();
    if(inChar == '#')
    {
      inChar = Serial1.read();
      delay(100);
      while(inChar != '#')
      {
        mainCommand = mainCommand + inChar;
        inChar = Serial1.read();        
      }
    }
  }
}

/*Main Control function that handles processing of the various message strings received to
allocate them to the correct operation*/
void mainControl()
{
  String command = mainCommand;
  mainCommand = "";
  if(command != "")
  {
    Serial.println(command);
    String operation = command.substring(0,1);
    if(operation == "i")
    {
      sendData(command.substring(1,command.length()));    
    }
    else if(operation == "w")
    {
      prepareUpload(); 
    }
    else if(operation == "c")
    {
      controlOperation(command.substring(1,command.length()));
    }
    else if(operation == "n")
    {
      newDevice(command.substring(1,command.length()));
    }
    else if(operation == "r")
    {
      removeDevices(command.substring(1,command.length()));
    }
    else if(operation == "l")
    {
      storeCredentials(command.substring(1,command.length()));
    }
    else if(operation == "p")
    {
      storePhoneContacts(command.substring(1,command.length()));
    }
  }
}

//Send data to the android application either via text or bluetooth
void sendData(String command)
{
  Serial.println("Send Data");
  int mode = command.substring(0,1).toInt();
  Serial.println(mode);
  int index = 2;
  String bluetoothMessage = "*";
  while(index < deviceIndex)
  {
    bluetoothMessage += String((devicePointer + index)->category) + ",";
    bluetoothMessage += String((devicePointer + index)->pin) + ",";
    bluetoothMessage += String((devicePointer + index)->state) + ",";
    if((devicePointer + index)->category != 3 )
    {
      bluetoothMessage += String((devicePointer + index)->level);
    }   
    index ++;
    bluetoothMessage += ":";      
  }
  bluetoothMessage += readTemperature() + ":";
  bluetoothMessage += "4," + String(motionSensor);
  bluetoothMessage += "*";
  Serial.println(bluetoothMessage);
  if(mode == 1)
  {
    Serial.println("Bluetooth");
    Serial3.print(bluetoothMessage.substring(1,bluetoothMessage.length()));
  }
  else if(mode == 2)
  {
    Serial.println("GSM");
    sendMessage(command.substring(1, command.length()),bluetoothMessage);
  }  
}

String readTemperature()
{
  temperature = analogRead(18);
  temperature = ( temperature * 100 * 0.0699794);
  int temp = (int) temperature;
  String result = "3," + String(temp);
  Serial.println(result);
  return result;
}

//Offer control of the various devices depending on the input
void controlOperation(String command)
{
  Serial.println("Control Op");
  if(command != "")
  {
    int pinId,task,level;
    pinId = command.substring(0,2).toInt();
    task = command.substring(2,3).toInt();
    switch(task)
    {
      case 1:
          level = command.substring(3,command.length()).toInt();
          toggleControl(pinId,level);
          break;
      case 2:
          level = command.substring(3,command.length()).toInt();
          brightnessControl(pinId, level);
          break;
      case 3:
          level = command.substring(3,command.length()).toInt();
          setupMotionSensor(pinId, level);
          break;
    }
  }
}

//Toggle the device either On or Off
void toggleControl(int pinId, int state)
{
  Serial.println("Toggle Control");
  int realPinId = mapPin(pinId);
  pinMode(realPinId, OUTPUT);
  digitalWrite(realPinId, state);
  updateDeviceState(pinId, state);
  if( pinId == 45 && state == 0)
  {
    attachInterrupt(realPinId, detectMotion, RISING);
  }
}

//Update Device Status
void updateDeviceState(int pin, int state)
{
  Serial.println("Update Device State");
  int index = 1;
  while (index < deviceIndex)
  {
    if((devicePointer + index)->pin == pin)
    {
      (devicePointer + index)->state = state;      
      break;
    }
    index ++;
  }
  showComponents();
}

//Vary the brighntess or speed of the device
void brightnessControl(int pinId, int level)
{
  Serial.println("Brightness Control");
  int realPinId = mapPin(pinId);
  int desiredBrightness = (level * 255) / 100;
  pinMode(realPinId, OUTPUT);
  while(desiredBrightness != brightness)
  {
    if(desiredBrightness > brightness)
    {
      brightness ++;
    }
    else
    {
      brightness --;
    }    
    analogWrite(realPinId, brightness);
    delay(10);  
  }
   updateDeviceLevel(pinId, desiredBrightness); 
}

//Update Device Level
void updateDeviceLevel(int pin, int level)
{
  Serial.println("Update Device Level");
  int index = 1;
  while (index < deviceIndex)
  {
    if((devicePointer + index)->pin == pin)
    {
      (devicePointer + index)->level = level;      
       break;
    }
    index ++;
  }
  showComponents();
}

//Setup the motion sensor to interrupt
void setupMotionSensor(int pin, int level)
{
  Serial.println("Set Motion Sensor");
  int realPinId = mapPin(pin);
  if (realPinId == 38)
  {
    if(level == 1)
    {
      attachInterrupt(realPinId, detectMotion, RISING);
      motionSensor = 1;
    }
    else if(level == 0)
    {
      detachInterrupt(realPinId);
      motionSensor = 1;
    }
  }
}

int x = 0;
//Interrupt Service Routine incase motion is detected
void detectMotion()
{
  detachInterrupt(38);
  Serial.println("Detect Motion");
  toggleControl(45, 1);
  sendAlerts(); 
  //sendEmailAlert();  
}

//Add a new device to the microcontroller devices structure
void newDevice(String command)
{
  Serial.println("New Device");
  String category = command.substring(0,1);
  String commandString = command.substring(1,command.length());
  String password;
  int pin;
  pin = commandString.toInt();
  if(category == "l")
  {    
    addDevice(pin,0);
  }
  else if (category == "h")
  {
    addDevice(pin,1);
  }
  else if (category == "s")
  {
    addDevice(pin,2);
  }
}

//Add the devices appropriately depending on category
void addDevice(int pin,int category)
{
  Serial.println("Add Device");
  if(deviceIndex == 1)
  {
    deviceIndex ++;
  }
  (devicePointer + deviceIndex)->category = category;
  (devicePointer + deviceIndex)->pin = pin;
  (devicePointer + deviceIndex)->state = 0;
  if(category == 0)
  {
    (devicePointer + deviceIndex)->level = 255;
  }else if(category == 1)
  {
    (devicePointer + deviceIndex)->level = 255;
  }else
  {
    (devicePointer + deviceIndex)->level = 0;
  }  
  deviceIndex ++;
  showComponents();
}

//Display the components in the Devices Structure 
void showComponents()
{
  Serial.println("Show Components");
  int index = 2;
  while(index < deviceIndex)
  {
    Serial.println("Data");
    Serial.println(deviceIndex);
    Serial.println((devicePointer+index)->category);
    Serial.println((devicePointer+index)->pin);
    Serial.println((devicePointer+index)->state);
    Serial.println((devicePointer+index)->level);
    index ++;
  }
}

//Remove a device from the devices structure
void removeDevices(String command)
{
  Serial.println("Remove Devices");
  String category = command.substring(0,1);
  String commandString = command.substring(1,command.length());
  int pin = commandString.toInt();
  removeDevice(pin);
}

//Removing the device
void removeDevice(int pin)
{
  Serial.println("Remove Device");
  int index = 1;
  while (index < deviceIndex)
  {
    if((devicePointer + index)->pin == pin)
    {
      deviceIndex --;
      shiftUp(index);      
      break;
    }
    index ++;
  }
  showComponents();
}

//Shifting up the elements in the structure
void shiftUp(int index)
{
  Serial.println("Shift Up");
  while(index < deviceIndex)
  {
    (devicePointer + index)->category = (devicePointer + index + 1)->category;
    (devicePointer + index)->pin = (devicePointer + index + 1)->pin;
    (devicePointer + index)->state = (devicePointer + index + 1)->state;
    (devicePointer + index)->level = (devicePointer + index + 1)->level; 
    index ++;
  }
}

//Store the login credentials for the website
void storeCredentials(String command)
{
  Serial.println("Store Credentials");
  email = command.substring(0,command.indexOf(':'));
  password = command.substring(command.indexOf(':') + 1, command.length());
}

//Store the phone contacts for the numbers to be alerted
void storePhoneContacts(String command)
{
  Serial.println("Store Phone Contacts");
  phoneContacts = phoneContacts + command + ":";
  
}

//Send alert messges to the respective alert numbers
void sendAlerts()
{
  Serial.println("Send Alerts");
  String recipients = phoneContacts;
  while (recipients.indexOf(":") != -1)
  {
    sendMessage(recipients.substring(0, recipients.indexOf(':')), "Intruder Alert"); 
    recipients = recipients.substring(recipients.indexOf(':') + 1, recipients.length());
  }  
}

