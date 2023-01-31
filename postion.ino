#define KEEP 1000
#define INIT_X 80
#define INIT_Y 0
#define INIT_Z 60
#define INIT_Z_HIGH 100
volatile float postionReady[6][3];
float stepSpeed = 1;//步速,最大为6

//alpha,beta,gama分别为由身体向足端三个舵机的角度
//MG92B 0到180 逆时针旋转

void setStepSpeed(float s)
{
  stepSpeed = s;
}
float angle2pi(float a)
{
  float f;
  f = a/180*M_PI;
  return f;
}
float pi2angle(float p)
{
  float a;
  a = p/M_PI*180;
  return a;
}
bool angle_to_postion(int leg,float alpha,float beta,float gama)
{//运动学正解
  
}

bool postion_to_angle(int leg, float x, float y, float z)
{//运动学逆解

  //平面方向俯视观察，右为x正，前为y正，垂直方向，下为z正
  //原始逆解是左边中间腿为计算，其他腿相应变换角度
//  旋转矩阵 逆时针t,顺时针为-t
//      |cos(t)   sin(t)|                   
//  R = |               |
//      |-sin(t) cos(t) |              
  float t = M_PI/4;
  float x1 = x;
  float y1 = y;
  if(leg == 0||leg ==3)//坐标顺时针旋转45度
  {
    t = -t;
    x = cos(t)*x1+sin(t)*y1;
    y = -sin(t)*x1+cos(t)*y1;
  }
  else if(leg == 2||leg == 5)//坐标逆时针旋转45度
  {
    x = cos(t)*x1+sin(t)*y1;
    y = -sin(t)*x1+cos(t)*y1;
  }
  
  if(verifyCoordinate(leg,x,y,z) == false)
    return false;

  float alpha,beta,gama;
  float L2 = sqrt(pow(x,2)+pow(y,2))-La;
  float Lg = sqrt(pow(L2,2)+pow(z,2));

  float d;
  if(z==0)
    d = M_PI/2;
  else
    d = atan(L2/z);
  gama = acos((pow(Lb,2)+pow(Lc,2)-pow(Lg,2))/(2*Lb*Lc));
  float c = acos((pow(Lg,2)+pow(Lb,2)-pow(Lc,2))/(2*Lg*Lb));
  beta = d+c;

  if(x==0)
    alpha=M_PI/2;
  else
    alpha = atan(y/x);  
  //舵机的实际角度
  alpha = M_PI/2+alpha;
  beta = M_PI-beta;
  //gama 保持不变
  
  //弧度换算成角度
  alpha = pi2angle(alpha);
  beta = pi2angle(beta);
  gama = pi2angle(gama);

  //其他腿做相应变换
  if(leg == 3||leg == 4||leg == 5)//右边腿
  {
//    alpha 不变;
    beta = 180-beta;
    gama = 180-gama;
  }
  
  angle_to_leg(leg,alpha,beta,gama);
  return true;
}

void angle_to_leg(int leg ,float alpha,float beta,float gama)
{
  verifyAnagleMaxAndMin(leg,alpha,beta,gama);
  setAngle(Legs_pin[leg][0],alpha);
  setAngle(Legs_pin[leg][1],beta); 
  setAngle(Legs_pin[leg][2],gama);  
}

void angle_to_servo(int leg,int servoid,float angle)
{
  if(leg<0||leg>=6)return;
  if(servoid<0||servoid>=3)return;
  setAngle(Legs_pin[leg][servoid],angle+legOffset[leg][servoid]);
}

bool verifyCoordinate(int leg,float x,float y,float z)
{
//  if(z==0)return false;
//  if(x==0)return false;
  if(z>Lb+Lc-10)return false;
  float Ln = sqrt(pow(x,2)+pow(y,2));
  if(Ln>(La+Lb+Lc-10))return false;
  if(sqrt(pow(Ln-La,2)+pow(z,2))>(Lb+Lc -10))return false;
  
  return true;
    
}
void verifyAnagleMaxAndMin(int leg,float& alpha,float& beta,float& gama)
{
  if(alpha>180)alpha = 180;
  if(beta>180)beta = 180;
  if(gama>180)gama = 180; 
  if(alpha<0)alpha = 0;
  if(beta<0)beta = 0;
  if(gama<0)gama = 0;   

  //gama内弯到极致是45，单腿自己的结构会撞到
  if(leg <3 && gama<45)gama=45;
  if(leg >=3 && gama>135)gama=135;

  //alpha超过45，135也会两条腿打架
  if(alpha<45)alpha = 45;
  if(alpha>135)alpha =135;  
}

void  check_expect_postion()
{
  static float alpha, beta, gama;
  int nReady=0;
  for(int i=0;i<6;i++)
  {
    nReady = 0;
    for(int j=0;j<3;j++)
    {
      if(curPostion[i][j]!= expPostion[i][j])
      {
        if(abs(curPostion[i][j] - expPostion[i][j])<stepSpeed)
        {
            curPostion[i][j] = expPostion[i][j];
        }
        else{
         if(curPostion[i][j] < expPostion[i][j])
           curPostion[i][j] += stepSpeed;
         else
           curPostion[i][j] -= stepSpeed;
        }
        postionReady[i][j] = 0; 
      }
      else{
        postionReady[i][j] = KEEP;
        continue;
      }
    }
    for(int j=0;j<3;j++)
    {
      nReady += postionReady[i][j];
    }
    if(nReady == 3*KEEP)//如果1条腿的3个节点已经驱动到位，就不要再计算舵机的角度了
      continue;
//    Serial.println("check_expect_postion action");  
    postion_to_angle(i,curPostion[i][0],curPostion[i][1],curPostion[i][2]);
 
  }
}

void wait_reach_postion(int leg)
{
  while (1){
    if (curPostion[leg][0] == expPostion[leg][0]){
      if (curPostion[leg][1] == expPostion[leg][1]){
        if (curPostion[leg][2] == expPostion[leg][2]){
          break;
        }
      }
    }
    vTaskDelay(10);
  }
}

void wait_all_reach_postion()
{
//  checkPostionOrAngle = 1;
  for (int i = 0; i < 6; i++)
    wait_reach_postion(i);
}

bool is_all_reach_postion()
{
  for (int i = 0; i < 6; i++)
  {
    if (curPostion[i][0] != expPostion[i][0])
       return false;
    if (curPostion[i][1] != expPostion[i][1])
       return false;
    if (curPostion[i][2] != expPostion[i][2])
       return false;   
  }
  return true;
}

void set_current_postion(int leg, float x, float y, float z)
{
  curPostion[leg][0] = x;
  curPostion[leg][1] = y;
  curPostion[leg][2] = z;    
}

void set_expect_postion(int leg, float x, float y, float z)
{
  if(x != KEEP)
    expPostion[leg][0] = x;
  if(y != KEEP)  
    expPostion[leg][1] = y;
  if(z != KEEP)   
    expPostion[leg][2] = z;     
}

void direct_postion(int nLeg,float x,float y,float z)
{
    if(postion_to_angle(nLeg,x,y,z))
    {
      set_current_postion(nLeg,x,y,z);
      set_expect_postion(nLeg,x,y,z);      
    }
}

void direct_all_postion(float x,float y,float z)
{
  for (int i = 0; i < 6; i++)
  {
    direct_postion(i,x,y,z);
  }
}

void initPostion(bool bHigh)
{
  float x = INIT_X;
  float y = INIT_Y;
  float z = INIT_Z;
  if(bHigh)
    z = INIT_Z_HIGH;
  float t = M_PI/4;
  float rx = sqrt(pow(x,2)+pow(y,2))*cos(t);
  float ry = sqrt(pow(x,2)+pow(y,2))*sin(t);
  for(int i = 0;i<6;i++){
    if(i==0){
      initPos[i][0]=-rx;
      initPos[i][1]= ry;
    }
    else if(i==1)
    {
      initPos[i][0]=-x;
      initPos[i][1]= y;     
    }
    else if(i==2)
    {
      initPos[i][0]= -rx;
      initPos[i][1]= -ry;        
    }
    else if(i==3)
    {
      initPos[i][0]= rx;
      initPos[i][1]= -ry;         
    }
    else if(i==4)
    {
      initPos[i][0]= x;
      initPos[i][1]= y;        
    }
    else if(i==5)
    {
      initPos[i][0]= rx;
      initPos[i][1]= ry;       
    }

    initPos[i][2]= z;      
  }
  calc_init_pos_polar_for_body();
  for(int i=0;i<6;i++)
  {
    direct_postion(i,initPos[i][0],initPos[i][1],initPos[i][2]);
  }
}

void set_rel_exp_pos(int leg,float x,float y,float z)
{//相对当前的坐标偏移
  float rx = curPostion[leg][0]+x;
  float ry = curPostion[leg][1]+y;
  float rz = curPostion[leg][2]+z;    
  set_expect_postion(leg,rx,ry,rz);
}
void set_rel_exp_pos_all(float x,float y,float z)
{
  for(int i=0;i<6;i++)
  {
      set_rel_exp_pos(i,x,y,z);
  }
}
bool isRightLeg(int leg)
{
  if(leg == 1||leg == 3||leg == 5)
    return true;
  return false;
}
bool isLeftLeg(int leg)
{
  if(leg == 0||leg == 2||leg == 4)
    return true;
  return false;
}

bool isRightBodyLeg(int leg)
{
  if(leg == 3||leg == 4||leg == 5)
    return true;
  return false;  
}
bool isLeftBodyLeg(int leg)
{
  if(leg == 0||leg == 1||leg == 2)
    return true;
  return false;  
}

float getStandHigh()
{
  if(isStandHighMode())
    return INIT_Z_HIGH;
  else
    return INIT_Z;
}


//      [1  0       0     ]
//Rx(t)=[0  cos(t) -sin(t)]
//      [0  sin(t) cos(t) ]  
void rotate_metrix_x(float &x,float&y,float&z,float t)
{//P_new = P*Rx(t)
  float rx,ry,rz;
  rx=x;ry=y;rz=z;
  x=rx;
  y=ry*cos(t)-rz*sin(t);
  z=ry*sin(t)+rz*cos(t);
}

//      [cos(t)   0   sin(t)]
//Ry(t)=[0        1   0     ]
//      [-sint(t) 0   cos(t)] 
void rotate_metrix_y(float &x,float&y,float&z,float t)
{//P_new = P*Ry(t)
  float rx,ry,rz;
  rx=x;ry=y;rz=z;
  x=rx*cos(t)+rz*sin(t);
  y=ry;
  z=-rx*sin(t)+rz*cos(t);
}

//      [cos(t) -sin(t) 0]
//Rz(t)=[sin(t)  cos(t) 0]
//      [0       0      1] 
void rotate_metrix_z(float &x,float&y,float&z,float t)
{//P_new = P*Rz(t)
  float rx,ry,rz;
  rx=x;ry=y;rz=z;
  x=rx*cos(t)-ry*sin(t);
  y=rx*sin(t)+ry*cos(t);
  z=rz; 
  
}
