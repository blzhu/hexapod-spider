
//摆线方程
/*
 *      x = r(t-sin*t)
 *      z = r(1-cos*t)
 * 
 *      r为半径，为抬腿高度的一半，t为滚动角， 0 < t < 2*pi , 
 *      从原点启动的话t是0-2pi，我们是从站立方式启动，启动是在t=pi位置，就是摆线的中心点开始
 * 
 * 
 * 
 *      对于0，3脚     半个摆动相 ----- 支撑相 ---- 半个摆动相
 *      对于1，2脚     半个支持相 ----- 摆动相 ---- 半个支撑相
 * 
 * 
 * */
#define STEP_HIGH 40
#define STEP_HIGH_MAX 80
#define TRUN_DIRECTION_RATE_MAX 1
int cycle_count = 0;
int step_cycle_time = 350;//300毫秒为一个周期，对应 2*PI
int step_last_time = 0;
int step_cur_time = 0;//当前所在周期的时间
int step_lift_high = STEP_HIGH;//trot 步态抬腿高度
int start_trot = 0;
float trot_step_length = 40;//这个值是限定真实trot的步长
float step_cycloid_phase_r = step_lift_high/2;//摆线半径
float step_length = step_cycloid_phase_r*2*M_PI;//x = r*(t-sin(t)),t=2*PI,x=r*(2*PI-0) = r*2*PI
float trot_step_rate = trot_step_length/step_length;//调整比例
float step_rate = 1;// 摆动相 / 支撑相， 必须小于1 ， 可以取值(0.5 - 1)
float step_cycloid_phase_time = (step_rate/(step_rate+1))*step_cycle_time;//摆动相占用的周期时间,对应2*PI
float step_support_phase_time = step_cycle_time - step_cycloid_phase_time;
float trot_direction_theta = 0;//方向转角，弧度，用来调整行进方向，逆时针旋转为正角度，
int step_cycle_time_set = step_cycle_time ;//设定值，但是要等到周期结束才能让step_cycle_time起作用
float trot_step_length_set= trot_step_length;
float trot_left_step_length_rate =1;//左右步长的比例，1-0.5，用于转向，履带逻辑
float trot_right_step_length_rate =1;
int trot_mode = 0;//0是行进模式，1是旋转模式
float turn_round_theta = angle2pi(20);//旋转时一个周期旋转度数
int turn_round_direction = -1;//-1为逆时针，1为顺时针
float initPosPolar[6][2]={0};//初始足端相对身体中心的极坐标，用于围绕身体中心旋转腿
float rotate_theta_x = 0;//绕x的旋转角度，俯仰角
float rotate_theta_y = 0;//绕y的旋转角度，翻滚角
float rotate_theta_z = 0;//绕z的旋转角度，航向角

void setRotateTheta(float rx,float ry,float rz)
{
  rotate_theta_x = rx;
  rotate_theta_y = ry;
  rotate_theta_z = rz; 
}
void setTrotMode(int m)
{
  trot_mode = m;
}
void setTurnRoundDirection(int d)
{
  turn_round_direction = d;
}
void setTurnDirection(int d)
{//按下左，左边步长比例下降，右边提高
  if(d<0)//左偏转
  {
    if(trot_right_step_length_rate<TRUN_DIRECTION_RATE_MAX)
      trot_right_step_length_rate+=0.5;
    else
      trot_left_step_length_rate -=0.5;

  }
  else if(d>0)//右偏转
  {
    if(trot_left_step_length_rate<TRUN_DIRECTION_RATE_MAX)
      trot_left_step_length_rate +=0.5;
    else
      trot_right_step_length_rate -=0.5;   
  }
  else//直行
  {
    trot_left_step_length_rate = 1;
    trot_right_step_length_rate = 1;
  }

  if(trot_left_step_length_rate<0)
  {
    trot_left_step_length_rate=0;
    BeepWarning =1;
  }
  if(trot_right_step_length_rate<0){
    trot_right_step_length_rate=0;
    BeepWarning =1;
  }  
  if(trot_left_step_length_rate>TRUN_DIRECTION_RATE_MAX)trot_left_step_length_rate=TRUN_DIRECTION_RATE_MAX;
  if(trot_right_step_length_rate>TRUN_DIRECTION_RATE_MAX)trot_right_step_length_rate=TRUN_DIRECTION_RATE_MAX;
  
  Serial.printf("setStepLengthRate %f= %f\n",trot_left_step_length_rate,trot_right_step_length_rate);
}
void setDirectionTheta(float t)
{
  trot_direction_theta = t;
  Serial.printf("setDirectionTheta = %f\n",t);
}
void setTrotCycleTime(int t)
{
  if(!isTrotRunning())
    step_cycle_time = t;
  step_cycle_time_set = t;
  Serial.printf("step_cycle_time_set = %d\n",t);
}
void setTrotStepLength(float l)
{
  if(!isTrotRunning())
    trot_step_length = l;
  trot_step_length_set = l;
  Serial.printf("trot_step_length_set = %f\n",l);
}
void startTrot()
{
  if(isTrotRunning())
  {
    stopTrot();
    return;
  }
  action_stand(isStandHighMode());
  if(!isStand())
    return;
  
  reCalcCycleValue();
  trot_ready_pos(true);
  step_last_time = millis();
  servoThreadMode = 1;  
  start_trot = 1; 
}
void stopTrot()
{
  start_trot = 0; 
  servoThreadMode = 0; 
  action_stand(isStandHighMode());
}

bool isTrotRunning()
{
  if(servoThreadMode == 1 &&start_trot)
    return true;
  return false; 
}
void reCalcCycleValue()
{
  step_cycle_time = step_cycle_time_set;
  trot_step_length = trot_step_length_set;
  if(isStandHighMode())
     step_lift_high = STEP_HIGH_MAX;  
  else
     step_lift_high = STEP_HIGH;
  //相应调整
  step_cycloid_phase_time = (step_rate/(step_rate+1))*step_cycle_time;//摆动相占用的周期时间,对应2*PI
  step_support_phase_time = step_cycle_time - step_cycloid_phase_time; 
  step_cycloid_phase_r = step_lift_high/2;//摆线半径
  step_length = step_cycloid_phase_r*2*M_PI;//x = r*(t-sin(t)),t=2*PI,x=r*(2*PI-0) = r*2*PI
  trot_step_rate = trot_step_length/step_length;
}
void check_cycle_trot()
{
  cycle_count++;
  float x,y,z;
//  float xp,yp,zp;
  float theta;  
  step_cur_time = millis();
  int period = step_cur_time - step_last_time;
  if(period > step_cycle_time)  
  {
    step_last_time = step_cur_time;
    period = 0;  
    reCalcCycleValue();
    
  }
  for(int i = 0;i<6;i++)
  {
    if(trot_mode == 0)
      cycle_trot(i,period,x,y,z);
    else
      cycle_trun_round(i,period,x,y,z);

    coordinate_from_leg_to_body(i,x,y,z);   
    rotate_metrix_x(x,y,z,rotate_theta_x);
    rotate_metrix_y(x,y,z,rotate_theta_y);
    rotate_metrix_z(x,y,z,rotate_theta_z);
    coordinate_from_body_to_leg(i,x,y,z);      
    direct_postion(i,x,y,z);
  }
}

void direction_turn(int i,float& y)
{
  if(isLeftBodyLeg(i))
    y=y*trot_left_step_length_rate;
  else
    y=y*trot_right_step_length_rate;
}
//  旋转矩阵 逆时针t,顺时针为-t
//      |cos(t)   sin(t)|                   
//  R = |               |
//      |-sin(t) cos(t) |    
//由于身体左右移动的方向和腿的左右移动相反，所以真实是右转t为正 
void direction_rotate_coordinate(float& x,float&y)
{
  float t,rx,ry;
  rx = x;
  ry = y;
  t = trot_direction_theta;
  x = cos(t)*rx+sin(t)*ry;
  y = -sin(t)*rx+cos(t)*ry;
  
}

void postion_with_init_pos(int i ,float&x,float&y,float&z)
{
    x = initPos[i][0]+x;
    y = initPos[i][1]+y;
    z = initPos[i][2]+z;   
}
void trot_ready_pos(bool bRight)
{
  float x,y,z;
  setStepSpeed(3);
  for(int i = 0;i<6;i++)
  {
    x = initPos[i][0];
    y = initPos[i][1];
    z = initPos[i][2];
    if(bRight)
    {
      if(isRightLeg(i))
      {
        z = z-step_lift_high;
        set_expect_postion(i,x,y,z);
      }
    }
    else
    {
      if(isLeftLeg(i))
      {
        z = z-step_lift_high;
        set_expect_postion(i,x,y,z);
      }
    }
  }
  wait_all_reach_postion();  
  setStepSpeed(1);
}

void cycle_trot(int leg,int period,float&x,float&y,float&z)
{
   float theta;
    if(isRightLeg(leg))//右腿先抬，1，3，5
    {
      if(period <= step_cycloid_phase_time/2)
      {//摆动相 前半
        theta = (float)map(period, 0, step_cycloid_phase_time/2, M_PI*zoomRate, 2*M_PI*zoomRate)/zoomRate;
        y = (step_cycloid_phase_r*(theta-sin(theta)) - step_length/2);//半个摆动相，平移半个步长，取负值适应狗的x轴方向
        z = step_cycloid_phase_r*(1-cos(theta));     
      }
      else if(period >= (step_cycle_time-step_cycloid_phase_time/2 ))
      {//摆动相 后半
        theta = (float)map(period, (step_cycle_time-step_cycloid_phase_time/2 ), step_cycle_time, 0, M_PI*zoomRate)/zoomRate;    
        y = (step_cycloid_phase_r*(theta-sin(theta)) - step_length/2);//半个摆动相，平移半个步长，取负值适应狗的x轴方向
        z = step_cycloid_phase_r*(1-cos(theta));//后半依然是摆动相
      }
      else
      {//支撑相
        y = (float)map(period, step_cycloid_phase_time/2, step_cycle_time-step_cycloid_phase_time/2 , step_length/2*zoomRate, -step_length/2*zoomRate)/zoomRate;
        z = 0;
      }
    }
    else//左腿 0，2，4
    {
      if(period <= step_support_phase_time/2)
      {//支撑相 前半
        y = (float)map(period, 0 ,step_support_phase_time/2 , 0, -step_length/2*zoomRate)/zoomRate;
        z = 0;
      }
      else if(period >= (step_cycle_time - step_support_phase_time/2))
      {//支撑相 后半
        y = (float)map(period, (step_cycle_time - step_support_phase_time/2) ,step_cycle_time , step_length/2*zoomRate,0)/zoomRate;
        z = 0;
      }
      else
      {//摆动相,  
        theta = (float)map(period,step_support_phase_time/2, step_cycle_time-step_support_phase_time/2, 0, 2*M_PI*zoomRate)/zoomRate;        
        y = (step_cycloid_phase_r*(theta-sin(theta)) - step_length/2);
        z = step_cycloid_phase_r*(1-cos(theta));         
      }
      
    }
    x = 0;
    y = y*trot_step_rate;//限定trot步长最大为trot_step_length 
    z = -z;

    direction_turn(leg,y); 
    direction_rotate_coordinate(x,y);
    postion_with_init_pos(leg,x,y,z);   
      
}
void cycle_trun_round(int leg,int period,float&x,float&y,float&z)
{//以身体几何中心作为坐标原点，进行绕圈，再转换到6腿的坐标进行驱动
  float theta,t,r,c,tr;
  c = initPosPolar[leg][0];
  r = initPosPolar[leg][1];
  tr = turn_round_theta*turn_round_direction;
    if(isRightLeg(leg))//右腿先抬，1，3，5
    {
      if(period <= step_cycloid_phase_time/2)
      {//摆动相 前半
        t = (float)map(period, 0, step_cycloid_phase_time/2, c*zoomRate, (c+tr/2)*zoomRate)/zoomRate;

        theta = (float)map(period, 0, step_cycloid_phase_time/2, M_PI*zoomRate, 2*M_PI*zoomRate)/zoomRate;
        z = step_cycloid_phase_r*(1-cos(theta));     
      }
      else if(period >= (step_cycle_time-step_cycloid_phase_time/2 ))
      {//摆动相 后半
        t = (float)map(period, (step_cycle_time-step_cycloid_phase_time/2 ), step_cycle_time, (c-tr/2)*zoomRate, c*zoomRate)/zoomRate;           
        theta = (float)map(period, (step_cycle_time-step_cycloid_phase_time/2 ), step_cycle_time, 0, M_PI*zoomRate)/zoomRate;    
        z = step_cycloid_phase_r*(1-cos(theta));//后半依然是摆动相
      }
      else
      {//支撑相
        t = (float)map(period, step_cycloid_phase_time/2, step_cycle_time-step_cycloid_phase_time/2 , (c+tr/2)*zoomRate, (c-tr/2)*zoomRate)/zoomRate;
        z = 0;
      }
    }
    else//左腿 0，2，4
    {
      if(period <= step_support_phase_time/2)
      {//支撑相 前半
        t = (float)map(period, 0, step_support_phase_time/2, c*zoomRate, (c-tr/2)*zoomRate)/zoomRate;
        z = 0;
      }
      else if(period >= (step_cycle_time - step_support_phase_time/2))
      {//支撑相 后半
         t = (float)map(period, (step_cycle_time - step_support_phase_time/2) ,step_cycle_time , (c+tr/2)*zoomRate,c*zoomRate)/zoomRate;
         z = 0;
      }
      else
      {//摆动相,   
        t = (float)map(period,step_support_phase_time/2, step_cycle_time-step_support_phase_time/2, (c-tr/2)*zoomRate, (c+tr/2)*zoomRate)/zoomRate;        
        theta = (float)map(period,step_support_phase_time/2, step_cycle_time-step_support_phase_time/2, 0, 2*M_PI*zoomRate)/zoomRate;        
        z = step_cycloid_phase_r*(1-cos(theta));         
      }
    }
    
    x = r*sin(t);
    y = r*cos(t);
    z = -z; 
    z = initPos[leg][2]+z;   
    coordinate_from_body_to_leg(leg,x,y,z);
}

void coordinate_from_leg_to_body(int leg,float&x,float&y,float&z)
{
  if(leg == 0)
  {
    x=x-body_width/2;
    y=y+body_length/2;
  }
  else if(leg == 1)
  {
    x = x-body_center_width/2;
  }
  else if(leg == 2)
  {
    x = x-body_width/2;
    y = y-body_length/2;
  }
  else if(leg == 3)
  {
    x = x+body_width/2;
    y = y-body_length/2;
  }
  else if(leg ==4)
  {
    x = x+body_center_width/2;
  }
  else if(leg == 5)
  {
    x = x+body_width/2;
    y = y+body_length/2;
  }  
}
void coordinate_from_body_to_leg(int leg,float&x,float&y,float&z)
{//从身体为中心的坐标系变换到 每条腿的坐标系
  if(leg == 0)
  {
    x=x+body_width/2;
    y=y-body_length/2;
  }
  else if(leg == 1)
  {
    x = x+body_center_width/2;
  }
  else if(leg == 2)
  {
    x = x+body_width/2;
    y = y+body_length/2;
  }
  else if(leg == 3)
  {
    x = x-body_width/2;
    y = y+body_length/2;
  }
  else if(leg ==4)
  {
    x = x-body_center_width/2;
  }
  else if(leg == 5)
  {
    x = x-body_width/2;
    y = y-body_length/2;
  }
}

void calc_init_pos_polar_for_body()
{
  float x,y,z,t,r;
  for(int i = 0;i<6;i++)
  {
    x = initPos[i][0];
    y = initPos[i][1];
    z = initPos[i][2];
    coordinate_from_leg_to_body(i,x,y,z);
    if(i==1)
      t = -M_PI/2;
    else if(i == 4)
      t = M_PI/2;
    else{
      if(y == 0)
      {
        BeepWarning = 3;
        Serial.println("y == 0 error");
        return;
      }
      t = abs(atan(x/y));
      if(i == 0)
        t = -t;
      else if(i == 2)
        t = -(M_PI-t);
      else if(i == 3)
        t = M_PI-t;
//      else if(i == 5)
//        t = t;
    }

    if(i == 1 || i == 4)
      r = abs(x);
    else
      r = sqrt(pow(x,2)+pow(y,2));
    initPosPolar[i][0] = t;
    initPosPolar[i][1] = r;
    Serial.printf("initPosPolar i=%d,t=%f,r=%f\n",i,t,r);
  }
}

int getInitTrotCycleTime()
{
  if(isStandHighMode())
    return 1000;
  else
    return 350;
}
