


bool invalid_name = false; 
String get_command_and_value(){
  String command_name;
  {
    String incomingData = Serial.readString();
    //Serial.print(incomingData);
    char *incomingData_ch = new char[incomingData.length()];
    incomingData.toCharArray(incomingData_ch , incomingData.length());
    int x_command_name_ch = 0; 
    for(int i=0 ; i<incomingData.length() ; i++){ 
      if(isSpace(incomingData_ch[i])){
        break;   
      }
      x_command_name_ch++;
    }
    char *command_name_char = new char[x_command_name_ch];
    for(int i=0; i< x_command_name_ch; i++){ 
      command_name_char[i] = incomingData_ch[i];
      if(isSpace(command_name_char[i])){
        invalid_name = true;  
      }
    }
    // my command 
    command_name = String(command_name_char);
    //Serial.print("My command -");
   // Serial.println( command_name);
    delete []incomingData_ch;
    delete []command_name_char;
  }
  return command_name;
}
