// THINGS WE NEED TO DO:
// - Implement RPS
// - Timeout on function


// Dependencies
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHWONKA.h>
#include <FEHMotor.h>
#include <cmath>

// Positioning and heading
FEHWONKA RPS;

// Motor ports
FEHMotor leftMotor(FEHMotor::Motor0);
FEHMotor rightMotor(FEHMotor::Motor1);

// Lift motor
FEHMotor liftMotor(FEHMotor::Motor2);

// CdS cell on robot bottom
AnalogInputPin cds(FEHIO::P0_0);

// Declare shaft encoders
FEHEncoder leftEncoder(FEHIO::P1_0);
FEHEncoder rightEncoder(FEHIO::P0_2);
FEHEncoder liftEncoder(FEHIO::P1_4);

// Digital microswitch for lift
DigitalInputPin liftBottomSwitch (FEHIO::P2_0);
DigitalInputPin backButtonRight (FEHIO::P2_2);
DigitalInputPin backButtonLeft (FEHIO::P2_1);



// ***************************ROBOT SETTINGS**************************

#define BREAKTIME .3

// CdS cell
#define CDS_THRESHOLD 0.4

// Motor speeds
#define LEFT_MOTOR_SPEED_HI 80
#define RIGHT_MOTOR_SPEED_HI 80

#define LEFT_MOTOR_SPEED_MID 59
#define RIGHT_MOTOR_SPEED_MID 60

#define LEFT_MOTOR_SPEED_LO 55 // PREVIOUSLY 55
#define RIGHT_MOTOR_SPEED_LO 55

// Lift motor
#define LIFT_SPEED_UP 40
#define LIFT_SPEED_DOWN 45

#define LIFT_COUNTS_TO_MIDDLE 4
#define LIFT_COUNTS_TO_TOP 10

// Encoder settings
#define COUNTS_PER_WHEEL 32.0
#define WHEEL_CIRCUMFERENCE 2.75 * 3.14159

#define COUNTS_TO_PIVOT 20
#define EXTRA_TURN_TIME 0

#define ENCODER_CORRECT 0



// ***********************FUNCTIONS*************************************


// Reset the shaft encoders
void resetEncoders()
{
    leftEncoder.ResetCounts();
    rightEncoder.ResetCounts();
    liftEncoder.ResetCounts();
}

// Stop moving robot
void stop()
{
    // Set both motors to 0 percent speed
    leftMotor.SetPercent(0);
    rightMotor.SetPercent(0);
}

// Drive both directions
void drive(float distance, int direction)
{
    // Reset the encoders
    resetEncoders();

    float distancePerCount = WHEEL_CIRCUMFERENCE / COUNTS_PER_WHEEL;


    // Calculations for distance
    int numberOfCounts = std::floor(distance / distancePerCount) - ENCODER_CORRECT;


    // Set the motor speed percentages to medium and direction
    leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO * direction);
    rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO * direction);

    // Drive specified number of counts
    while(leftEncoder.Counts() <= numberOfCounts && rightEncoder.Counts() <= numberOfCounts){
    }

    // Stop the motors
    stop();
}

// Readable function names for driving
void driveForward(double distance)  {
    if(distance > 0)
    {
        // Distance is specified
        drive(distance, 1);
    }
    else
    {
        // Drive continuously
        leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO);
        rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO);
    }
}
void driveBackward(double distance) {
    if(distance > 0)
    {
        drive(distance, -1);
    }
    else
    {
        // Drive continuously
        leftMotor.SetPercent(-1 * LEFT_MOTOR_SPEED_LO);
        rightMotor.SetPercent(-1 * RIGHT_MOTOR_SPEED_LO);
    }
}

// Pivot the robot in place
void pivot(int direction)
{
    // Reset the encoders
    resetEncoders();

    // Check which direction to pivot
    if(direction > 0)
    {
        // TURN LEFT - set motor speeds and directions
        leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO * -1);
        rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO);
    }
    else
    {
        // TURN RIGHT - set motor speeds and directions
        leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO);
        rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO * -1);
    }

    // Wait for proper number of encoder counts
    while(leftEncoder.Counts() <= COUNTS_TO_PIVOT && rightEncoder.Counts() <= COUNTS_TO_PIVOT);

    Sleep(EXTRA_TURN_TIME);

    // Stop turning
    stop();
}

// Readable function names for pivoting
void pivotRightTurn() { pivot(0); }
void pivotLeftTurn()  { pivot(1); }

//---------------------------------RPS Methods experimential-------------------------------------------------------

int isCloseTo(double a, double b)
{
    if (std::abs(a - b) < 4) return 1;
    return 0;
}

// Pivot the robot in place
void pivotRPS(int direction)
{
    // Reset the encoders
    resetEncoders();

    double startHeading = RPS.Heading();

    if (startHeading != 0 && RPS.Heading() != 0) {
        LCD.WriteLine(RPS.Heading());

        if (direction < 1) {
            // Turn left
            if (startHeading < 90) {
                startHeading += 90;
            } else {
                startHeading -= 90;
            }

            // Turn
            while(!isCloseTo((double)(startHeading), (double)(RPS.Heading()))) {
                if (RPS.Heading() < startHeading) {
                    leftMotor.SetPercent(-40);
                    rightMotor.SetPercent(40);
                    Sleep(.2);
                    stop();
                } else {
                    leftMotor.SetPercent(40);
                    rightMotor.SetPercent(-40);
                    Sleep(.2);
                    stop();
                }
            }
        } else {
            // Turn right
            if(startHeading > 90) {
                startHeading += 90;
            } else {
                startHeading -= 90;
            }

            // Turn
            while(!isCloseTo((double)(startHeading), (double)(RPS.Heading()))) {
                if (RPS.Heading() < startHeading) {
                    leftMotor.SetPercent(40);
                    rightMotor.SetPercent(-40);
                    Sleep(.2);
                    stop();
                } else {
                    leftMotor.SetPercent(-40);
                    rightMotor.SetPercent(40);
                    Sleep(.2);
                    stop();
                }
            }
        }
    } else {
        // Check which direction to pivot
        if(direction > 0)
        {
            // TURN LEFT - set motor speeds and directions
            leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO * -1);
            rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO);
        }
        else
        {
            // TURN RIGHT - set motor speeds and directions
            leftMotor.SetPercent(LEFT_MOTOR_SPEED_LO);
            rightMotor.SetPercent(RIGHT_MOTOR_SPEED_LO * -1);
        }

        // Wait for proper number of encoder counts
        while(leftEncoder.Counts() <= COUNTS_TO_PIVOT && rightEncoder.Counts() <= COUNTS_TO_PIVOT);

        Sleep(EXTRA_TURN_TIME);
    }

    stop();



//    while(startHeading != 0 && RPS.Heading() != 0)
//    {
//        LCD.WriteLine(RPS.Heading());

//        if(direction == 1)
//        {
//            if(startHeading < 90)
//            {
//                startHeading += 90;
//            }
//            else
//            {
//                startHeading -= 90;
//            }
//            if(isCloseTo((double)(startHeading), (double)(RPS.Heading()))) break;
//            if(RPS.Heading()<startHeading)
//            {    leftMotor.SetPercent(-40); rightMotor.SetPercent(40); Sleep(.2); stop();}
//            else {leftMotor.SetPercent(40); rightMotor.SetPercent(-40); Sleep(.2); stop();}
//        }
//        else
//        {
//            if(startHeading>90)
//            {
//                startHeading+=90;
//            }
//            else
//            {
//                startHeading-=90;
//            }
//            if(isCloseTo(startHeading, RPS.Heading())) break;
//            if(RPS.Heading()<startHeading)
//            {    leftMotor.SetPercent(-40); rightMotor.SetPercent(40); Sleep(.2); stop();}
//            else {leftMotor.SetPercent(40); rightMotor.SetPercent(-40); Sleep(.2); stop();}
//        }
//    }

}

// Readable function names for pivoting
void pivotRightTurnRPS() { pivot(0); }
void pivotLeftTurnRPS()  { pivot(1); }



//-------------------------------End RPS ---------------------------------------------------------------------------


// Set the lift height
void liftHeight(int clicks)
{
    ButtonBoard buttons(FEHIO::Bank3);

    // Reset the encoders
    resetEncoders();

    if(clicks < 1)
    {
        liftMotor.SetPercent(LIFT_SPEED_DOWN * -1);
        while(liftBottomSwitch.Value() > 0);
        liftMotor.SetPercent(0);

        resetEncoders();
    }
    else
    {
        liftHeight(0);

        liftMotor.SetPercent(LIFT_SPEED_UP);

        // Move for number of counts
        while(liftEncoder.Counts() <= clicks)
        {
            if(buttons.LeftPressed()) break;
            //LCD.WriteLine(liftSensor.Value());

        }

        // Stop moving
        liftMotor.SetPercent(0);
    }
}

// Back up until both sensors are hitting the wall
void reverseToWall()
{
    driveBackward(0);
    while(backButtonRight.Value() == 1 && backButtonLeft.Value() == 1) {
    }
    Sleep(.5);
    stop();
}

void reverseToWallHigh()
{
    leftMotor.SetPercent(-78); rightMotor.SetPercent(-78);
    while(backButtonRight.Value() == 1 && backButtonLeft.Value() == 1) {
    }
    Sleep(.5);
    stop();
}

void takeBreak()
{
    Sleep(BREAKTIME);
}
//End functions

//---------------------------------------------------------------------------------------------------------

//START MAIN

// Main function
int main(void)
{
    ButtonBoard buttons(FEHIO::Bank3);

    //Declare variables
    double nolight;
    int blue;
    int parity;
    double startTime;

    // Configure shaft encoders
    float lowThreshold = 0.388;
    float highThreshold = 1.547;

    float lowThresholdLift = 0.4;
    float highThresholdLift = 1.7;

    leftEncoder.SetThresholds(lowThreshold, highThreshold);
    rightEncoder.SetThresholds(lowThreshold, highThreshold);
    liftEncoder.SetThresholds(lowThresholdLift, highThresholdLift);

    // Reset screen
    LCD.Clear(FEHLCD::Black);
    LCD.SetFontColor(FEHLCD::White);

    // Initialize the positioning system
  //   RPS.InitializeMenu();
   //  RPS.Enable();
/*
     while(true)
     {
         if(backButtonLeft.Value()==0)
         {
             Sleep(.8);
             pivotLeftTurnRPS();
         }
         if(backButtonRight.Value()==0)
         {
             Sleep(.8);
             pivotRightTurnRPS();
         }
     }
*/


    //The program goes for the objectives in this order: pin, skid, read light, drop skid, drop scoop, flip switch, BUTTON NEEDS IMPLEMENTATION, Charger
    //
    //          SPACE FOR ERROR LOGGING AND NOTES
    //
    //
    //
    // Main program loop
    while(true)
    {
        // Wait for the left button to be pressed before starting
        while(!buttons.MiddlePressed());
        while(buttons.MiddlePressed());
        //goto segment4;

        //SEgment one will pick up the skid
        //segment1:
            liftHeight(6);

            // Move into waitForCdS()
            nolight = cds.Value(); //Wait for the light

            double startTime = TimeNow();
            while((std::abs(nolight - cds.Value()) < CDS_THRESHOLD) && (TimeNow() - startTime < 30)) {
                LCD.WriteLine(cds.Value());
            }

            double veryStart = TimeNow();

            // Drive to in front of the skid Step 1
            driveForward(35.0);
            // RPS CHECK X

            pivotLeftTurnRPS();
            reverseToWall();

            driveForward(4.0);
            // CHECK RPS Y

            pivotRightTurnRPS();
            driveForward(4.0);

            //liftHeight(5);
            liftMotor.SetPercent(50);
            Sleep(.82);
            liftMotor.SetPercent(0);


            driveBackward(2);
            takeBreak();
            pivotLeftTurn();

            liftHeight(0);

            driveBackward(0);
            startTime = TimeNow();
            while(TimeNow()-startTime<4.0) {             //Line up with the skid
                if(backButtonLeft.Value() == 1 && backButtonRight.Value() == 0) { stop(); break; }
                else if(backButtonLeft.Value() == 1 && backButtonRight.Value() == 1) { driveBackward(0); LCD.WriteLine("WHY!"); }
                else if(backButtonLeft.Value() == 0 && backButtonRight.Value() == 0) {
                    LCD.WriteLine("DrivingLoop");
                    driveForward(2);
                    leftMotor.SetPercent(50); Sleep(.2);
                    driveBackward(0);
                    Sleep(.4);
                }
            }

            rightMotor.SetPercent(40); Sleep(.1);
            rightMotor.SetPercent(0);

            //Pick up skid step 6
            driveForward(17);
            liftMotor.SetPercent(40);
            takeBreak();

            //Navigate down ramp and yada yada step 9
            driveBackward(10);
            liftMotor.SetPercent(0);
            pivotLeftTurn();
            driveForward(9);     //Now in the center
            takeBreak();
            pivotRightTurn();    //Turn so as to go down ramp

            //goto menu;           //Segment 2 will Read the light

//segment2:
  Sleep(.2);
            reverseToWall();     //Drive back until the counter is hit

            parity = 1;      //Find the light
            nolight=cds.Value();
            startTime = TimeNow();
            while(std::abs(nolight - cds.Value()) < 0.2 && (TimeNow()-startTime)<3.0) {
                if(parity) {
                    driveForward(2);
                    parity = 0;
                } else {
                    driveBackward(1);
                    parity = 1;
                }
            }

            blue = 0;                 //Determining the light color
            startTime = TimeNow();
            while(TimeNow() - startTime < 1.5) {
                blue = cds.Value() < (nolight - 1) ? 0 : 1; //Look up ternary operator if this confuses you.
            }

            if(blue) LCD.WriteLine("I'm blue :(");
            else LCD.WriteLine("Red, the blood of angry men");

            //Deposit skid step 19
            reverseToWall();                //Back to the counter

//goto menu; //Deposit the skid in segment 3
//segment3:
Sleep(.5);
driveForward(3);
pivotRightTurn();
reverseToWall();

//Check if we hit the chiller door and messed up that way
if(backButtonLeft.Value()==0&&backButtonRight.Value()==1)
{
    driveForward(2);
    rightMotor.SetPercent(40);
    Sleep(.2);
    reverseToWall();
}

driveForward(1);
pivotLeftTurn(); //Now I'm facing the chiller
reverseToWall();

//Deposit skid in chiller step 28
driveForward(12);

liftHeight(0);

driveBackward(4);

liftMotor.SetPercent(50);
Sleep(.6);
liftMotor.SetPercent(0);
driveForward(12);

//move to corner to begin scoop dropping step 33
reverseToWall(); //Skid is all good

//goto menu; //End segment3
//segment4:
Sleep(.3);

//Get to the front corner of the shop
driveForward(2);
pivotLeftTurn();
reverseToWall();
rightMotor.SetPercent(50);
Sleep(.4);
driveForward(5);
rightMotor.SetPercent(-50);
Sleep(.4);
reverseToWall();

//deposit scoop step 37 CORNER
if(blue) driveForward(5);
else driveForward(23);
liftMotor.SetPercent(60); Sleep(1.2);
liftMotor.SetPercent(0);
reverseToWall();

liftHeight(0);

//Get away from corner step 40
leftMotor.SetPercent(50);
Sleep(1.4);
driveForward(6);
leftMotor.SetPercent(-50);
Sleep(1.4);
reverseToWall();

//Line up to ramp step 44
driveForward(12);
pivotRightTurn();
reverseToWall();

//goto menu;
//segment5:
Sleep(.5);
//Drive up ramp step 48
driveForward(8);
pivotRightTurn(); pivotRightTurn(); leftMotor.SetPercent(50); Sleep(.4); stop();
reverseToWallHigh();

//Position ourselves in front of switch
int count2 = 0;
while((backButtonLeft.Value() == 1 || backButtonRight.Value() == 1) && count2 < 2)
{
    leftMotor.SetPercent(50);
    Sleep(.7);
    driveForward(5);
    leftMotor.SetPercent(-50);
    Sleep(.7);
    reverseToWall();
    Sleep(.7);
    count2++;
}

reverseToWall();


//goto menu;
//segment6:
Sleep(.5);

//Turn to face the switch
driveForward(4);
pivotLeftTurn(); pivotLeftTurn(); stop();

//Drive to switch then turn it
driveForward(8);
pivotLeftTurn();

//Need to test this area more

driveBackward(10);
rightMotor.SetPercent(-40);
Sleep(.3);
rightMotor.SetPercent(0);
reverseToWall();


driveForward(7);
rightMotor.SetPercent(50);
Sleep(.6);

int oven = RPS.Oven();
int count=1;
while(count<oven)
{
    driveBackward(2);
    driveForward(3);
}

rightMotor.SetPercent(50);
Sleep(.12);
reverseToWall();

LCD.WriteLine(TimeNow()-veryStart);
//driveForward(2);
//pivotRightTurn();
//reverseToWall();



goto menu;
//End all code
/*
driveForward(4);
pivotLeftTurn();
reverseToWall();
pivotRightTurn();
reverseToWall();

liftHeight(0);

//Line up with button step 63.
pivotLeftTurn();
reverseToWall();
driveForward(7.8);
pivotLeftTurn();
driveForward(35);

//************************************************************

//Will put button number code in when we're ready to implement wonka for now default value is assigned
//Push button step 69
int ovenTimes = 2;
for(int i=0; i<ovenTimes; i++)
{
driveForward(3);
driveBackward(2);
}

//Flip Switch
driveBackward(13);
pivotRightTurn();
reverseToWall();

while(true)              //Line up with the switch
{
    if(backButtonLeft.Value()==0&&backButtonRight.Value()==1)
break;
    else if(backButtonLeft.Value()==1&&backButtonRight.Value()==1) {driveBackward(-6);}
    else if(backButtonLeft.Value()==0&&backButtonRight.Value()==0)
    {

        driveForward(2);
        leftMotor.SetPercent(50); Sleep(.2);
        driveBackward(6);
        Sleep(0.4);
    }
}

//Lift code for step 78 needed here
pivotRightTurn();
reverseToWall();
driveForward(5);
pivotRightTurn();
reverseToWall();
*/



        // Press middle button to run again
        LCD.WriteLine("Run course again? Press middle button");
    }

    if(false)
    {
    //Menu tool
    menu:
        while(!buttons.LeftPressed()&&!buttons.MiddlePressed()&&!buttons.RightPressed());
        if(buttons.LeftPressed())
        {
             while(buttons.LeftPressed());
             //goto segment4;
        }
        if(buttons.MiddlePressed())
        {
             while(buttons.MiddlePressed());
             //goto segment5;
        }
        if(buttons.RightPressed())
        {
             while(buttons.RightPressed());
             //goto segment6;
        }
        //End menu tool
    }
}
