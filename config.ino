const String fileName = "/config.ini";
void readFile(fs::FS &fs, const String path,String& readStr){
    Serial.printf("Reading file: %s\n", path.c_str());

    File file = fs.open(path.c_str());
    if(!file || file.isDirectory()){ 
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        char c = file.read();
        readStr+= String(c);
    }
    file.close();
//    Serial.println(readStr);
}

void writeFile(fs::FS &fs, const String path, const String message){
    Serial.printf("Writing file: %s\r\n", path.c_str());

    File file = fs.open(path.c_str(), FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message.c_str())){
        Serial.println("- file written");
    } else {
        Serial.println("- frite failed");
    }
}
void readConfig()
{
  if(!SPIFFS.begin(true)){
      Serial.println("SPIFFS Mount Failed");
      return;
  }  
  String jsonRead;
  readFile(SPIFFS, fileName,jsonRead);
  if(jsonRead.length()<=0)
  {
    Serial.println("no config file");
    return;
  }
  DynamicJsonDocument configJson(1024);
  deserializeJson(configJson, jsonRead);    
  for(int i=0;i<6;i++)
  {
    for(int j=0;j<3;j++)
    {
      legOffset[i][j] = configJson["offset"][i][j];
    }
  }
  Serial.println(jsonRead);
}

void saveConfig()
{
  DynamicJsonDocument configJson(1024);
  for(int i=0;i<6;i++)
  {
    for(int j=0;j<3;j++)
    {
      configJson["offset"][i][j]=legOffset[i][j];
    }
  } 
  String jsonStr;
  serializeJson(configJson, jsonStr);
  Serial.println(jsonStr);
  writeFile(SPIFFS, fileName, jsonStr);

}
