#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <time.h>

#include <TimeLib.h>

LiquidCrystal_I2C lcd1(0x27, 16, 2);
LiquidCrystal_I2C lcd2(0x26, 16, 2);

int num = 0; //to store user inputs
int len = 0;
int entryInpNo = 1;
int exitInpNo = 1;
int car_reg[100]; //array to store registration no. of cars
int otp[100]; //array to store otp for cars    
int enter_time[100]; //array to store entry time  of cars
int exit_time[100]; //array to store exit time  of cars
int total_fare[100]; //array to store total fare  of cars
int car_reg_counter = 0; //no. to uniquely identify each entry in the arrays 

int s1 = A0; //ir sensor pins of slots 
int s2 = A1;
int s3 = A2;
int s4 = A3;
int slarr[] = { //array of ir sensors of the slots
    s1,
    s2,
    s3,
    s4
};
int parkingState[4]; //stores the states of the parking slots 0-if it is free , 1-if its occupied 

void setup() 
{
    Serial.begin(9600);
    // defining analog pins as input
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A5, INPUT);
    pinMode(A4, INPUT);
    // defining digital pins 13 to 2 as input
    for (int i = 13; i >= 2; i--) {
        pinMode(i, INPUT);
    }

    // initialising lcds
    lcd1.begin();
    lcd1.backlight();
    lcd1.clear();
    lcd2.begin();
    lcd2.backlight();
    lcd2.clear();
}

// function that calculates the digit entered using the keypad 
// based on the pin input
int calcDig(int i, int j, int p1, int p2) {
    int r = p1 + 1 - i;
    int c = p2 + 1 - j;

    // formula that relates the row and column number to the digit
    return (3 * (r - 1) + c);
}

// handles all from of keypad input - entry/exit
// i.e. retrieves 4 digit input from keypad
void getInput(int p1, int p2) {
    num = 0;
    len = 0;

    // while loop runs until number length is 4
    while (1) {
        int flag = 0;
        // scans through the input pins checking for digital high row
        for (int i = p1; i > p1 - 3; i--) {
            if (digitalRead(i)) {
                // if found then corresponding column is scanned
                for (int j = p2; j > p2 - 3; j--) {
                    if (digitalRead(j)) {
                        // .if found then the digit is calculated
                        len += 1;
                        // number is updated
                        num = num * 10 + calcDig(i, j, p1, p2);
                        while ((digitalRead(i) == 1 && digitalRead(j) == 1)) {}
                        flag = 1;
                    }
                }
            }
            if (flag) break;
        }

        // when number length becomes == 4, number is returned
        if (len == 4) {
            Serial.println(num);
            break;
        }
    }
}

int find_car_i(int car_reg_no) {
    for (int i = 0; i < 100; i++) { 
        //searching the array for the registration no.
        if (car_reg_no == car_reg[i]) {
            return i;
        }
    }
    return -1; //if no car with the given registration no.
}

int otp_generator() {

    int number1[10] = {
        1432,
        5472, //a sample set of otps
        6243,
        6123,
        9856,
        9623,
        7452,
        9262,
        1392,
        5139
    };
    int number = rand() % 10; //an index generated with random generator 
    return number1[number]; //the index is used to retrieve an otp from the sample set
}

void fare_calculator(int x) { //fare calculated according to the time the car is parked
    int fare = exit_time[x] - enter_time[x]; //the time stayed
    total_fare[x] = fare * .24;
}

//function to give the nearest slot no to the mall enrtrance
int nearestSlot() { 

    // as the mall entrance is in between  parking slot 2 and 3 we start moving from the
    // between  to left side and right side and where we encounter a free parking slot we return the slot no. which is minimum
    int left = -1; //initially set to -1
    int right = -1;
    //left and right respectively store the nearest parking slots on the left and right side 
    for (int i = 1; i >= 0; i--) { //searching on the left side
        if (parkingState[i] == 0) { //the parking slot is free
            left = i;
            break;
        }
    }
    for (int i = 2; i <= 3; i++) { //searching on the right side
        if (parkingState[i] == 0) {
            right = i;
            break;
        }
    }
    if (abs(1.5 - left) <= abs(1.5 - right)) { //the distance from the middle (entrance) ,whichever is minimum will be returned
        return left;
    } else
        return right;
}

int entryInput() {
    int nearest = nearestSlot();

    if (nearest == -1) { //when all slots  are filled
        lcd1.clear();
        lcd1.setCursor(0, 0);
        lcd1.print("No free slots");
        lcd1.setCursor(0, 1);
        lcd1.print("Sorry ;..( ");
        delay(3000);
        entryInpNo = 0;
        return;
    }

    num = 0;
    len = 0;
    // in order to calculate the time elapsed, the entry time stamp is stored in an array
    int seconds = second();
    int minutes = minute();
    int hours = hour();
    int in_timing = seconds + (60 * minutes) + (hours * 3600);
    enter_time[car_reg_counter] = in_timing; //enter_time to store the enrty time of each car 

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Enter reg no");

    int otp_of_car = 0;
    // gets the regestraion number using keypad
    getInput(13, 10);
    int reg_number = num; //registration no. of the car 

    lcd1.clear();
    lcd1.print("Confirm reg no");
    lcd1.setCursor(0, 1);
    lcd1.print(reg_number);
    lcd1.print(" 1-ok 2-no");

    // .conformation of regestraion number
    int x = 0;
    int y = 0;
    while (1) {
        x = digitalRead(13) && digitalRead(10);
        y = digitalRead(13) && digitalRead(9);

        if (x || y) {
            break;
        }
    }

    if (x) {
        entryInpNo = 0;
        while ((digitalRead(13) == 1 && digitalRead(10) == 1)) {}
    } else if (y) {
        entryInpNo = 1;
        num = 0;
        len = 0;
        while ((digitalRead(13) == 1 && digitalRead(9) == 1)) {}
        return;
    }

    car_reg[car_reg_counter] = reg_number; //stores the registration no.

    otp_of_car = otp_generator(); //random otp generated

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Your otp"); //otp displayed
    lcd1.setCursor(0, 1);
    lcd1.print(otp_of_car);

    otp[car_reg_counter] = otp_of_car; //otp stored in array
    car_reg_counter++; //counter increased by 1
    delay(5000);

    lcd1.clear();
    lcd1.setCursor(0, 0);
    lcd1.print("Nearest slot : ");
    lcd1.print(nearest + 1);
    Serial.println("Gate opening");
    delay(3000);
}

int exitInput() {
    Serial.println("In exit");
    int exit_counter = 0; //to count the no. of attempts while entering otp

    int carIdx; //the index by which the car entry is stored in the arrays
    int reg_counter = 0; //to store the no. of attempts while entering the registration counter
    int validReg = 0; //to store whether registration no. found
    int reg_number;
    while (validReg == 0 && reg_counter < 3) {
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Enter reg no");
        getInput(7, 4); //processes the registration no.
        reg_number = num;
        carIdx = find_car_i(reg_number); //finds the unique id according to registration no. of the car
        reg_counter++;
        if (carIdx == -1) { //no such registration no. as entered by the user
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Reg num missing");
            lcd2.setCursor(0, 1);
            lcd2.print((3 - reg_counter));
            lcd2.print(" attempts left");
            delay(3000);
        } else {
            validReg = 1;
            break;
        }
    }

    lcd2.clear();
    lcd2.print("Confirm reg no");
    lcd2.setCursor(0, 1);
    lcd2.print(reg_number);
    lcd2.print(" 1-ok 2-no");
    int x = 0;
    int y = 0;
    while (1) {
        // if 1 is choosen the x turns high
        x = digitalRead(7) && digitalRead(4);
        // is 2 is chosen then y turns high
        y = digitalRead(7) && digitalRead(3);
        if (x || y) {
            break;
        }
    }

    if (validReg == 1) {
        if (x) {
            exitInpNo = 0;
            while ((digitalRead(7) == 1 && digitalRead(4) == 1)) {}
        } else if (y) {
            exitInpNo = 1;
            while ((digitalRead(7) == 1 && digitalRead(3) == 1)) {}
            return;
        }
        int validOTP = 0; //to check if otp matches

        int carIdx = find_car_i(reg_number); //the index of the entry

        while (validOTP == 0 && exit_counter < 3) {
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Enter OTP");
            getInput(7, 4);
            int otpEnt = num; //the otp entered by the user
            if (otpEnt == otp[carIdx] && exit_counter < 3) { //correct otp
                lcd2.clear();
                lcd2.setCursor(0, 0);
                lcd2.print("CORRECT OTP!");
                delay(2000);

                validOTP = 1;
                break;
            } else if (exit_counter < 3) { //3 atempts allowed for otp
                exit_counter++;
                lcd2.clear();
                lcd2.setCursor(0, 0);
                lcd2.print("WRONG OTP!");
                lcd2.setCursor(0, 1);
                lcd2.print((3 - exit_counter));
                lcd2.print(" attempts left");
                delay(4000);
            } //else case..when wrong otp is entered

        }

        if (validOTP == 1) { //on receiving valid otp
            int seconds = second();
            int minutes = minute();
            int hours = hour();
            int out_timing = seconds + (60 * minutes) + (hours * 3600);
            exit_time[carIdx] = out_timing;
            fare_calculator(carIdx); //calculating the fare according to the time stayed 
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Fare: Rs");
            lcd2.print(total_fare[carIdx]);
            delay(5000);
            Serial.println("Your total fare: Rs");
            Serial.print(total_fare[carIdx]);
        } else { //for mismatch of otp in all attempts
            lcd2.clear();
            lcd2.setCursor(0, 0);
            lcd2.print("Fine amt:Rs1000");
            delay(5000);
        }
    } else { //registration no. mismatch in all 3 attempts
        lcd2.clear();
        lcd2.setCursor(0, 0);
        lcd2.print("Wrong reg num");
        lcd2.setCursor(0, 1);
        lcd2.print("Fine amt :Rs1000");
        delay(4000);

    }

    car_reg[carIdx] = -1; //the entry removed from database
    otp[carIdx] = -1;

}

int checkState() {
    // updates the state of every parking slot and stores it in an array 

    int count = 0; //to count the no. of filled slots 
    for (int i = 0; i < 4; i++) {
        if (analogRead(slarr[i]) < 500) { //car is parked
            count += 1;
            parkingState[i] = 1; //state changed to one
        } else {
            parkingState[i] = 0; //slot is empty
        }

    }
    return count;
}
// 
void loop() {
    checkState(); //function checks the no. of filled slots and fills the state of the parking slots
    if (analogRead(A4) < 500) { 
        //if a car is detected at the entry gate
        lcd1.clear();
        // while loop runs until valid input is detected
        while (entryInpNo == 1)
            entryInput();
        entryInpNo = 1;
        lcd1.clear();
    }
    if (analogRead(A5) < 300) { 
        //if a car is detected at the exity gate
        lcd2.clear();
        // while loop runs until valid input is detected
        while (exitInpNo == 1) {
            exitInput();
        }
        exitInpNo = 1;
        lcd2.clear();
    }
}