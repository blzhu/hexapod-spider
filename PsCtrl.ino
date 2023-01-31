int battery = 0;
int ps_mode = 0;//0为平移模式，1为偏转模式，2为绕圈模式,5为设置模式
int ps_button_control_count = 0;
int ps_button_setting_count = 0;
int setting_leg_id = -1;
int setting_leg_servo_id = -1;
float rotate_pitch=15;
float rotate_roll =15;
float rotate_yaw=15;
float move_x = 15;
float move_y = 15;
int indexOfarray = 0;
int PS4_RSTICK_X;
int PS4_RSTICK_Y;
int PS4_LSTICK_X;
int PS4_LSTICK_Y;
int PS4_R2;
int PS4_L2;
int LStickXarray[num_avg]={0};
int LStickYarray[num_avg]={0};
int RStickXarray[num_avg]={0};
int RStickYarray[num_avg]={0};
int R2array[num_avg] = {0};
int L2array[num_avg] = {0};
//int bStickAction = 0;
//int bShoulderAction = 0;

void psInit()
{
  Ps3.attach(notify);
  Ps3.attachOnConnect(onConnect);
  Ps3.attachOnDisconnect(onDisconnect);
  Ps3.begin("01:02:03:04:05:06");
}
void notify()
{
  if(ps_mode == 5)
  {
    onSettings();
    return;
  }
  ps_button_setting_count = 0;
    //--- Digital cross/square/triangle/circle button events ---
    if( Ps3.event.button_down.cross ){
        Serial.println("Started pressing the cross button");
//        setTrotMode(1);
        ps_mode =2;
    }
    if( Ps3.event.button_down.square ){
        Serial.println("Started pressing the square button");
    }
    if( Ps3.event.button_down.triangle ){
        Serial.println("Started pressing the triangle button");
    }   
    if( Ps3.event.button_down.circle ){
        Serial.println("Started pressing the circle button");
        ps_mode = 1;
    }
    //--------------- Digital D-pad button events --------------
    if( Ps3.event.button_down.up ){
        Serial.println("Started pressing the up button");
        setTurnDirection(0);
        setDirectionTheta(0);
        if(!isTrotRunning())
          startTrot();
    }
    if( Ps3.event.button_down.right )
    {
        Serial.println("Started pressing the right button");
        if(ps_mode == 0){
          setTurnDirection(0);
          setDirectionTheta(M_PI/2);
          if(!isTrotRunning())
            startTrot();            
       
        }  
        else if(ps_mode == 1)
        {
           if(isTrotRunning())
            setTurnDirection(1);           
        }
        else if(ps_mode == 2)
        {
           setTurnRoundDirection(1);
           setTrotMode(1);  
           if(!isTrotRunning())
            startTrot();        
        }
    }    
    if( Ps3.event.button_down.down ){
        Serial.println("Started pressing the down button");
        setTurnDirection(0);
        setDirectionTheta(M_PI);
        if(!isTrotRunning())
          startTrot();        
    }
    if( Ps3.event.button_down.left ){
        Serial.println("Started pressing the left button");
        if(ps_mode == 0){
          setTurnDirection(0);
          setDirectionTheta(-M_PI/2);
          if(!isTrotRunning())
            startTrot();   
        }  
        else if(ps_mode == 1){
           if(isTrotRunning())
            setTurnDirection(-1);  
        }
        else if(ps_mode == 2)
        {
          setTurnRoundDirection(-1);
          setTrotMode(1);
          if(!isTrotRunning())
            startTrot(); 
        }
    }
    //------------- Digital shoulder button events -------------
    if( Ps3.event.button_down.l1 ){
        Serial.println("Started pressing the left shoulder button");

    }
    if( Ps3.event.button_down.r1 ){
        Serial.println("Started pressing the right shoulder button");
        
    }

    
    if( Ps3.event.button_up.cross ){
        Serial.println("Released the cross button");
        ps_mode = 0;
        setTrotMode(0); 
    }
    if( Ps3.event.button_up.square )
        Serial.println("Released the square button");
    if( Ps3.event.button_up.triangle ){
        Serial.println("Released the triangle button");
    }
    if( Ps3.event.button_up.circle ){
        Serial.println("Released the circle button");
        ps_mode = 0;
    }
    if( Ps3.event.button_up.up )
        Serial.println("Released the up button");
    if( Ps3.event.button_up.right ){
        Serial.println("Released the right button");

    }


    if( Ps3.event.button_up.down )
        Serial.println("Released the down button");


    if( Ps3.event.button_up.left ){
        Serial.println("Released the left button");

    }


    if( Ps3.event.button_up.l1 ){
        Serial.println("Released the left shoulder button");
        if(isTrotRunning())
        {
          stopTrot();
        }
        else
          action_stand(false);

    }


    if( Ps3.event.button_up.r1 ){
        Serial.println("Released the right shoulder button");
        if(isTrotRunning())
        {
          stopTrot();
        }
        else
          action_stand(true);
    }

    //-------------- Digital trigger button events -------------
    if( Ps3.event.button_down.l2 )
        Serial.println("Started pressing the left trigger button");

    if( Ps3.event.button_up.l2 )
        Serial.println("Released the left trigger button");

    if( Ps3.event.button_down.r2 )
        Serial.println("Started pressing the right trigger button");
    if( Ps3.event.button_up.r2 )
        Serial.println("Released the right trigger button");

    //--------------- Digital stick button events --------------
    if( Ps3.event.button_down.l3 )
        Serial.println("Started pressing the left stick button");
    if( Ps3.event.button_up.l3 )
        Serial.println("Released the left stick button");

    if( Ps3.event.button_down.r3 )
        Serial.println("Started pressing the right stick button");
    if( Ps3.event.button_up.r3 )
        Serial.println("Released the right stick button");

    //---------- Digital select/start/ps button events ---------
    if( Ps3.event.button_down.select )
        Serial.println("Started pressing the select button");
    if( Ps3.event.button_up.select )
        Serial.println("Released the select button");

    if( Ps3.event.button_down.start )
        Serial.println("Started pressing the start button");
    if( Ps3.event.button_up.start )
        Serial.println("Released the start button");

    if( Ps3.event.button_down.ps ){
        Serial.println("Started pressing the Playstation button");
        ps_button_control_count++;
        if(ps_button_control_count>=5){
          Serial.println("Started setting"); 
          ps_mode = 5;
          BeepWarning = 5;
          setLedLightCount(8);
        }
    }
    if( Ps3.event.button_up.ps )
        Serial.println("Released the Playstation button");



    //---------------- Analog stick value events ---------------
    if( abs(Ps3.event.analog_changed.stick.lx) + abs(Ps3.event.analog_changed.stick.ly) > 2 ){
       Serial.print("Moved the left stick:");//右下角为正
       Serial.print(" x="); Serial.print(Ps3.data.analog.stick.lx, DEC);
       Serial.print(" y="); Serial.print(Ps3.data.analog.stick.ly, DEC);
       Serial.println();
    }

   if( abs(Ps3.event.analog_changed.stick.rx) + abs(Ps3.event.analog_changed.stick.ry) > 2 ){
       Serial.print("Moved the right stick:");
       Serial.print(" x="); Serial.print(Ps3.data.analog.stick.rx, DEC);
       Serial.print(" y="); Serial.print(Ps3.data.analog.stick.ry, DEC);
       Serial.println();     
   }

//   //--------------- Analog D-pad button events ----------------
//   if( abs(Ps3.event.analog_changed.button.up) ){
//       Serial.print("Pressing the up button: ");
//       Serial.println(Ps3.data.analog.button.up, DEC);     
//   }
//
//   if( abs(Ps3.event.analog_changed.button.right) ){
//       Serial.print("Pressing the right button: ");
//       Serial.println(Ps3.data.analog.button.right, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.down) ){
//       Serial.print("Pressing the down button: ");
//       Serial.println(Ps3.data.analog.button.down, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.left) ){
//       Serial.print("Pressing the left button: ");
//       Serial.println(Ps3.data.analog.button.left, DEC);
//   }
//
//   //---------- Analog shoulder/trigger button events ----------
//   if( abs(Ps3.event.analog_changed.button.l1)){
//       Serial.print("Pressing the left shoulder button: ");
//       Serial.println(Ps3.data.analog.button.l1, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.r1) ){
//       Serial.print("Pressing the right shoulder button: ");
//       Serial.println(Ps3.data.analog.button.r1, DEC);
//   }
//
   if( abs(Ps3.event.analog_changed.button.l2) ){
       Serial.print("Pressing the left trigger button: ");
       Serial.println(Ps3.data.analog.button.l2, DEC);
//       int t = map(Ps3.data.analog.button.l2,0,255,getInitTrotCycleTime(),3500);
//       setTrotCycleTime(t);
   }

   if( abs(Ps3.event.analog_changed.button.r2) ){
       Serial.print("Pressing the right trigger button: ");
       Serial.println(Ps3.data.analog.button.r2, DEC);   
//       int l = map(Ps3.data.analog.button.r2,0,255,40,80);
//       setTrotStepLength(l);
   }
//
//   //---- Analog cross/square/triangle/circle button events ----
//   if( abs(Ps3.event.analog_changed.button.triangle)){
//       Serial.print("Pressing the triangle button: ");
//       Serial.println(Ps3.data.analog.button.triangle, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.circle) ){
//       Serial.print("Pressing the circle button: ");
//       Serial.println(Ps3.data.analog.button.circle, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.cross) ){
//       Serial.print("Pressing the cross button: ");
//       Serial.println(Ps3.data.analog.button.cross, DEC);
//   }
//
//   if( abs(Ps3.event.analog_changed.button.square) ){
//       Serial.print("Pressing the square button: ");
//       Serial.println(Ps3.data.analog.button.square, DEC);
//   }

    
   //---------------------- Battery events ---------------------
    if( battery != Ps3.data.status.battery ){
        battery = Ps3.data.status.battery;
        Serial.print("The controller battery is ");
        if( battery == ps3_status_battery_charging )      Serial.println("charging");
        else if( battery == ps3_status_battery_full )     Serial.println("FULL");
        else if( battery == ps3_status_battery_high )     Serial.println("HIGH");
        else if( battery == ps3_status_battery_low) 
        {      
          Serial.println("LOW");
          BeepWarning = 3;
        }
        else if( battery == ps3_status_battery_dying )
        {      
          Serial.println("DYING");
          BeepWarning = 4;
        }
        else if( battery == ps3_status_battery_shutdown ) 
        {
          Serial.println("SHUTDOWN");
          BeepWarning = 5;      
        }
        else Serial.println("UNDEFINED");
    }

}

void onConnect(){
    Serial.println(":) Connected.");
    BeepWarning=2;
    Ps3.setPlayer(1);
}
void onDisconnect(){
    Serial.println(":( Disconnect.");
    BeepWarning=1; 
    psInit();
}

void doPose()
{
  int xl = getAvgStickLX();
  int yl = getAvgStickLY();
  int xr = getAvgStickRX();
  int yr = getAvgStickRY();  
  if(xl==0&&yl==0&&xr==0&&yr==0)
    return;
  if(isZero()){
    initPostion(false);
    return;
  }  
  
  float tx = (float)map(xr,-127,127,-rotate_pitch*zoomRate,rotate_pitch*zoomRate)/zoomRate;
  float ty = (float)map(yr,-127,127,-rotate_roll*zoomRate,rotate_roll*zoomRate)/zoomRate;
  float mx=0;
  float my=0;
  float tz=0;
  if(Ps3.data.analog.button.circle==255)
    tz = (float)map(xl,-127,127,-rotate_yaw*zoomRate,rotate_yaw*zoomRate)/zoomRate;
  else{
    mx = (float)map(xl,-127,127,move_x*zoomRate,-move_x*zoomRate)/zoomRate;
    my = (float)map(yl,-127,127,-move_y*zoomRate,move_y*zoomRate)/zoomRate;
  }
 

  float x,y,z;
  for(int i=0;i<6;i++)
  {
    x = initPos[i][0];
    y = initPos[i][1];
    z = initPos[i][2];         
    coordinate_from_leg_to_body(i,x,y,z);
    rotate_metrix_x(x,y,z,angle2pi(ty));
    rotate_metrix_y(x,y,z,angle2pi(tx));
    rotate_metrix_z(x,y,z,angle2pi(tz));
    coordinate_from_body_to_leg(i,x,y,z);
    x+=mx;
    y+=my;
    direct_postion(i,x,y,z);
    Serial.printf("rotate x=%f, y=%f,z=%f\n",x,y,z);
  }
 
}

void shiftMove()
{
  if(!isTrotRunning())
    return;
  int xl = getAvgStickLX();
  int yl = getAvgStickLY();
  int xr = getAvgStickRX();
  int yr = getAvgStickRY();  
      
  float tx = (float)map(xr,-127,127,-rotate_pitch*zoomRate,rotate_pitch*zoomRate)/zoomRate;
  float ty = (float)map(yr,-127,127,-rotate_roll*zoomRate,rotate_roll*zoomRate)/zoomRate;
  float tz=0;
  if(Ps3.data.analog.button.circle==255)
    tz = (float)map(xl,-127,127,-rotate_yaw*zoomRate,rotate_yaw*zoomRate)/zoomRate;    
  setRotateTheta(angle2pi(ty)/2,angle2pi(tx)/2,angle2pi(tz)); //行进时rotate在x,y方向上减半 
     
  float t = 0;
  if(xl ==0 && yl ==0)
  {
    return;
  }
  else if(yl==0)
  {
    if(xl<0)
      t = -M_PI/2;
    else
      t = M_PI/2;
  }
  else if(xl == 0)
  {
      if(yl<0)
        t = 0;
      else
        t = M_PI;
  }
  else
  {
      t = atan(abs((float)(xl/yl)));    
      if(xl<0&&yl<0) 
        t = -t;
      else if(xl<0&&yl>0)
        t = -(M_PI-t);
      else if(xl>0&&yl>0)
        t = M_PI-t;   
  }
  if(Ps3.data.analog.button.circle==0)
    setDirectionTheta(t);
  if(!isTrotRunning())
    startTrot();        
}


void checkPS(){

  PS4_LSTICK_X = Ps3.data.analog.stick.lx;
  PS4_LSTICK_Y = Ps3.data.analog.stick.ly;
  PS4_RSTICK_X = Ps3.data.analog.stick.rx;
  PS4_RSTICK_Y = Ps3.data.analog.stick.ry;
  PS4_L2 = Ps3.data.analog.button.l2;
  PS4_R2 = Ps3.data.analog.button.r2;
  RStickXarray[indexOfarray] = PS4_RSTICK_X;
  RStickYarray[indexOfarray] = PS4_RSTICK_Y;
  LStickXarray[indexOfarray] = PS4_LSTICK_X;
  LStickYarray[indexOfarray] = PS4_LSTICK_Y;
  R2array[indexOfarray] = PS4_R2;
  L2array[indexOfarray] = PS4_L2;  
  indexOfarray++;
  if(indexOfarray>=num_avg)
    indexOfarray = 0;

  if(isTrotRunning())
  {
      int l = map(getAvgR2(),0,255,40,80);
      setTrotStepLength(l);
      int t = map(getAvgL2(),0,255,getInitTrotCycleTime(),3500);
      setTrotCycleTime(t);
      shiftMove();
    
  }
  else
  {
     doPose(); 
  } 
    
}

int getAvgStickLX()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += LStickXarray[i];
  }

  return sum/num_avg;
}

int getAvgStickLY()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += LStickYarray[i];
  }

  return sum/num_avg;
}

int getAvgStickRX()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += RStickXarray[i];
  }

  return sum/num_avg;
}

int getAvgStickRY()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += RStickYarray[i];
  }

  return sum/num_avg;
}

int getAvgL2()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += L2array[i];
  }
  return sum/num_avg;  
}

int getAvgR2()
{
  int sum = 0;
  for(int i=0;i<num_avg;i++)
  {
    sum += R2array[i];
  }
  return sum/num_avg;  
}

bool isSetting()
{
  if(ps_mode == 5)
    return true;
  return false;
}
void onSettings()
{
  ps_button_control_count=0; 
  //---------- Digital select/start/ps button events ---------
  if( Ps3.event.button_down.select ){
      Serial.println("setting select leg");
      //setting_leg_id set 90
      setting_leg_id++;
      if(setting_leg_id>=6)
        setting_leg_id = 0;
      BeepWarning = 1;
      setting_leg_servo_id = 0;
      setLedLight(setting_leg_id,setting_leg_servo_id);  
      for(int j=0;j<3;j++)
      {
        setOffset(setting_leg_id,j,0);  
      }
      
  }
  //--- Digital cross/square/triangle/circle button events ---
  if( Ps3.event.button_down.square ){
      Serial.println("Started pressing the square button");
  }
  if( Ps3.event.button_down.triangle ){
      setting_leg_servo_id = 0;
      BeepWarning = 1;
      setLedLight(setting_leg_id,setting_leg_servo_id);  
      Serial.printf("setting leg=%d,servo=%d\n",setting_leg_id,setting_leg_servo_id);           
      setOffset(setting_leg_id,setting_leg_servo_id,0);      
  }   
  if( Ps3.event.button_down.circle ){
      setting_leg_servo_id = 1;
      BeepWarning = 1;
      setLedLight(setting_leg_id,setting_leg_servo_id);  
      Serial.printf("setting leg=%d,servo=%d\n",setting_leg_id,setting_leg_servo_id);     
      setOffset(setting_leg_id,setting_leg_servo_id,0);     
  }
  if( Ps3.event.button_down.cross ){
      setting_leg_servo_id = 2;
      BeepWarning = 1;
      setLedLight(setting_leg_id,setting_leg_servo_id);  
      Serial.printf("setting leg=%d,servo=%d\n",setting_leg_id,setting_leg_servo_id);     
      setOffset(setting_leg_id,setting_leg_servo_id,0);
      
  } 
  if( Ps3.event.button_down.start ){
      Serial.println("Started pressing the start button");
  }

  if( Ps3.event.button_down.ps ){
      Serial.println("Started pressing the Playstation button");
      ps_button_setting_count++;
      if(ps_button_setting_count>=3){
        BeepWarning = 3;
        ps_mode = 0;
        saveConfig();
      }
  }
  if( Ps3.event.button_down.up )   
  {
    BeepWarning = 1;
    setOffset(setting_leg_id,setting_leg_servo_id,0.5);
  }
  if( Ps3.event.button_down.down )   
  {
    BeepWarning = 1;
    setOffset(setting_leg_id,setting_leg_servo_id,-0.5);
  } 
  if( Ps3.event.button_down.left )   
  {
    BeepWarning = 1;
    setOffset(setting_leg_id,setting_leg_servo_id,-1);
  }
  if( Ps3.event.button_down.right )   
  {
    BeepWarning = 1;
    setOffset(setting_leg_id,setting_leg_servo_id,1);
  }    
}

void setOffset(int i,int j,float offset)
{
  if(i<0||i>=6)return;
  if(j<0||j>=3)return;
  legOffset[i][j]+=offset;
  angle_to_servo(i,j,90);
  Serial.printf("setOffset leg=%d,servo=%d,offset=%f\n",i,j,legOffset[i][j]);
  
}
