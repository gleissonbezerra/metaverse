/*
  IMU Classifier
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU, once enough samples are read, it then uses a
  TensorFlow Lite (Micro) model to try to classify the movement as a known gesture.
  Note: The direct use of C/C++ pointers, namespaces, and dynamic memory is generally
        discouraged in Arduino examples, and in the future the TensorFlowLite library
        might change to make the sketch simpler.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>

#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>

//#include <tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h>
//#include <tensorflow/lite/experimental/micro/micro_error_reporter.h>
//#include <tensorflow/lite/experimental/micro/micro_interpreter.h>

#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include <ArduinoBLE.h>

#include "model.h"

const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 100;

int samplesRead = numSamples;

//Custom BLE Service
BLEService* kickDetectionService = nullptr;

//One byte detection data
// 2 = Kick confirmed
// 1 = Moving
// 0 = No Kick
BLEUnsignedCharCharacteristic* inferenceChar = nullptr;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
  "kick",
  "no-kick"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void setup() {

  // service and char UUID:
  kickDetectionService = new BLEService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  inferenceChar = new BLEUnsignedCharCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8",  // custom characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes
  

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  digitalWrite(LEDR, HIGH);               // will turn the LED off
  digitalWrite(LEDG, HIGH);               // will turn the LED off
  digitalWrite(LEDB, HIGH);                // will turn the LED off
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);                // will turn the LED off

  Serial.begin(9600);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Error:IMU");
    digitalWrite(LEDR, LOW);   // turn the LED on (HIGH is the voltage level)
    while (1);
  }

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    digitalWrite(LEDR, LOW);   // turn the LED on (HIGH is the voltage level)
    while (1);
  }

  
  
  /* Set a local name for the Bluetooth® Low Energy device
     This name will appear in advertising packets
     and can be used by remote devices to identify this Bluetooth® Low Energy device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */
 
  BLE.setAdvertisedService(*kickDetectionService); // add the service UUID
  kickDetectionService->addCharacteristic(*inferenceChar); // add the inference characteristic
  
  BLE.addService(*kickDetectionService); // Add the detection  service

  BLE.setLocalName("KickSense");
  
  inferenceChar->writeValue(0);  // and update the inference characteristic
  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth® device active, waiting for connections...");

  //print out the samples rates of the IMUs
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");

  Serial.println();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(__kick_model_tflite);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    digitalWrite(LEDR, LOW);   // turn the LED on (HIGH is the voltage level)
    Serial.println("Error: Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  digitalWrite(LEDG, LOW);               // will turn the LED on

}

bool active = false;

void loop() {

  // wait for a Bluetooth® Low Energy central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {

    float aX, aY, aZ, gX, gY, gZ;
    
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(LEDG, HIGH);               // will turn the LED off

    // while the central is connected:
    while (central.connected()) 
    {

      // wait for significant motion
      if(active == false && samplesRead == numSamples) 
      {
        
        if (IMU.accelerationAvailable()) {
          // read the acceleration data
          IMU.readAcceleration(aX, aY, aZ);
  
          // sum up the absolutes
          float aSum = fabs(aX) + fabs(aY) + fabs(aZ);
  
          // check if it's above the threshold
          if (aSum >= accelerationThreshold) {
            // reset the sample read count
            samplesRead = 0;
            active = true;

            if (central.connected())
            {
              //Serial.println(inferenceData);
              inferenceChar->writeValue(1);  // and update the inference characteristic
            }

          }
        }

      }

      // check if the all the required samples have been read since
      // the last time the significant motion was detected
      if (active == true && samplesRead < numSamples) 
      {
        // check if the all the required samples have been read since
        // the last time the significant motion was detected
        digitalWrite(LEDB, LOW);   // turn the LED on (HIGH is the voltage level)
        
        // check if new acceleration AND gyroscope data is available
        if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
          
          // read the acceleration and gyroscope data
          IMU.readAcceleration(aX, aY, aZ);
          IMU.readGyroscope(gX, gY, gZ);
    
          // normalize the IMU data between 0 to 1 and store in the model's
          // input tensor
          tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
          tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
          tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
          tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;
    
          samplesRead++;
    
          if (samplesRead == numSamples) {
    
            active = false;
            
            // Run inferencing
            TfLiteStatus invokeStatus = tflInterpreter->Invoke();
            if (invokeStatus != kTfLiteOk) {
              Serial.println("Error: TF Invoke failed!");
              while (1);
              return;
            }
    
            // DEBUG ONLY: Loop through the output tensor values from the model
            for (int i = 0; i < NUM_GESTURES; i++) {
              Serial.print(GESTURES[i]);
              Serial.print(":");
              Serial.print(tflOutputTensor->data.f[i], 6);
              if ( i < NUM_GESTURES - 1)
                Serial.print("|");
            }
            
            Serial.println();
    
            if (central.connected())
            {
              if ( tflOutputTensor->data.f[0] > tflOutputTensor->data.f[1] && tflOutputTensor->data.f[0] > 0.5){ //Kick greater than no-kick
                inferenceChar->writeValue(2);  // and update the inference characteristic
              }else{
                inferenceChar->writeValue(0);  // and update the inference characteristic
              }
      
              //Serial.println(inferenceData);
            }
            
          }
        }

       digitalWrite(LEDB, HIGH);   // turn the LED on (HIGH is the voltage level)
        
      }
  
    
    }
   
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LEDG, LOW);               // will turn the LED on
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

}
