#include <Servo.h>
#define L_1 10.4
#define L_2 9.75
#define L_3_FIXED 5
#define N 3   // 3x3 grid
#define W 1.2  // cube width
#define fast 10

float  L_3 ;
int fi;
int old_omega;
int old_beta;
int old_alfa;
int old_gamma;
int set_a, set_b;  // row and column of the matrix

typedef struct {
float asc;
float ord;
char box;
} matrix;

typedef matrix Grid[N][N];

Grid tris;

int inclination_values[]={29,55,80};

Servo servo1;  // omega
Servo servo2;  // beta
Servo servo3;  // alfa
Servo servo4;  // gamma
Servo servo_gripper;  //fi

// functions
void init_tic_tac_toe( Grid ); 
int check_winner( Grid g, char ch );
int count_empty_cells( Grid g );
int move_tic_tac_toe( Grid g, char simbolo );
int two_in_line( Grid grid, char ch );
int save_yourself( Grid grid, int i, int flag);
int matlab (void);
void pick (void);

int maximum (int, int, int, int );
void trajectory(int, int , int , int); // it is a way to make all the joints starting and finishing at the same time
void move_omegam(int);
void move_betam(int);
void move_alfam(int);
void move_gammam(int);
float alfa (float, float);
float beta (float, float);
float omega (float, float );
float fhi (float );
void calculate_L3( float );
void reach (float, float, float);
void reach_theta(float, float, float, float); // here i can specify also the orientation of the last joint wrt to the object
void grab (float, float, float, float, float);
void disengage (float );
int check_theta(float , float );
int check (float , float );
float check_3gdl(float , float );


void setup() {

Serial.begin(9600);

servo1.attach(3);
servo2.attach(6);
servo3.attach(9);
servo4.attach(11);
servo_gripper.attach(10);

old_omega=servo1.read();
old_beta=servo2.read();
old_alfa=servo3.read();
old_gamma=servo4.read();
fi=servo_gripper.read();


// set the coordinates of the chess squares

// row 1

tris[0][0].asc=-7.8;
tris[0][1].asc=-0.8;
tris[0][2].asc=6.6;

tris[0][0].ord=23;
tris[0][1].ord=23;
tris[0][2].ord=22.8;

// row 2 

tris[1][0].asc=-9.1;
tris[1][1].asc=-1;
tris[1][2].asc=6.7;

tris[1][0].ord=16.4;
tris[1][1].ord=16.1;
tris[1][2].ord=16.4;

// row 3

tris[2][0].asc=-10;
tris[2][1].asc=-1.1;
tris[2][2].asc=7;

tris[2][0].ord=11;
tris[2][1].ord=9.6;
tris[2][2].ord=10.5;

// init
calculate_L3( W );
reach(22,3,11); // start position must be above the pick-up point
delay(2000);
disengage(W);
delay(3000);

}

void loop() { 

  int game_over = 0;
 
  init_tic_tac_toe( tris );

  while( ! game_over ) {
                        game_over = move_tic_tac_toe( tris, 'O' );
                        if( ! game_over )
                        game_over = move_tic_tac_toe( tris, 'X' );
                       }
  delay(30000);                   
}


void init_tic_tac_toe( Grid grid ) {
  int i, j;
  for( i=0; i<N; i++ )
    for( j=0; j<N; j++ )
      grid[i][j].box = ' ';
}

int count_empty_cells( Grid grid ) {
  int i, j, vuote=0;
  for( i=0; i<N; ++i )
    for( j=0; j<N; ++j )                      
      if( grid[i][j].box == ' ' )
        vuote++;
  return vuote;
}

int check_winner( Grid grid, char ch ) {
  
  int i;
  int j=0;
  
  for( i=0; i<N; i++ ) 
  
  if (grid [i] [j].box ==ch && grid [i] [j+1].box ==ch &&  grid [i] [j+2].box == ch)   return 2;
  
  i=0;
  j=0;
  
  for (i=0; i<N; i++ )  
  
  if (grid [j] [i].box ==ch && grid [j+1] [i].box==ch &&  grid [j+2] [i].box == ch)   return 2; 
  if ( grid [0] [0].box== ch && grid  [1][1].box==ch && grid [2] [2].box==ch)   return 2;  
  if ( grid [0] [2].box== ch && grid  [1][1].box==ch && grid [2] [0].box== ch)  return 2;
  
  return 0;

}

int move_tic_tac_toe( Grid grid, char ch) {
  
  int x,tasto=0;
    
  if (ch == 'O') {   // human move

  Serial.println(1);
  delay(250);

  do {
     
     tasto = matlab(); // get human move from matlab code
    
     set_a = ceil( float(tasto)/3 )-1;
     set_b = (tasto-1)%3;  
    
     } while ( !tasto || (grid[set_a][set_b].box != ' ')  );
  
  grid[set_a][set_b].box = 'O';
    
                }
    
    
  else {   // robot move
  
    
  if (  !two_in_line(grid, 'X')   &&  !two_in_line(grid, 'O') ) {    // if human and robot are not going to win, then move randomly

  
  do {
      set_a=random(3);    
      set_b=random(3);
 
     } while (!(set_a<3 && set_a>=0 && set_b<3 && set_b>=0) || (grid [set_a][set_b].box != ' ') );
  
                                                                }
                                                                
   grid[set_a][set_b].box ='X';

   pick();
 
   if ( !(set_a==2 && set_b ==1 ) &&  !(set_a==2 && set_b ==2 )    )   reach( grid[set_a][set_b].asc , grid[set_a][set_b].ord , 10);         // move the cube in the new position
   else   reach_theta( grid[set_a][set_b].asc , grid[set_a][set_b].ord , 10, 45);                          
   
   reach_theta( grid[set_a][set_b].asc , grid[set_a][set_b].ord , -0.1, inclination_values[set_a] );         //lower the ball    
                               
   disengage(W);    // release the ball
                               
   if (  !(set_a==2 && set_b ==1 ) &&  !(set_a==2 && set_b == 2)  ) reach( grid[set_a][set_b].asc , grid[set_a][set_b].ord , 10);          // raise the arm up
   else   reach_theta( grid[set_a][set_b].asc , grid[set_a][set_b].ord , 10, 45);

   reach( 22 , 3 , 11);          //   come back above the pick-up point 
   delay(1500);
   
       }

//  Serial.print(set_a);
//  Serial.print("     ");
//  Serial.print(set_b);
//  Serial.print("     ");
//  Serial.println(grid[set_a][set_b].box);

  x=check_winner(  grid , ch );
  
  if (x==2) return 1; // vittoria
              
  else if ( (count_empty_cells( grid )==0) && x==0) return 1;  // pair
                                                 
  else return 0;
  
}


int two_in_line( Grid grid, char ch ) {    
  
  int i,j,cont=0;
  
  for( i=0; i<N; i++ ) { // horizontal check

  for( j=0; j<N; j++ ) {
                         
                 if (grid [i] [j].box==ch)  cont++;
                         
                             if (cont==2) {
                                          if ( save_yourself(grid, i, 0) ) return 1;   
                                          }
                         }
     cont=0;                     
                          }
                                               
    cont=0, i=0, j=0;
  
  for( j=0; j<N; j++ ) {   // vertical check
    
  for( i=0; i<N; i++ )  {
                         
                if (grid [i] [j].box==ch)  cont++;
                         
                            if (cont==2) {
                                          if ( save_yourself(grid, j, 1) ) return 1;
                                         }
                          }
   cont=0;                      
                       }
                       
  cont=0; 
                     
  for( i=0; i<N; i++ ) {     // diagonal check
                         
                 if (grid [i][i].box==ch)  cont++;
                         
                             if (cont==2) {
                                          if ( save_yourself(grid, i, 2) ) return 1;  
                                          }
                         }
                         
    cont=0;
     
  for( i=0; i<N; i++ ) {     // diagonal check (opposite diagonal)
                         
                 if (grid [i][2-i].box==ch)  cont++;
                         
                             if (cont==2) {
                                          if ( save_yourself(grid, i, 3) ) return 1;  
                                          }
                         }
  
  
  
return 0;   
  
  
}

int save_yourself( Grid grid, int i, int flag) { // human is going to win, protect yourself to block him
  
  int j;
  
  if (flag==0) {

  for( j=0; j<N; j++ )
  if (grid[i][j].box==' ') {
                       set_a=i;
                       set_b=j;
                       //grid[i][j]='X';
                       return 1;
                      }
               }
  
  else if  (flag==1)  {

  for( j=0; j<N; j++ )
  if (grid[j][i].box==' ')  {
                         set_a=j;
                         set_b=i;
                         //grid[j][i]='X';
                         return 1;
                          }
  
                      }

  else if  (flag==2)  {

  for( j=0; j<N; j++ )
  if (grid[j][j].box==' ')  {
                         set_a=j;
                         set_b=j;
                         //grid[j][j]='X';
                         return 1;
                          }
  
                      }
    
  else if  (flag==3)  {

  for( j=0; j<N; j++ )
  if (grid[j][2-j].box==' ')  {
                          set_a=j;
                          set_b=2-j;
                          //grid[j][2-j]='X';
                          return 1;
                          }
  
                      } 
    
  return 0; 
  
}

int matlab (void) {

while ( !Serial.available() ) {};
return Serial.read();
  
}

void pick (void) {
                       
  grab(22 , 3 , 2.2 , W , 54);     // lower the claw and grab the ball
                               
  reach(21 , 2 , 10);         // raise the ball up
                                 
  }


void calculate_L3( float D ) {
  
  float f=fhi(D);   // D: length of the cube
 
  float lun_1=2.85; 
  float lun=3;  
  float lun_2=lun*sin(f);   
  float lun_3=2.1; 
  float lun_pinza=lun_1+lun_2+lun_3;

  L_3 = L_3_FIXED + lun_pinza;  
  L_3= L_3*L_3 + 3*3; 
  L_3=sqrt(L_3);  // new link length considering gripper opening
  
  }


void grab ( float x, float y, float z, float D, float teta) {

  int prec=fi;
  float f=fhi(D) *180/M_PI;    // deg
  fi=floor(f+0.5);

if (teta == -1) reach(x, y, z); 
else reach_theta(x, y, z, teta);

if (prec <= fi)   servo_gripper.write(fi+6);            
else  servo_gripper.write(fi+10);  

delay(700); 
  
}

void disengage (float D) {
  
float f=fhi(D) *180/M_PI;    // deg
fi=floor(f+0.5);

servo_gripper.write(fi-38);   //  +8 as gap
delay(800);
  
}


void reach_theta ( float x, float y, float z, float teta) {

float a,b,c,d,g=0;
int alfam, betam, gammam, omegam;

float gap=180/M_PI*acos(3.2/L_3);
float omega_x=x;
float omega_y=y;
x=sqrt(x*x+y*y);
y=z;

float A= x - L_3*cos(teta*M_PI/180);
float B= y +  L_3*sin(teta*M_PI/180);
g=270-teta-alfa_teta( A , B)- beta_teta(A, B);     // g contains gamma value
if (g<0) g+=360;

if (   check_theta (A , B)  && g-90+gap >= 40  &&  g <= 180 ) {
                                                   a= beta_teta( A, B );   //beta
                                                   b= alfa_teta(A, B )+90;   //alfa
                                                   c=g;  // gamma
                                                   c=-90+c+gap; 
                                                   d= omega(omega_x, omega_y);  //omega     
                                                   
                                                   betam=floor(a+0.5);
                                                   alfam=floor(b+0.5);
                                                   gammam=floor(c+0.5);
                                                   omegam=floor(d+0.5);  

                                                   /*Serial.println(L_3);
                                                   Serial.println(betam);
                                                   Serial.println(alfam);
                                                   Serial.println(gammam);
                                                   Serial.println(omegam);*/
                                                                                                    
                                                   trajectory(omegam, betam, alfam , gammam);

                                                   delay(800);
                                                   
                                                   } 
 
}


void reach ( float x, float y, float z) {

float a,b,c,d,w=0;
int alfam, betam, gammam, omegam;

 float gap=180/M_PI*acos(3.2/L_3); 
 float omega_x=x;
 float omega_y=y;
 x=sqrt(x*x+y*y);
 y=z;

 w=check_3gdl(x,y);
 
 float X= x - L_1*cos(w*M_PI/180);
 float Y= y -  L_1*sin(w*M_PI/180); 
 
 if (w!=-1  &&  alfa( X , Y)+gap >= 40  ) {

            a= w;   //beta
            b= 180+beta( X , Y)-w;  //alfa
            c= alfa( X , Y)+90;  //gamma     
            c=c-90+gap;   
            d= omega(omega_x, omega_y); //omega  

            betam=floor(a+0.5);
            alfam=floor(b+0.5);
            gammam=floor(c+0.5);
            omegam=floor(d+0.5);  
            
            /*Serial.println(betam);
            Serial.println(alfam);
            Serial.println(gammam);
            Serial.println(omegam);*/
                                                                                                    
            trajectory(omegam, betam, alfam , gammam);

            delay(800);
            
            } 
 
}


int maximum (int o, int b, int a, int g ) {

int massimo_1, massimo_2;
 
if (o>b) massimo_1=o;
else massimo_1=b;

if (a>g) massimo_2=a;
else massimo_2=g;

if (massimo_1 > massimo_2) return massimo_1;
else return massimo_2;

}


void trajectory(int omegam, int betam , int alfam , int gammam) {

double  i=old_omega;
double  j=old_beta;
double  s=old_alfa;
double  t=old_gamma;

int delta_o = abs(old_omega - omegam);
int delta_b = abs(old_beta - betam);
int delta_a = abs(old_alfa - alfam);
int delta_g = abs(old_gamma - gammam);

int delta_max= maximum (delta_o , delta_b , delta_a , delta_g);

int flag_omega=0,flag_beta=0,flag_alfa=0, flag_gamma=0;


 while ( !flag_omega || !flag_beta || !flag_alfa  || !flag_gamma ) {
  
  
  if (floor(i) < omegam && !flag_omega) {     move_omegam(floor(i));   i+=(double )delta_o/delta_max;   }
  
  else if (floor(i) > omegam && !flag_omega) {   move_omegam(floor(i));   i-=(double )delta_o/delta_max;    }
  
  else  {   move_omegam(i);  flag_omega=1;   }
  
  delay(1);
  
  if (floor(j)< betam && !flag_beta) {   move_betam(floor(j));  j+=(double )delta_b/delta_max;  }
  
  else if (floor(j)> betam && !flag_beta) {   move_betam(floor(j));  j-=(double )delta_b/delta_max;    }
  
  else {  move_betam(j);   flag_beta=1;  }

  delay(1);
  
  if (floor(s)< alfam && !flag_alfa) {   move_alfam(floor(s));  s+=(double )delta_a/delta_max;  }
  
  else if ( floor(s)> alfam && !flag_alfa) {   move_alfam(floor(s));  s-=(double )delta_a/delta_max;    }
  
  else {  move_alfam(s);   flag_alfa=1;  }

  delay(1);
  
  if (floor(t) < gammam && !flag_gamma )   {   move_gammam(floor(t));  t+=(double )delta_g/delta_max;  }
  
  else if (floor(t)> gammam && !flag_gamma )   {   move_gammam(floor(t));  t-=(double )delta_g/delta_max;    }
  
  else {  move_gammam(t);   flag_gamma=1;   }

  
  delay(fast);
 
}
  
  } 


void move_omegam(int i){

if (i <=160 ) {   i=i+6;   servo1.write(i);   }
else if  (i > 160 ) { i=i+map(i, 160,180, 4,0);   servo1.write(i);     }

old_omega=i;  
   
  }

void move_betam(int i){

servo2.write(i); 

old_beta=i;
  
  }

void move_alfam(int i){
    
servo3.write(180-i); 

old_alfa=i; 
      
  }

void move_gammam(int i){
  
servo4.write(180-i);  

old_gamma=i;  
       
  }


float fhi (float D ) {
  
  float lun= 3; 
  float val=D/(2*lun);
  float angolo=acos(val);
  
  return angolo;  // rad
  
  }


float alfa (float x, float y) {
  
float val1 = (x*x+y*y-L_2*L_2-L_3*L_3) / (2*L_2*L_3);
float angolo=asin(val1);

return angolo*180/M_PI;     // rad
  
}

float beta (float x, float y) {
  
float t= x*x + y*y;
float LL_2=L_2*L_2;
float LL_3=L_3*L_3;

float r=  t - LL_2 - LL_3 ;  
float rad=sqrt(4*LL_2*LL_3 - r*r);

float val = (-y*rad + x*( t + LL_2 - LL_3 ) ) / (2*L_2*t);
float angolo=acos(val);
  
return angolo*180/M_PI; 

}


float omega (float x, float y) {
  
float t= x*x + y*y;
float rad=sqrt(t);

if (x==0 && y==0) return 0;

float angolo=acos(x/rad);
  
return angolo*180/M_PI; 

}

float check_3gdl (float x, float y)  {
  
float b;

for (b=0; b<=180; b+=0.1) 

if ( check ( x - L_1*cos(b*M_PI/180), y -  L_1*sin(b*M_PI/180) )    &&  90 -b + beta( x - L_1*cos(b*M_PI/180) ,  y -  L_1*sin(b*M_PI/180) ) <= 90.005 )    return b;
                                                                               
return -1;  
  
}

int check (float x, float y) {

float t= x*x + y*y;
float R_3 = sqrt(3);
float Rlim = (L_2*L_2) + (L_3*L_3) - (R_3*L_2*L_3) ;

if (t < Rlim || y < -8   ) return 0; 

if (y>=0)   

if (t<= ((L_2 + L_3)*(L_2 + L_3)) && (x+L_2)*(x+L_2) + y*y >= L_3*L_3) return 1;
else return 0;

if (y<0)   

if ( (x-L_2)*(x-L_2) + y*y <= L_3*L_3  )return 1;
else   return 0;
  
}

float alfa_teta (float x, float y) {
  
float val1 = (x*x+y*y-L_1*L_1-L_2*L_2) / (2*L_1*L_2);
float angolo=asin(val1);

return angolo*180/M_PI;     
  
}

float beta_teta (float x, float y) {
  
float t= x*x + y*y;
float LL_1=L_1*L_1;
float LL_2=L_2*L_2;

float r=  t - LL_1 - LL_2 ;  
float rad=sqrt(4*LL_1*LL_2 - r*r);

float val = (-y*rad + x*( t + LL_1 - LL_2 ) ) / (2*L_1*t);
float angolo=acos(val);
  
return angolo*180/M_PI; 

}

int check_theta (float x, float y) {
  
float t= x*x + y*y;
float R_3 = sqrt(3);
float Rlim = (L_1*L_1) + (L_2*L_2) - (R_3*L_1*L_2) ;

if (t < Rlim  || y < -8  ) return 0; 

if (y>=0)   

if (t<= ((L_1 + L_2)*(L_1 + L_2)) && (x+L_1)*(x+L_1) + y*y >= L_2*L_2) return 1;
else return 0;    

if (y<0)   

if ( (x-L_1)*(x-L_1) + y*y <= L_2*L_2  ) return 1;
else return 0;  
}