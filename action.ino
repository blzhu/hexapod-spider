bool bStandHighMode = false;
void action_stand(bool bHigh)
{
  float x,y,z;
  servoThreadMode = 0;     
  bStandHighMode = bHigh;
  if(isZero()){
    initPostion(bHigh);
    return;
  }
  if(bHigh)
    setTrotCycleTime(1000);
  else
    setTrotCycleTime(350);
  for(int i=0;i<6;i++){
    x = initPos[i][0];
    y = initPos[i][1];
    if(initPos[i][2] != getStandHigh())
      initPos[i][2]=getStandHigh();
    z = initPos[i][2];    
    set_expect_postion(i,x,y,z);
  }
  wait_all_reach_postion(); 
}

bool isStand()
{
  for(int i=0;i<6;i++){
    for(int j=0;j<3;j++)
      if(curPostion[i][j]!=initPos[i][j])
        return false;
  }
  return true;
}
bool isZero()
{
  for(int i=0;i<6;i++){
    for(int j=0;j<3;j++)
      if(initPos[i][j]!=0)
        return false;
  }
  return true;
}
bool isStandHighMode()
{
  return bStandHighMode;
}
