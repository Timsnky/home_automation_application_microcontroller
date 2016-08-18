void gsmSetup()
{  
  Serial1.begin(2400);  //Baud rate of the GSM/GPRS Module 
  Serial1.print("\r");
  delay(1000); 
}

//Sends the Message to the supplied Number and the given message
void sendMessage(String mobileNumber, String message)
{ 
  Serial.println("Start");   
  Serial.println(mobileNumber);
  Serial.println(message);
  Serial1.print("AT+CMGF=1\r");    
  delay(1000);
  Serial1.print("AT+CMGS=\"+254" + mobileNumber +"\"\r"); //Number to which you want to send the sms
  delay(1000);
  Serial1.print(message +"\r");   //The text of the message to be sent
  delay(1000);
  Serial1.write(0x1A);
  delay(1000);
  Serial.println("Sent");  
}

//Delete all messages
void deleteMessages()
{
   Serial1.println("AT+CMGD=1,4"); //Delete all messages in memory
   delay(1000);
}

//Setup to recieve message
void receiveMessage()
{
  Serial1.print("AT+CMGF=1\r");  // set SMS mode to text
  delay(1000);
  Serial1.print("AT+CNMI=2,2,0,0,0\r");
  delay(1000);
  deleteMessages();
}

void gprsSetup()
{
  Serial1.println("AT+CGATT?");
  delay(100);
  
  Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  
  Serial1.println("AT+SAPBR=3,1,\"APN\",\"safaricom\"");
  delay(2000);
  
  Serial1.println("AT+SAPBR=1,1");
  delay(2000);
}
